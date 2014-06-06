/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_cc115l.h>
#include <ao_exti.h>
#include <ao_telemetry.h>
#include <ao_fec.h>

#define AO_RADIO_MAX_SEND	sizeof (struct ao_telemetry_generic)

uint8_t ao_radio_mutex;

static uint8_t ao_radio_fifo;		/* fifo drained interrupt received */
static uint8_t ao_radio_done;		/* tx done interrupt received */
static uint8_t ao_radio_wake;		/* sleep address for radio interrupts */
static uint8_t ao_radio_abort;		/* radio operation should abort */

/* Debugging commands */
#define CC115L_DEBUG	0

/* Runtime tracing */
#define CC115L_TRACE	0

#define FOSC	26000000

#define ao_radio_select()	ao_spi_get_mask(AO_CC115L_SPI_CS_PORT,(1 << AO_CC115L_SPI_CS_PIN),AO_CC115L_SPI_BUS,AO_SPI_SPEED_1MHz)
#define ao_radio_deselect()	ao_spi_put_mask(AO_CC115L_SPI_CS_PORT,(1 << AO_CC115L_SPI_CS_PIN),AO_CC115L_SPI_BUS)
#define ao_radio_spi_send(d,l)	ao_spi_send((d), (l), AO_CC115L_SPI_BUS)
#define ao_radio_spi_send_fixed(d,l) ao_spi_send_fixed((d), (l), AO_CC115L_SPI_BUS)
#define ao_radio_spi_recv(d,l)	ao_spi_recv((d), (l), AO_CC115L_SPI_BUS)
#define ao_radio_duplex(o,i,l)	ao_spi_duplex((o), (i), (l), AO_CC115L_SPI_BUS)

struct ao_cc115l_reg {
	uint16_t	addr;
	char		*name;
};

#if CC115L_TRACE

static const struct ao_cc115l_reg ao_cc115l_reg[];
static const char *cc115l_state_name[];

enum ao_cc115l_trace_type {
	trace_strobe,
	trace_read,
	trace_write,
	trace_dma,
	trace_line,
};

struct ao_cc115l_trace {
	enum ao_cc115l_trace_type	type;
	int16_t				addr;
	int16_t				value;
	const char			*comment;
};

#define NUM_TRACE	256

static struct ao_cc115l_trace	trace[NUM_TRACE];
static int			trace_i;
static int			trace_disable;

static void trace_add(enum ao_cc115l_trace_type type, int16_t addr, int16_t value, const char *comment)
{
	if (trace_disable)
		return;
	switch (type) {
	case trace_read:
	case trace_write:
		comment = ao_cc115l_reg[addr].name;
		break;
	case trace_strobe:
		comment = cc115l_state_name[(value >> 4) & 0x7];
		break;
	default:
		break;
	}
	trace[trace_i].type = type;
	trace[trace_i].addr = addr;
	trace[trace_i].value = value;
	trace[trace_i].comment = comment;
	if (++trace_i == NUM_TRACE)
		trace_i = 0;
}
#else
#define trace_add(t,a,v,c)
#endif

static uint8_t
ao_radio_reg_read(uint8_t addr)
{
	uint8_t	data[1];

	data[0] = ((1 << CC115L_READ)  |
		   (0 << CC115L_BURST) |
		   addr);
	ao_radio_select();
	ao_radio_spi_send(data, 1);
	ao_radio_spi_recv(data, 1);
	ao_radio_deselect();
	trace_add(trace_read, addr, data[0], NULL);
	return data[0];
}

static void
ao_radio_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	data[2];

	trace_add(trace_write, addr, value, NULL);
	data[0] = ((0 << CC115L_READ)  |
		   (0 << CC115L_BURST) |
		   addr);
	data[1] = value;
	ao_radio_select();
	ao_radio_spi_send(data, 2);
	ao_radio_deselect();
}

#if UNUSED
static void
ao_radio_burst_read_start (uint16_t addr)
{
	uint8_t data[1];

	data[0] = ((1 << CC115L_READ)  |
		   (1 << CC115L_BURST) |
		   addr);
	ao_radio_select();
	ao_radio_spi_send(data, 1);
}

static void
ao_radio_burst_read_stop (void)
{
	ao_radio_deselect();
}
#endif


static uint8_t
ao_radio_strobe(uint8_t addr)
{
	uint8_t	in;

	ao_radio_select();
	ao_radio_duplex(&addr, &in, 1);
	ao_radio_deselect();
	trace_add(trace_strobe, addr, in, NULL);
	return in;
}

