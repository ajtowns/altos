/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
#include <ao_cc1120.h>
#include <ao_exti.h>

uint8_t	ao_radio_done;
uint8_t ao_radio_mutex;
uint8_t ao_radio_abort;

#define CC1120_DEBUG	1

uint32_t	ao_radio_cal = 1186611;

#define ao_radio_select()	ao_spi_get_mask(AO_CC1120_SPI_CS_PORT,(1 << AO_CC1120_SPI_CS_PIN),AO_CC1120_SPI_BUS)
#define ao_radio_deselect()	ao_spi_put_mask(AO_CC1120_SPI_CS_PORT,(1 << AO_CC1120_SPI_CS_PIN),AO_CC1120_SPI_BUS)
#define ao_radio_spi_send(d,l)	ao_spi_send((d), (l), AO_CC1120_SPI_BUS)
#define ao_radio_spi_send_fixed(d,l) ao_spi_send_fixed((d), (l), AO_CC1120_SPI_BUS)
#define ao_radio_spi_recv(d,l)	ao_spi_recv((d), (l), AO_CC1120_SPI_BUS)
#define ao_radio_duplex(o,i,l)	ao_spi_duplex((o), (i), (l), AO_CC1120_SPI_BUS)

static uint8_t
ao_radio_reg_read(uint16_t addr)
{
	uint8_t	data[2];
	uint8_t	d;

#if CC1120_DEBUG
	printf("ao_radio_reg_read (%04x): ", addr); flush();
#endif
	if (CC1120_IS_EXTENDED(addr)) {
		data[0] = ((1 << CC1120_READ)  |
			   (0 << CC1120_BURST) |
			   CC1120_EXTENDED);
		data[1] = addr;
		d = 2;
	} else {
		data[0] = ((1 << CC1120_READ)  |
			   (0 << CC1120_BURST) |
			   addr);
		d = 1;
	}
	ao_radio_select();
	ao_radio_spi_send(data, d);
	ao_radio_spi_recv(data, 1);
	ao_radio_deselect();
#if CC1120_DEBUG
	printf (" %02x\n", data[0]);
#endif
	return data[0];
}

static void
ao_radio_reg_write(uint16_t addr, uint8_t value)
{
	uint8_t	data[3];
	uint8_t	d;

#if CC1120_DEBUG
	printf("ao_radio_reg_write (%04x): %02x\n", addr, value);
#endif
	if (CC1120_IS_EXTENDED(addr)) {
		data[0] = ((1 << CC1120_READ)  |
			   (0 << CC1120_BURST) |
			   CC1120_EXTENDED);
		data[1] = addr;
		d = 2;
	} else {
		data[0] = ((1 << CC1120_READ)  |
			   (0 << CC1120_BURST) |
			   addr);
		d = 1;
	}
	data[d] = value;
	ao_radio_select();
	ao_radio_spi_send(data, d+1);
	ao_radio_deselect();
}

static uint8_t
ao_radio_strobe(uint8_t addr)
{
	uint8_t	in;

	ao_radio_select();
	ao_radio_duplex(&addr, &in, 1);
	ao_radio_deselect();
	return in;
}