static uint8_t
ao_radio_fifo_write_start(void)
{
	uint8_t	addr = ((0 << CC115L_READ)  |
			(1 << CC115L_BURST) |
			CC115L_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	return status;
}

static inline uint8_t ao_radio_fifo_write_stop(uint8_t status) {
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_fifo_write(uint8_t *data, uint8_t len)
{
	uint8_t	status = ao_radio_fifo_write_start();
	trace_add(trace_dma, CC115L_FIFO, len, NULL);
	ao_radio_spi_send(data, len);
	return ao_radio_fifo_write_stop(status);
}

static uint8_t
ao_radio_tx_fifo_space(void)
{
	return CC115L_FIFO_SIZE - (ao_radio_reg_read(CC115L_TXBYTES) & CC115L_TXBYTES_NUM_TX_BYTES_MASK);
}

#if CC115L_DEBUG
static uint8_t
ao_radio_status(void)
{
	return ao_radio_strobe (CC115L_SNOP);
}

static uint8_t
ao_radio_get_marcstate(void)
{
	return ao_radio_reg_read(CC115L_MARCSTATE) & CC115L_MARCSTATE_MASK;
}
#endif

#define ao_radio_rdf_value 0x55

static void
ao_radio_done_isr(void)
{
	ao_exti_disable(AO_CC115L_DONE_INT_PORT, AO_CC115L_DONE_INT_PIN);
	trace_add(trace_line, __LINE__, 0, "done_isr");
	ao_radio_done = 1;
	ao_wakeup(&ao_radio_wake);
}

static void
ao_radio_fifo_isr(void)
{
	ao_exti_disable(AO_CC115L_FIFO_INT_PORT, AO_CC115L_FIFO_INT_PIN);
	trace_add(trace_line, __LINE__, 0, "fifo_isr");
	ao_radio_fifo = 1;
	ao_wakeup(&ao_radio_wake);
}

static void
ao_radio_idle(void)
{
	ao_radio_pa_off();
	for (;;) {
		uint8_t	state = ao_radio_strobe(CC115L_SIDLE);
		if ((state >> CC115L_STATUS_STATE) == CC115L_STATUS_STATE_IDLE)
			break;
		if ((state >> CC115L_STATUS_STATE) == CC115L_STATUS_STATE_TX_FIFO_UNDERFLOW)
			ao_radio_strobe(CC115L_SFTX);
	}
	/* Flush any pending TX bytes */
	ao_radio_strobe(CC115L_SFTX);
}

/*
 * Packet deviation is 20.5kHz
 *
 *	fdev = fosc >> 17 * (8 + dev_m) << dev_e
 *
 *     	26e6 / (2 ** 17) * (8 + 5) * (2 ** 3) = 20630Hz
 */

#define PACKET_DEV_E	3
#define PACKET_DEV_M	5

/*
 * For our packet data, set the symbol rate to 38400 Baud
 *
 *              (256 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 28
 *
 *		(256 + 131) * (2 ** 10) / (2**28) * 26e6 = 38383
 *
 *	DATARATE_M = 131
 *	DATARATE_E = 10
 */
#define PACKET_DRATE_E	10
#define PACKET_DRATE_M	131

static const uint16_t packet_setup[] = {
	CC115L_DEVIATN,		((PACKET_DEV_E << CC115L_DEVIATN_DEVIATION_E) |
				 (PACKET_DEV_M << CC115L_DEVIATN_DEVIATION_M)),
	CC115L_MDMCFG4,		((0xf << 4) |
				 (PACKET_DRATE_E << CC115L_MDMCFG4_DRATE_E)),
	CC115L_MDMCFG3,		(PACKET_DRATE_M),
	CC115L_MDMCFG2,		(0x00 |
				 (CC115L_MDMCFG2_MOD_FORMAT_GFSK << CC115L_MDMCFG2_MOD_FORMAT) |
				 (0 << CC115L_MDMCFG2_MANCHESTER_EN) |
				 (CC115L_MDMCFG2_SYNC_MODE_16BITS << CC115L_MDMCFG2_SYNC_MODE)),
};


/*
 * RDF deviation is 5kHz
 *
 *	fdev = fosc >> 17 * (8 + dev_m) << dev_e
 *
 *     	26e6 / (2 ** 17) * (8 + 4) * (2 ** 1) = 4761Hz
 */

#define RDF_DEV_E	1
#define RDF_DEV_M	4

/*
 * For our RDF beacon, set the symbol rate to 2kBaud (for a 1kHz tone)
 *
 *              (256 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 28
 *
 *		(256 + 67) * (2 ** 6) / (2**28) * 26e6 = 2002
 *
 *	DATARATE_M = 67
 *	DATARATE_E = 6
 */
#define RDF_DRATE_E	6
#define RDF_DRATE_M	67

static const uint16_t rdf_setup[] = {
	CC115L_DEVIATN,		((RDF_DEV_E << CC115L_DEVIATN_DEVIATION_E) |
				 (RDF_DEV_M << CC115L_DEVIATN_DEVIATION_M)),
	CC115L_MDMCFG4,		((0xf << 4) |
				 (RDF_DRATE_E << CC115L_MDMCFG4_DRATE_E)),
	CC115L_MDMCFG3,		(RDF_DRATE_M),
	CC115L_MDMCFG2,		(0x00 |
				 (CC115L_MDMCFG2_MOD_FORMAT_GFSK << CC115L_MDMCFG2_MOD_FORMAT) |
				 (0 << CC115L_MDMCFG2_MANCHESTER_EN) |
				 (CC115L_MDMCFG2_SYNC_MODE_NONE << CC115L_MDMCFG2_SYNC_MODE)),
};

/*
 * APRS deviation is 3kHz
 *
 * 26e6 / (2 ** 17) * (8 + 7) * (2 ** 0) = 2975
 */

#define APRS_DEV_E	0
#define APRS_DEV_M	7

/*
 * For our APRS beacon, set the symbol rate to 9.6kBaud (8x oversampling for 1200 baud data rate)
 *
 *              (256 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 28
 *
 *		(256 + 131) * (2 ** 8) / (2**28) * 26e6 = 9596
 *
 *	DATARATE_M = 131
 *	DATARATE_E = 8
 *
 */
#define APRS_DRATE_E	8
#define APRS_DRATE_M	131

static const uint16_t aprs_setup[] = {
	CC115L_DEVIATN,		((APRS_DEV_E << CC115L_DEVIATN_DEVIATION_E) |
				 (APRS_DEV_M << CC115L_DEVIATN_DEVIATION_M)),
	CC115L_MDMCFG4,		((0xf << 4) |
				 (APRS_DRATE_E << CC115L_MDMCFG4_DRATE_E)),
	CC115L_MDMCFG3,		(APRS_DRATE_M),
	CC115L_MDMCFG2,		(0x00 |
				 (CC115L_MDMCFG2_MOD_FORMAT_GFSK << CC115L_MDMCFG2_MOD_FORMAT) |
				 (0 << CC115L_MDMCFG2_MANCHESTER_EN) |
				 (CC115L_MDMCFG2_SYNC_MODE_NONE << CC115L_MDMCFG2_SYNC_MODE)),
};

#define AO_PKTCTRL0_INFINITE	((CC115L_PKTCTRL0_PKT_FORMAT_NORMAL << CC115L_PKTCTRL0_PKT_FORMAT) | \
				 (0 << CC115L_PKTCTRL0_PKT_CRC_EN) |					\
				 (CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_INFINITE << CC115L_PKTCTRL0_PKT_LENGTH_CONFIG))
#define AO_PKTCTRL0_FIXED	((CC115L_PKTCTRL0_PKT_FORMAT_NORMAL << CC115L_PKTCTRL0_PKT_FORMAT) | \
				 (0 << CC115L_PKTCTRL0_PKT_CRC_EN) |					\
				 (CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_FIXED << CC115L_PKTCTRL0_PKT_LENGTH_CONFIG))

static uint16_t ao_radio_mode;


/*
 * These set the data rate and modulation parameters
 */
#define AO_RADIO_MODE_BITS_PACKET_TX	1
#define AO_RADIO_MODE_BITS_RDF		2
#define AO_RADIO_MODE_BITS_APRS		4

/*
 * Flips between infinite packet mode and fixed packet mode;
 * we use infinite mode until the sender gives us the
 * last chunk of data
 */
#define AO_RADIO_MODE_BITS_INFINITE	40
#define AO_RADIO_MODE_BITS_FIXED	80

#define AO_RADIO_MODE_NONE		0

#define AO_RADIO_MODE_RDF		AO_RADIO_MODE_BITS_RDF
#define AO_RADIO_MODE_PACKET_TX		AO_RADIO_MODE_BITS_PACKET_TX
#define AO_RADIO_MODE_APRS		AO_RADIO_MODE_BITS_APRS

static void
ao_radio_set_mode(uint16_t new_mode)
{
	uint16_t changes;
	unsigned int i;

	if (new_mode == ao_radio_mode)
		return;

	changes = new_mode & (~ao_radio_mode);
	if (changes & AO_RADIO_MODE_BITS_PACKET_TX)
		for (i = 0; i < sizeof (packet_setup) / sizeof (packet_setup[0]); i += 2)
			ao_radio_reg_write(packet_setup[i], packet_setup[i+1]);

	if (changes & AO_RADIO_MODE_BITS_RDF)
		for (i = 0; i < sizeof (rdf_setup) / sizeof (rdf_setup[0]); i += 2)
			ao_radio_reg_write(rdf_setup[i], rdf_setup[i+1]);

	if (changes & AO_RADIO_MODE_BITS_APRS)
		for (i = 0; i < sizeof (aprs_setup) / sizeof (aprs_setup[0]); i += 2)
			ao_radio_reg_write(aprs_setup[i], aprs_setup[i+1]);

	if (changes & AO_RADIO_MODE_BITS_INFINITE)
		ao_radio_reg_write(CC115L_PKTCTRL0, AO_PKTCTRL0_INFINITE);

	if (changes & AO_RADIO_MODE_BITS_FIXED)
		ao_radio_reg_write(CC115L_PKTCTRL0, AO_PKTCTRL0_FIXED);

	ao_radio_mode = new_mode;
}