static uint8_t
ao_radio_fifo_read(uint8_t *data, uint8_t len)
{
	uint8_t	addr = ((1 << CC1120_READ)  |
			(1 << CC1120_BURST) |
			CC1120_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	ao_radio_spi_recv(data, len);
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_fifo_write(uint8_t *data, uint8_t len)
{
	uint8_t	addr = ((0 << CC1120_READ)  |
			(1 << CC1120_BURST) |
			CC1120_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	ao_radio_spi_send(data, len);
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_fifo_write_fixed(uint8_t data, uint8_t len)
{
	uint8_t	addr = ((0 << CC1120_READ)  |
			(1 << CC1120_BURST) |
			CC1120_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	ao_radio_spi_send_fixed(data, len);
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_status(void)
{
	return ao_radio_strobe (CC1120_SNOP);
}

void
ao_radio_recv_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_done);
}

#define ao_radio_rdf_value 0x55

static const uint16_t	rdf_setup[] = {
};

void
ao_radio_rdf(uint8_t len)
{
	int i;

	ao_radio_abort = 0;
	ao_radio_get(len);
	ao_radio_done = 0;
	for (i = 0; i < sizeof (rdf_setup) / sizeof (rdf_setup[0]); i += 2)
		ao_radio_reg_write(rdf_setup[i], rdf_setup[i+1]);
	ao_radio_fifo_write_fixed(ao_radio_rdf_value, len);
	ao_radio_reg_write(CC1120_IOCFG2, CC1120_IOCFG_GPIO_CFG_RX0TX1_CFG);
	ao_exti_enable(&AO_CC1120_INT_PORT, AO_CC1120_INT_PIN);
	ao_radio_strobe(CC1120_STX);
	cli();
	while (!ao_radio_done)
		ao_sleep(&ao_radio_done);
	sei();
	ao_radio_set_packet();
	ao_radio_put();
}

void
ao_radio_rdf_abort(void)
{
}

static void
ao_radio_test(void)
{
	uint8_t	mode = 2;
	uint8_t radio_on;
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
		ao_radio_get(0xff);
		ao_radio_strobe(CC1120_STX);
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

void
ao_radio_send(void *d, uint8_t size)
{
	ao_radio_get(size);
	ao_radio_done = 0;
	ao_radio_fifo_write(d, size);
	ao_radio_reg_write(CC1120_IOCFG2, CC1120_IOCFG_GPIO_CFG_RX0TX1_CFG);
	ao_exti_enable(&AO_CC1120_INT_PORT, AO_CC1120_INT_PIN);
	ao_radio_strobe(CC1120_STX);
	cli();
	while (!ao_radio_done)
		ao_sleep(&ao_radio_done);
	sei();
	ao_radio_put();
}

uint8_t
ao_radio_recv(__xdata void *d, uint8_t size)
{
	/* configure interrupt pin */
	ao_radio_get(size);
	ao_radio_done = 0;
	ao_radio_reg_write(CC1120_IOCFG2, CC1120_IOCFG_GPIO_CFG_RXFIFO_THR_PKT);
	ao_exti_enable(&AO_CC1120_INT_PORT, AO_CC1120_INT_PIN);
	ao_radio_strobe(CC1120_SRX);
	cli();
	while (!ao_radio_done && !ao_radio_abort)
		ao_sleep(&ao_radio_done);
	sei();
	if (ao_radio_done)
		ao_radio_fifo_read(d, size);
	ao_radio_put();
	return 0;
}

static const uint16_t packet_setup[] = {
};

void
ao_radio_set_packet(void)
{
	int i;

	for (i = 0; i < sizeof (rdf_setup) / sizeof (rdf_setup[0]); i += 2)
		ao_radio_reg_write(packet_setup[i], packet_setup[i+1]);
}

void
ao_radio_idle(void)
{
	for (;;) {
		uint8_t	state = ao_radio_strobe(CC1120_SIDLE);
		if ((state >> CC1120_STATUS_STATE) == CC1120_STATUS_STATE_IDLE)
			break;
	}
}

static const uint16_t radio_setup[] = {
#include "ao_cc1120_CC1120.h"
};

static uint8_t	ao_radio_configured = 0;

static void
ao_radio_isr(void)
{
	ao_exti_disable(&AO_CC1120_INT_PORT, AO_CC1120_INT_PIN);
	ao_radio_done = 1;
	ao_wakeup(&ao_radio_done);
}

static void
ao_radio_setup(void)
{
	int	i;

	for (i = 0; i < sizeof (radio_setup) / sizeof (radio_setup[0]); i += 2)
		ao_radio_reg_write(radio_setup[i], radio_setup[i+1]);

	/* Disable GPIO2 pin (radio_int) */
	ao_radio_reg_write(CC1120_IOCFG2, CC1120_IOCFG_GPIO_CFG_HIGHZ);

	/* Enable the EXTI interrupt for the appropriate pin */
	ao_enable_port(AO_CC1120_INT_PORT);
	ao_exti_setup(&AO_CC1120_INT_PORT, AO_CC1120_INT_PIN, AO_EXTI_MODE_RISING, ao_radio_isr);

	ao_radio_set_packet();
	ao_radio_configured = 1;
}

void
ao_radio_get(uint8_t len)
{
	ao_mutex_get(&ao_radio_mutex);
	if (!ao_radio_configured)
		ao_radio_setup();
	ao_radio_reg_write(CC1120_PKT_LEN, len);
}

#if CC1120_DEBUG
static char *cc1120_state_name[] = {
	[CC1120_STATUS_STATE_IDLE] = "IDLE",
	[CC1120_STATUS_STATE_RX] = "RX",
	[CC1120_STATUS_STATE_TX] = "TX",
	[CC1120_STATUS_STATE_FSTXON] = "FSTXON",
	[CC1120_STATUS_STATE_CALIBRATE] = "CALIBRATE",
	[CC1120_STATUS_STATE_SETTLING] = "SETTLING",
	[CC1120_STATUS_STATE_RX_FIFO_ERROR] = "RX_FIFO_ERROR",
	[CC1120_STATUS_STATE_TX_FIFO_ERROR] = "TX_FIFO_ERROR",
};

struct ao_cc1120_reg {
	uint16_t	addr;
	char		*name;
};

const static struct ao_cc1120_reg ao_cc1120_reg[] = {
	{ .addr = CC1120_IOCFG3,	.name = "IOCFG3" },
	{ .addr = CC1120_IOCFG2,	.name = "IOCFG2" },
	{ .addr = CC1120_IOCFG1,	.name = "IOCFG1" },
	{ .addr = CC1120_IOCFG0,	.name = "IOCFG0" },
	{ .addr = CC1120_SYNC3,	.name = "SYNC3" },
	{ .addr = CC1120_SYNC2,	.name = "SYNC2" },
	{ .addr = CC1120_SYNC1,	.name = "SYNC1" },
	{ .addr = CC1120_SYNC0,	.name = "SYNC0" },
	{ .addr = CC1120_SYNC_CFG1,	.name = "SYNC_CFG1" },
	{ .addr = CC1120_SYNC_CFG0,	.name = "SYNC_CFG0" },
	{ .addr = CC1120_DEVIATION_M,	.name = "DEVIATION_M" },
	{ .addr = CC1120_MODCFG_DEV_E,	.name = "MODCFG_DEV_E" },
	{ .addr = CC1120_DCFILT_CFG,	.name = "DCFILT_CFG" },
	{ .addr = CC1120_PREAMBLE_CFG1,	.name = "PREAMBLE_CFG1" },
	{ .addr = CC1120_PREAMBLE_CFG0,	.name = "PREAMBLE_CFG0" },
	{ .addr = CC1120_FREQ_IF_CFG,	.name = "FREQ_IF_CFG" },
	{ .addr = CC1120_IQIC,	.name = "IQIC" },
	{ .addr = CC1120_CHAN_BW,	.name = "CHAN_BW" },
	{ .addr = CC1120_MDMCFG1,	.name = "MDMCFG1" },
	{ .addr = CC1120_MDMCFG0,	.name = "MDMCFG0" },
	{ .addr = CC1120_DRATE2,	.name = "DRATE2" },
	{ .addr = CC1120_DRATE1,	.name = "DRATE1" },
	{ .addr = CC1120_DRATE0,	.name = "DRATE0" },
	{ .addr = CC1120_AGC_REF,	.name = "AGC_REF" },
	{ .addr = CC1120_AGC_CS_THR,	.name = "AGC_CS_THR" },
	{ .addr = CC1120_AGC_GAIN_ADJUST,	.name = "AGC_GAIN_ADJUST" },
	{ .addr = CC1120_AGC_CFG3,	.name = "AGC_CFG3" },
	{ .addr = CC1120_AGC_CFG2,	.name = "AGC_CFG2" },
	{ .addr = CC1120_AGC_CFG1,	.name = "AGC_CFG1" },
	{ .addr = CC1120_AGC_CFG0,	.name = "AGC_CFG0" },
	{ .addr = CC1120_FIFO_CFG,	.name = "FIFO_CFG" },
	{ .addr = CC1120_DEV_ADDR,	.name = "DEV_ADDR" },
	{ .addr = CC1120_SETTLING_CFG,	.name = "SETTLING_CFG" },
	{ .addr = CC1120_FS_CFG,	.name = "FS_CFG" },
	{ .addr = CC1120_WOR_CFG1,	.name = "WOR_CFG1" },
	{ .addr = CC1120_WOR_CFG0,	.name = "WOR_CFG0" },
	{ .addr = CC1120_WOR_EVENT0_MSB,	.name = "WOR_EVENT0_MSB" },
	{ .addr = CC1120_WOR_EVENT0_LSB,	.name = "WOR_EVENT0_LSB" },
	{ .addr = CC1120_PKT_CFG2,	.name = "PKT_CFG2" },
	{ .addr = CC1120_PKT_CFG1,	.name = "PKT_CFG1" },
	{ .addr = CC1120_PKT_CFG0,	.name = "PKT_CFG0" },
	{ .addr = CC1120_RFEND_CFG1,	.name = "RFEND_CFG1" },
	{ .addr = CC1120_RFEND_CFG0,	.name = "RFEND_CFG0" },
	{ .addr = CC1120_PA_CFG2,	.name = "PA_CFG2" },
	{ .addr = CC1120_PA_CFG1,	.name = "PA_CFG1" },
	{ .addr = CC1120_PA_CFG0,	.name = "PA_CFG0" },
	{ .addr = CC1120_PKT_LEN,	.name = "PKT_LEN" },
	{ .addr = CC1120_IF_MIX_CFG,	.name = "IF_MIX_CFG" },
	{ .addr = CC1120_FREQOFF_CFG,	.name = "FREQOFF_CFG" },
	{ .addr = CC1120_TOC_CFG,	.name = "TOC_CFG" },
	{ .addr = CC1120_MARC_SPARE,	.name = "MARC_SPARE" },
	{ .addr = CC1120_ECG_CFG,	.name = "ECG_CFG" },
	{ .addr = CC1120_SOFT_TX_DATA_CFG,	.name = "SOFT_TX_DATA_CFG" },
	{ .addr = CC1120_EXT_CTRL,	.name = "EXT_CTRL" },
	{ .addr = CC1120_RCCAL_FINE,	.name = "RCCAL_FINE" },
	{ .addr = CC1120_RCCAL_COARSE,	.name = "RCCAL_COARSE" },
	{ .addr = CC1120_RCCAL_OFFSET,	.name = "RCCAL_OFFSET" },
	{ .addr = CC1120_FREQOFF1,	.name = "FREQOFF1" },
	{ .addr = CC1120_FREQOFF0,	.name = "FREQOFF0" },
	{ .addr = CC1120_FREQ2,	.name = "FREQ2" },
	{ .addr = CC1120_FREQ1,	.name = "FREQ1" },
	{ .addr = CC1120_FREQ0,	.name = "FREQ0" },
	{ .addr = CC1120_IF_ADC2,	.name = "IF_ADC2" },
	{ .addr = CC1120_IF_ADC1,	.name = "IF_ADC1" },
	{ .addr = CC1120_IF_ADC0,	.name = "IF_ADC0" },
	{ .addr = CC1120_FS_DIG1,	.name = "FS_DIG1" },
	{ .addr = CC1120_FS_DIG0,	.name = "FS_DIG0" },
	{ .addr = CC1120_FS_CAL3,	.name = "FS_CAL3" },
	{ .addr = CC1120_FS_CAL2,	.name = "FS_CAL2" },
	{ .addr = CC1120_FS_CAL1,	.name = "FS_CAL1" },
	{ .addr = CC1120_FS_CAL0,	.name = "FS_CAL0" },
	{ .addr = CC1120_FS_CHP,	.name = "FS_CHP" },
	{ .addr = CC1120_FS_DIVTWO,	.name = "FS_DIVTWO" },
	{ .addr = CC1120_FS_DSM1,	.name = "FS_DSM1" },
	{ .addr = CC1120_FS_DSM0,	.name = "FS_DSM0" },
	{ .addr = CC1120_FS_DVC1,	.name = "FS_DVC1" },
	{ .addr = CC1120_FS_DVC0,	.name = "FS_DVC0" },
	{ .addr = CC1120_FS_LBI,	.name = "FS_LBI" },
	{ .addr = CC1120_FS_PFD,	.name = "FS_PFD" },
	{ .addr = CC1120_FS_PRE,	.name = "FS_PRE" },
	{ .addr = CC1120_FS_REG_DIV_CML,	.name = "FS_REG_DIV_CML" },
	{ .addr = CC1120_FS_SPARE,	.name = "FS_SPARE" },
	{ .addr = CC1120_FS_VCO4,	.name = "FS_VCO4" },
	{ .addr = CC1120_FS_VCO3,	.name = "FS_VCO3" },
	{ .addr = CC1120_FS_VCO2,	.name = "FS_VCO2" },
	{ .addr = CC1120_FS_VCO1,	.name = "FS_VCO1" },
	{ .addr = CC1120_FS_VCO0,	.name = "FS_VCO0" },
	{ .addr = CC1120_GBIAS6,	.name = "GBIAS6" },
	{ .addr = CC1120_GBIAS5,	.name = "GBIAS5" },
	{ .addr = CC1120_GBIAS4,	.name = "GBIAS4" },
	{ .addr = CC1120_GBIAS3,	.name = "GBIAS3" },
	{ .addr = CC1120_GBIAS2,	.name = "GBIAS2" },
	{ .addr = CC1120_GBIAS1,	.name = "GBIAS1" },
	{ .addr = CC1120_GBIAS0,	.name = "GBIAS0" },
	{ .addr = CC1120_IFAMP,	.name = "IFAMP" },
	{ .addr = CC1120_LNA,	.name = "LNA" },
	{ .addr = CC1120_RXMIX,	.name = "RXMIX" },
	{ .addr = CC1120_XOSC5,	.name = "XOSC5" },
	{ .addr = CC1120_XOSC4,	.name = "XOSC4" },
	{ .addr = CC1120_XOSC3,	.name = "XOSC3" },
	{ .addr = CC1120_XOSC2,	.name = "XOSC2" },
	{ .addr = CC1120_XOSC1,	.name = "XOSC1" },
	{ .addr = CC1120_XOSC0,	.name = "XOSC0" },
	{ .addr = CC1120_ANALOG_SPARE,	.name = "ANALOG_SPARE" },
	{ .addr = CC1120_PA_CFG3,	.name = "PA_CFG3" },
	{ .addr = CC1120_WOR_TIME1,	.name = "WOR_TIME1" },
	{ .addr = CC1120_WOR_TIME0,	.name = "WOR_TIME0" },
	{ .addr = CC1120_WOR_CAPTURE1,	.name = "WOR_CAPTURE1" },
	{ .addr = CC1120_WOR_CAPTURE0,	.name = "WOR_CAPTURE0" },
	{ .addr = CC1120_BIST,	.name = "BIST" },
	{ .addr = CC1120_DCFILTOFFSET_I1,	.name = "DCFILTOFFSET_I1" },
	{ .addr = CC1120_DCFILTOFFSET_I0,	.name = "DCFILTOFFSET_I0" },
	{ .addr = CC1120_DCFILTOFFSET_Q1,	.name = "DCFILTOFFSET_Q1" },
	{ .addr = CC1120_DCFILTOFFSET_Q0,	.name = "DCFILTOFFSET_Q0" },
	{ .addr = CC1120_IQIE_I1,	.name = "IQIE_I1" },
	{ .addr = CC1120_IQIE_I0,	.name = "IQIE_I0" },
	{ .addr = CC1120_IQIE_Q1,	.name = "IQIE_Q1" },
	{ .addr = CC1120_IQIE_Q0,	.name = "IQIE_Q0" },
	{ .addr = CC1120_RSSI1,	.name = "RSSI1" },
	{ .addr = CC1120_RSSI0,	.name = "RSSI0" },
	{ .addr = CC1120_MARCSTATE,	.name = "MARCSTATE" },
	{ .addr = CC1120_LQI_VAL,	.name = "LQI_VAL" },
	{ .addr = CC1120_PQT_SYNC_ERR,	.name = "PQT_SYNC_ERR" },
	{ .addr = CC1120_DEM_STATUS,	.name = "DEM_STATUS" },
	{ .addr = CC1120_FREQOFF_EST1,	.name = "FREQOFF_EST1" },
	{ .addr = CC1120_FREQOFF_EST0,	.name = "FREQOFF_EST0" },
	{ .addr = CC1120_AGC_GAIN3,	.name = "AGC_GAIN3" },
	{ .addr = CC1120_AGC_GAIN2,	.name = "AGC_GAIN2" },
	{ .addr = CC1120_AGC_GAIN1,	.name = "AGC_GAIN1" },
	{ .addr = CC1120_AGC_GAIN0,	.name = "AGC_GAIN0" },
	{ .addr = CC1120_SOFT_RX_DATA_OUT,	.name = "SOFT_RX_DATA_OUT" },
	{ .addr = CC1120_SOFT_TX_DATA_IN,	.name = "SOFT_TX_DATA_IN" },
	{ .addr = CC1120_ASK_SOFT_RX_DATA,	.name = "ASK_SOFT_RX_DATA" },
	{ .addr = CC1120_RNDGEN,	.name = "RNDGEN" },
	{ .addr = CC1120_MAGN2,	.name = "MAGN2" },
	{ .addr = CC1120_MAGN1,	.name = "MAGN1" },
	{ .addr = CC1120_MAGN0,	.name = "MAGN0" },
	{ .addr = CC1120_ANG1,	.name = "ANG1" },
	{ .addr = CC1120_ANG0,	.name = "ANG0" },
	{ .addr = CC1120_CHFILT_I2,	.name = "CHFILT_I2" },
	{ .addr = CC1120_CHFILT_I1,	.name = "CHFILT_I1" },
	{ .addr = CC1120_CHFILT_I0,	.name = "CHFILT_I0" },
	{ .addr = CC1120_CHFILT_Q2,	.name = "CHFILT_Q2" },
	{ .addr = CC1120_CHFILT_Q1,	.name = "CHFILT_Q1" },
	{ .addr = CC1120_CHFILT_Q0,	.name = "CHFILT_Q0" },
	{ .addr = CC1120_GPIO_STATUS,	.name = "GPIO_STATUS" },
	{ .addr = CC1120_FSCAL_CTRL,	.name = "FSCAL_CTRL" },
	{ .addr = CC1120_PHASE_ADJUST,	.name = "PHASE_ADJUST" },
	{ .addr = CC1120_PARTNUMBER,	.name = "PARTNUMBER" },
	{ .addr = CC1120_PARTVERSION,	.name = "PARTVERSION" },
	{ .addr = CC1120_SERIAL_STATUS,	.name = "SERIAL_STATUS" },
	{ .addr = CC1120_RX_STATUS,	.name = "RX_STATUS" },
	{ .addr = CC1120_TX_STATUS,	.name = "TX_STATUS" },
	{ .addr = CC1120_MARC_STATUS1,	.name = "MARC_STATUS1" },
	{ .addr = CC1120_MARC_STATUS0,	.name = "MARC_STATUS0" },
	{ .addr = CC1120_PA_IFAMP_TEST,	.name = "PA_IFAMP_TEST" },
	{ .addr = CC1120_FSRF_TEST,	.name = "FSRF_TEST" },
	{ .addr = CC1120_PRE_TEST,	.name = "PRE_TEST" },
	{ .addr = CC1120_PRE_OVR,	.name = "PRE_OVR" },
	{ .addr = CC1120_ADC_TEST,	.name = "ADC_TEST" },
	{ .addr = CC1120_DVC_TEST,	.name = "DVC_TEST" },
	{ .addr = CC1120_ATEST,	.name = "ATEST" },
	{ .addr = CC1120_ATEST_LVDS,	.name = "ATEST_LVDS" },
	{ .addr = CC1120_ATEST_MODE,	.name = "ATEST_MODE" },
	{ .addr = CC1120_XOSC_TEST1,	.name = "XOSC_TEST1" },
	{ .addr = CC1120_XOSC_TEST0,	.name = "XOSC_TEST0" },
	{ .addr = CC1120_RXFIRST,	.name = "RXFIRST" },
	{ .addr = CC1120_TXFIRST,	.name = "TXFIRST" },
	{ .addr = CC1120_RXLAST,	.name = "RXLAST" },
	{ .addr = CC1120_TXLAST,	.name = "TXLAST" },
	{ .addr = CC1120_NUM_TXBYTES,	.name = "NUM_TXBYTES" },
	{ .addr = CC1120_NUM_RXBYTES,	.name = "NUM_RXBYTES" },
	{ .addr = CC1120_FIFO_NUM_TXBYTES,	.name = "FIFO_NUM_TXBYTES" },
	{ .addr = CC1120_FIFO_NUM_RXBYTES,	.name = "FIFO_NUM_RXBYTES" },
};

#define AO_NUM_CC1120_REG	(sizeof ao_cc1120_reg / sizeof ao_cc1120_reg[0])

static void ao_radio_show(void) {
	uint8_t	status = ao_radio_status();
	int	i;

	ao_radio_get(0xff);
	status = ao_radio_status();
	printf ("Status:   %02x\n", status);
	printf ("CHIP_RDY: %d\n", (status >> CC1120_STATUS_CHIP_RDY) & 1);
	printf ("STATE:    %s\n", cc1120_state_name[(status >> CC1120_STATUS_STATE) & CC1120_STATUS_STATE_MASK]);

	for (i = 0; i < AO_NUM_CC1120_REG; i++)
		printf ("\t%02x %-20.20s\n", ao_radio_reg_read(ao_cc1120_reg[i].addr), ao_cc1120_reg[i].name);
	ao_radio_put();
}
#endif

static const struct ao_cmds ao_radio_cmds[] = {
	{ ao_radio_test,	"C <1 start, 0 stop, none both>\0Radio carrier test" },
#if CC1120_DEBUG
	{ ao_radio_show,	"R\0Show CC1120 status" },
#endif
	{ 0, NULL }
};

void
ao_radio_init(void)
{
	ao_radio_configured = 0;
	ao_spi_init_cs (AO_CC1120_SPI_CS_PORT, (1 << AO_CC1120_SPI_CS_PIN));

	ao_cmd_register(&ao_radio_cmds[0]);
}