/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with address, value
 *
 *  RF device: CC115L
 *
 ***************************************************************/

static const uint16_t radio_setup[] = {

	/* High when FIFO is above threshold, low when fifo is below threshold */
	AO_CC115L_FIFO_INT_GPIO_IOCFG,	    CC115L_IOCFG_GPIO_CFG_TXFIFO_THR,

	/* High when transmitter is running, low when off */
	AO_CC115L_DONE_INT_GPIO_IOCFG,	    CC115L_IOCFG_GPIO_CFG_PA_PD | (1 << CC115L_IOCFG_GPIO_INV),

        CC115L_FIFOTHR,                     0x47,       /* TX FIFO Thresholds */
	CC115L_MDMCFG1,			    (0x00 |
					     (CC115L_MDMCFG1_NUM_PREAMBLE_4 << CC115L_MDMCFG1_NUM_PREAMBLE) |
					     (1 << CC115L_MDMCFG1_CHANSPC_E)),
	CC115L_MDMCFG0,			    248,	/* Channel spacing M value (100kHz channels) */
        CC115L_MCSM0,                       0x38,       /* Main Radio Control State Machine Configuration */
        CC115L_RESERVED_0X20,               0xfb,       /* Use setting from SmartRF Studio */
        CC115L_FSCAL3,                      0xe9,       /* Frequency Synthesizer Calibration */
        CC115L_FSCAL2,                      0x2a,       /* Frequency Synthesizer Calibration */
        CC115L_FSCAL1,                      0x00,       /* Frequency Synthesizer Calibration */
        CC115L_FSCAL0,                      0x1f,       /* Frequency Synthesizer Calibration */
        CC115L_TEST2,                       0x81,       /* Various Test Settings */
        CC115L_TEST1,                       0x35,       /* Various Test Settings */
        CC115L_TEST0,                       0x09,       /* Various Test Settings */
};

static uint8_t	ao_radio_configured = 0;

static void
ao_radio_setup(void)
{
	unsigned int	i;

	ao_radio_strobe(CC115L_SRES);
	ao_delay(AO_MS_TO_TICKS(10));

	for (i = 0; i < sizeof (radio_setup) / sizeof (radio_setup[0]); i += 2)
		ao_radio_reg_write(radio_setup[i], radio_setup[i+1]);

	ao_radio_mode = 0;

	ao_config_get();

	ao_radio_strobe(CC115L_SCAL);

	ao_radio_configured = 1;
}

static void
ao_radio_set_len(uint8_t len)
{
	static uint8_t	last_len;

	if (len != last_len) {
		ao_radio_reg_write(CC115L_PKTLEN, len);
		last_len = len;
	}
}

static void
ao_radio_get(void)
{
	static uint32_t	last_radio_setting;

	ao_mutex_get(&ao_radio_mutex);
	if (!ao_radio_configured)
		ao_radio_setup();
	if (ao_config.radio_setting != last_radio_setting) {
		ao_radio_reg_write(CC115L_FREQ2, ao_config.radio_setting >> 16);
		ao_radio_reg_write(CC115L_FREQ1, ao_config.radio_setting >> 8);
		ao_radio_reg_write(CC115L_FREQ0, ao_config.radio_setting);
		last_radio_setting = ao_config.radio_setting;
	}
}

static void
_ao_radio_send_lots(ao_radio_fill_func fill, uint8_t mode);

#define ao_radio_put()	ao_mutex_put(&ao_radio_mutex)

struct ao_radio_tone {
	uint8_t	value;
	uint8_t	len;
};

struct ao_radio_tone *ao_radio_tone;
uint8_t ao_radio_tone_count;
uint8_t ao_radio_tone_current;
uint8_t ao_radio_tone_offset;

int16_t
ao_radio_tone_fill(uint8_t *buf, int16_t len)
{
	int16_t	ret = 0;

	while (len) {
		int16_t 		this_time;
		struct ao_radio_tone	*t;

		/* Figure out how many to send of the current value */
		t = &ao_radio_tone[ao_radio_tone_current];
		this_time = t->len - ao_radio_tone_offset;
		if (this_time > len)
			this_time = len;

		/* queue the data */
		memset(buf, t->value, this_time);

		/* mark as sent */
		len -= this_time;
		ao_radio_tone_offset += this_time;
		ret += this_time;

		if (ao_radio_tone_offset >= t->len) {
			ao_radio_tone_offset = 0;
			ao_radio_tone_current++;
			if (ao_radio_tone_current >= ao_radio_tone_count) {
				trace_add(trace_line, __LINE__, ret, "done with tone");
				return -ret;
			}
		}
	}
	trace_add(trace_line, __LINE__, ret, "got some tone");
	return ret;
}

static void
ao_radio_tone_run(struct ao_radio_tone *tones, int ntones)
{
	ao_radio_get();
	ao_radio_tone = tones;
	ao_radio_tone_current = 0;
	ao_radio_tone_offset = 0;
	ao_radio_tone_count = ntones;
	_ao_radio_send_lots(ao_radio_tone_fill, AO_RADIO_MODE_RDF);
	ao_radio_put();
}

void
ao_radio_rdf(void)
{
	struct ao_radio_tone	tone;

	tone.value = ao_radio_rdf_value;
	tone.len = AO_RADIO_RDF_LEN;
	ao_radio_tone_run(&tone, 1);
}

void
ao_radio_continuity(uint8_t c)
{
	struct ao_radio_tone	tones[7];
	uint8_t	count = 0;
	uint8_t i;

	for (i = 0; i < 3; i++) {
		tones[count].value = 0x00;
		tones[count].len = AO_RADIO_CONT_PAUSE_LEN;
		count++;
		if (i < c)
			tones[count].value = ao_radio_rdf_value;
		else
			tones[count].value = 0x00;
		tones[count].len = AO_RADIO_CONT_TONE_LEN;
		count++;
	}
	tones[count].value = 0x00;
	tones[count].len = AO_RADIO_CONT_PAUSE_LEN;
	count++;
	ao_radio_tone_run(tones, count);
}

void
ao_radio_rdf_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_wake);
}

#define POWER_STEP	0x08

#if HAS_RADIO_POWER
#define RADIO_POWER	ao_config.radio_power
#else
#define RADIO_POWER	0xc0
#endif

static void
ao_radio_stx(void)
{
	uint8_t	power;
	ao_radio_pa_on();
	ao_radio_reg_write(CC115L_PA, 0);
	ao_radio_strobe(CC115L_STX);
	for (power = POWER_STEP; power < RADIO_POWER; power += POWER_STEP)
		ao_radio_reg_write(CC115L_PA, power);
	if (power != RADIO_POWER)
		ao_radio_reg_write(CC115L_PA, RADIO_POWER);
}

static void
ao_radio_test_cmd(void)
{
	uint8_t	mode = 2;
	static uint8_t radio_on;
	ao_cmd_white();
	if (ao_cmd_lex_c != '\n') {
		ao_cmd_decimal();
		mode = (uint8_t) ao_cmd_lex_u32;
	}
	mode++;
	if ((mode & 2) && !radio_on) {
#if HAS_MONITOR
		ao_monitor_disable();
#endif
#if PACKET_HAS_SLAVE
		ao_packet_slave_stop();
#endif
		ao_radio_get();
		ao_radio_strobe(CC115L_SFTX);
		ao_radio_set_len(0xff);
		ao_radio_set_mode(AO_RADIO_MODE_RDF);
		ao_radio_stx();
		radio_on = 1;
	}
	if (mode == 3) {
		printf ("Hit a character to stop..."); flush();
		getchar();
		putchar('\n');
	}
	if ((mode & 1) && radio_on) {
		ao_radio_idle();
		ao_radio_put();
		radio_on = 0;
#if HAS_MONITOR
		ao_monitor_enable();
#endif
	}
}

#if CC115L_TRACE
static inline int16_t
ao_radio_gpio_bits(void)
{
	return ((ao_gpio_get(AO_CC115L_DONE_INT_PORT, AO_CC115L_DONE_INT_PIN, AO_CC115L_DONE_INT) << 1) |
		ao_gpio_get(AO_CC115L_FIFO_INT_PORT, AO_CC115L_FIFO_INT_PIN, AO_CC115L_FIFO_INT));
}
#endif

static void
ao_radio_wait_fifo(void)
{
	ao_arch_block_interrupts();
	while (!ao_radio_fifo && !ao_radio_done && !ao_radio_abort) {
		trace_add(trace_line, __LINE__, ao_radio_gpio_bits(), "wait_fifo");
		ao_sleep(&ao_radio_wake);
	}
	ao_arch_release_interrupts();
	trace_add(trace_line, __LINE__, ao_radio_gpio_bits(), "wake bits");
	trace_add(trace_line, __LINE__, ao_radio_fifo, "wake fifo");
	trace_add(trace_line, __LINE__, ao_radio_done, "wake done");
	trace_add(trace_line, __LINE__, ao_radio_abort, "wake abort");
}

static void
ao_radio_wait_done(void)
{
	ao_arch_block_interrupts();
	while (!ao_radio_done && !ao_radio_abort) {
		trace_add(trace_line, __LINE__, ao_radio_gpio_bits(), "wait_done");
		ao_sleep(&ao_radio_wake);
	}
	ao_arch_release_interrupts();
	trace_add(trace_line, __LINE__, ao_radio_gpio_bits(), "wake bits");
	trace_add(trace_line, __LINE__, ao_radio_fifo, "wake fifo");
	trace_add(trace_line, __LINE__, ao_radio_done, "wake done");
	trace_add(trace_line, __LINE__, ao_radio_abort, "wake abort");
}

static uint8_t	tx_data[(AO_RADIO_MAX_SEND + 4) * 2];

static uint8_t	*ao_radio_send_buf;
static int16_t	ao_radio_send_len;

static int16_t
ao_radio_send_fill(uint8_t *buf, int16_t len)
{
	int16_t this_time;

	this_time = ao_radio_send_len;
	if (this_time > len)
		this_time = len;
	memcpy(buf, ao_radio_send_buf, this_time);
	ao_radio_send_buf += this_time;
	ao_radio_send_len -= this_time;
	if (ao_radio_send_len == 0)
		return -this_time;
	return this_time;
}

void
ao_radio_send(const void *d, uint8_t size)
{
	ao_radio_get();
	ao_radio_send_len = ao_fec_encode(d, size, tx_data);
	ao_radio_send_buf = tx_data;
	_ao_radio_send_lots(ao_radio_send_fill, AO_RADIO_MODE_PACKET_TX);
	ao_radio_put();
}

#define AO_RADIO_LOTS	64

static void
_ao_radio_send_lots(ao_radio_fill_func fill, uint8_t mode)
{
	uint8_t	buf[AO_RADIO_LOTS], *b;
	int	cnt;
	int	total = 0;
	uint8_t	done = 0;
	uint8_t	started = 0;
	uint8_t	fifo_space;

	fifo_space = CC115L_FIFO_SIZE;
	ao_radio_abort = 0;

	ao_radio_strobe(CC115L_SFTX);

	ao_radio_done = 0;
	ao_radio_fifo = 0;
	while (!done) {
		cnt = (*fill)(buf, sizeof(buf));
		trace_add(trace_line, __LINE__, cnt, "send data count");
		if (cnt < 0) {
			done = 1;
			cnt = -cnt;
		}
		total += cnt;

		/* At the last buffer, set the total length */
		if (done) {
			ao_radio_set_len(total & 0xff);
			ao_radio_set_mode(mode | AO_RADIO_MODE_BITS_FIXED);
		} else {
			ao_radio_set_len(0xff);
			ao_radio_set_mode(mode | AO_RADIO_MODE_BITS_INFINITE);
		}

		b = buf;
		while (cnt) {
			uint8_t	this_len = cnt;

			/* Wait for some space in the fifo */
			while (!ao_radio_abort && (fifo_space = ao_radio_tx_fifo_space()) == 0) {
				trace_add(trace_line, __LINE__, this_len, "wait for space");
				ao_radio_wait_fifo();
			}
			if (ao_radio_abort || ao_radio_done)
				break;
			trace_add(trace_line, __LINE__, fifo_space, "got space");
			if (this_len > fifo_space)
				this_len = fifo_space;

			cnt -= this_len;

			ao_radio_done = 0;
			ao_radio_fifo = 0;
			ao_radio_fifo_write(b, this_len);
			b += this_len;

			ao_exti_enable(AO_CC115L_FIFO_INT_PORT, AO_CC115L_FIFO_INT_PIN);
			ao_exti_enable(AO_CC115L_DONE_INT_PORT, AO_CC115L_DONE_INT_PIN);

			if (!started) {
				ao_radio_stx();
				started = 1;
			}
		}
		if (ao_radio_abort || ao_radio_done)
			break;
	}
	if (ao_radio_abort)
		ao_radio_idle();
	ao_radio_wait_done();
	ao_radio_pa_off();
}

void
ao_radio_send_aprs(ao_radio_fill_func fill)
{
	ao_radio_get();
	_ao_radio_send_lots(fill, AO_RADIO_MODE_APRS);
	ao_radio_put();
}

#if CC115L_DEBUG
static const char *cc115l_state_name[] = {
	[CC115L_STATUS_STATE_IDLE] = "IDLE",
	[CC115L_STATUS_STATE_TX] = "TX",
	[CC115L_STATUS_STATE_FSTXON] = "FSTXON",
	[CC115L_STATUS_STATE_CALIBRATE] = "CALIBRATE",
	[CC115L_STATUS_STATE_SETTLING] = "SETTLING",
	[CC115L_STATUS_STATE_TX_FIFO_UNDERFLOW] = "TX_FIFO_UNDERFLOW",
};

static const struct ao_cc115l_reg ao_cc115l_reg[] = {
	{ .addr = CC115L_IOCFG2, .name = "IOCFG2" },
	{ .addr = CC115L_IOCFG1, .name = "IOCFG1" },
	{ .addr = CC115L_IOCFG0, .name = "IOCFG0" },
	{ .addr = CC115L_FIFOTHR, .name = "FIFOTHR" },
	{ .addr = CC115L_SYNC1, .name = "SYNC1" },
	{ .addr = CC115L_SYNC0, .name = "SYNC0" },
	{ .addr = CC115L_PKTLEN, .name = "PKTLEN" },
	{ .addr = CC115L_PKTCTRL0, .name = "PKTCTRL0" },
	{ .addr = CC115L_CHANNR, .name = "CHANNR" },
	{ .addr = CC115L_FSCTRL0, .name = "FSCTRL0" },
	{ .addr = CC115L_FREQ2, .name = "FREQ2" },
	{ .addr = CC115L_FREQ1, .name = "FREQ1" },
	{ .addr = CC115L_FREQ0, .name = "FREQ0" },
	{ .addr = CC115L_MDMCFG4, .name = "MDMCFG4" },
	{ .addr = CC115L_MDMCFG3, .name = "MDMCFG3" },
	{ .addr = CC115L_MDMCFG2, .name = "MDMCFG2" },
	{ .addr = CC115L_MDMCFG1, .name = "MDMCFG1" },
	{ .addr = CC115L_MDMCFG0, .name = "MDMCFG0" },
	{ .addr = CC115L_DEVIATN, .name = "DEVIATN" },
	{ .addr = CC115L_MCSM1, .name = "MCSM1" },
	{ .addr = CC115L_MCSM0, .name = "MCSM0" },
	{ .addr = CC115L_RESERVED_0X20, .name = "RESERVED_0X20" },
	{ .addr = CC115L_FREND0, .name = "FREND0" },
	{ .addr = CC115L_FSCAL3, .name = "FSCAL3" },
	{ .addr = CC115L_FSCAL2, .name = "FSCAL2" },
	{ .addr = CC115L_FSCAL1, .name = "FSCAL1" },
	{ .addr = CC115L_FSCAL0, .name = "FSCAL0" },
	{ .addr = CC115L_RESERVED_0X29, .name = "RESERVED_0X29" },
	{ .addr = CC115L_RESERVED_0X2A, .name = "RESERVED_0X2A" },
	{ .addr = CC115L_RESERVED_0X2B, .name = "RESERVED_0X2B" },
	{ .addr = CC115L_TEST2, .name = "TEST2" },
	{ .addr = CC115L_TEST1, .name = "TEST1" },
	{ .addr = CC115L_TEST0, .name = "TEST0" },
	{ .addr = CC115L_PARTNUM, .name = "PARTNUM" },
	{ .addr = CC115L_VERSION, .name = "VERSION" },
	{ .addr = CC115L_MARCSTATE, .name = "MARCSTATE" },
	{ .addr = CC115L_PKTSTATUS, .name = "PKTSTATUS" },
	{ .addr = CC115L_TXBYTES, .name = "TXBYTES" },
	{ .addr = CC115L_PA, .name = "PA" },
};

#define AO_NUM_CC115L_REG	(sizeof ao_cc115l_reg / sizeof ao_cc115l_reg[0])

static void ao_radio_show(void) {
	uint8_t	status = ao_radio_status();
	unsigned int	i;

	ao_radio_get();
	status = ao_radio_status();
	printf ("Status:   %02x\n", status);
	printf ("CHIP_RDY: %d\n", (status >> CC115L_STATUS_CHIP_RDY) & 1);
	printf ("STATE:    %s\n", cc115l_state_name[(status >> CC115L_STATUS_STATE) & CC115L_STATUS_STATE_MASK]);
	printf ("MARC:     %02x\n", ao_radio_get_marcstate());

	for (i = 0; i < AO_NUM_CC115L_REG; i++)
		printf ("\t%02x %-20.20s\n", ao_radio_reg_read(ao_cc115l_reg[i].addr), ao_cc115l_reg[i].name);
	ao_radio_put();
}

static void ao_radio_beep(void) {
	ao_radio_rdf();
}

static void ao_radio_packet(void) {
	static const uint8_t packet[] = {
#if 1
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
#else
		3, 1, 2, 3
#endif
	};

	ao_radio_send(packet, sizeof (packet));
}


#if HAS_APRS
#include <ao_aprs.h>

static void
ao_radio_aprs()
{
#if PACKET_HAS_SLAVE
	ao_packet_slave_stop();
#endif
	ao_aprs_send();
}
#endif
#endif /* CC115L_DEBUG */

static const struct ao_cmds ao_radio_cmds[] = {
	{ ao_radio_test_cmd,	"C <1 start, 0 stop, none both>\0Radio carrier test" },
#if CC115L_DEBUG
#if HAS_APRS
	{ ao_radio_aprs,	"G\0Send APRS packet" },
#endif
	{ ao_radio_show,	"R\0Show CC115L status" },
	{ ao_radio_beep,	"b\0Emit an RDF beacon" },
	{ ao_radio_packet,	"p\0Send a test packet" },
#endif
	{ 0, NULL }
};

void
ao_radio_init(void)
{
#if 0
	int	i;
#endif

	ao_radio_configured = 0;
	ao_spi_init_cs (AO_CC115L_SPI_CS_PORT, (1 << AO_CC115L_SPI_CS_PIN));

#if 0
	AO_CC115L_SPI_CS_PORT->bsrr = ((uint32_t) (1 << AO_CC115L_SPI_CS_PIN));
	for (i = 0; i < 10000; i++) {
		if ((SPI_2_PORT->idr & (1 << SPI_2_MISO_PIN)) == 0)
			break;
	}
	AO_CC115L_SPI_CS_PORT->bsrr = (1 << AO_CC115L_SPI_CS_PIN);
	if (i == 10000)
		ao_panic(AO_PANIC_SELF_TEST_CC115L);
#endif

	/* Enable the fifo threhold interrupt pin */
	ao_enable_port(AO_CC115L_FIFO_INT_PORT);
	ao_exti_setup(AO_CC115L_FIFO_INT_PORT, AO_CC115L_FIFO_INT_PIN,
		      AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_HIGH,
		      ao_radio_fifo_isr);

	/* Enable the tx done interrupt pin */
	ao_enable_port(AO_CC115L_DONE_INT_PORT);
	ao_exti_setup(AO_CC115L_DONE_INT_PORT, AO_CC115L_DONE_INT_PIN,
		      AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED,
		      ao_radio_done_isr);

	ao_radio_pa_init();

	ao_cmd_register(&ao_radio_cmds[0]);
}
