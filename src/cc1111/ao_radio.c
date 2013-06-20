/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "ao.h"
#if HAS_PAD
#include <ao_pad.h>
#endif

/* Values from SmartRF® Studio for:
 *
 * Deviation:	20.507812 kHz
 * Datarate:	38.360596 kBaud
 * Modulation:	GFSK
 * RF Freq:	434.549927 MHz
 * Channel:	99.975586 kHz
 * Channel:	0
 * RX filter:	93.75 kHz
 */

/*
 * For IF freq of 140.62kHz, the IF value is:
 *
 * 140.62e3 / (24e6 / 2**10) = 6
 */

#define IF_FREQ_CONTROL	6

/*
 * For channel bandwidth of 93.75 kHz, the CHANBW_E and CHANBW_M values are
 *
 * BW = 24e6 / (8 * (4 + M) * 2 ** E)
 *
 * So, M = 0 and E = 3
 */

#define CHANBW_M	0
#define CHANBW_E	3

/*
 * For a symbol rate of 38360kBaud, the DRATE_E and DRATE_M values are:
 *
 * R = (256 + M) * 2** E * 24e6 / 2**28
 *
 * So M is 163 and E is 10
 */

#define DRATE_E		10
#define DRATE_M		163

/*
 * For a channel deviation of 20.5kHz, the DEVIATION_E and DEVIATION_M values are:
 *
 * F = 24e6/2**17 * (8 + DEVIATION_M) * 2**DEVIATION_E
 *
 * So M is 6 and E is 3
 */

#define DEVIATION_M	6
#define DEVIATION_E	3

/*
 * For our RDF beacon, set the symbol rate to 2kBaud (for a 1kHz tone),
 * so the DRATE_E and DRATE_M values are:
 *
 * M is 94 and E is 6
 *
 * To make the tone last for 200ms, we need 2000 * .2 = 400 bits or 50 bytes
 */

#define RDF_DRATE_E	6
#define RDF_DRATE_M	94
#define RDF_PACKET_LEN	50

/*
 * RDF deviation should match the normal NFM value of 5kHz
 *
 * M is 6 and E is 1
 *
 */

#define RDF_DEVIATION_M	6
#define RDF_DEVIATION_E	1

/* This are from the table for 433MHz */

#define RF_POWER_M30_DBM	0x12
#define RF_POWER_M20_DBM	0x0e
#define RF_POWER_M15_DBM	0x1d
#define RF_POWER_M10_DBM	0x34
#define RF_POWER_M5_DBM		0x2c
#define RF_POWER_0_DBM		0x60
#define RF_POWER_5_DBM		0x84
#define RF_POWER_7_DBM		0xc8
#define RF_POWER_10_DBM		0xc0

#define RF_POWER		RF_POWER_10_DBM

static __code uint8_t radio_setup[] = {
	RF_PA_TABLE7_OFF,	RF_POWER,
	RF_PA_TABLE6_OFF,	RF_POWER,
	RF_PA_TABLE5_OFF,	RF_POWER,
	RF_PA_TABLE4_OFF,	RF_POWER,
	RF_PA_TABLE3_OFF,	RF_POWER,
	RF_PA_TABLE2_OFF,	RF_POWER,
	RF_PA_TABLE1_OFF,	RF_POWER,
	RF_PA_TABLE0_OFF,	RF_POWER,

	RF_FSCTRL1_OFF,		(IF_FREQ_CONTROL << RF_FSCTRL1_FREQ_IF_SHIFT),
	RF_FSCTRL0_OFF,		(0 << RF_FSCTRL0_FREQOFF_SHIFT),

	RF_MDMCFG4_OFF,		((CHANBW_E << RF_MDMCFG4_CHANBW_E_SHIFT) |
				 (CHANBW_M << RF_MDMCFG4_CHANBW_M_SHIFT) |
				 (DRATE_E << RF_MDMCFG4_DRATE_E_SHIFT)),
	RF_MDMCFG3_OFF,		(DRATE_M << RF_MDMCFG3_DRATE_M_SHIFT),
	RF_MDMCFG2_OFF,		(RF_MDMCFG2_DEM_DCFILT_OFF |
				 RF_MDMCFG2_MOD_FORMAT_GFSK |
				 RF_MDMCFG2_SYNC_MODE_15_16_THRES),
	RF_MDMCFG1_OFF,		(RF_MDMCFG1_FEC_EN |
				 RF_MDMCFG1_NUM_PREAMBLE_4 |
				 (2 << RF_MDMCFG1_CHANSPC_E_SHIFT)),
	RF_MDMCFG0_OFF,		(17 << RF_MDMCFG0_CHANSPC_M_SHIFT),

	RF_CHANNR_OFF,		0,

	RF_DEVIATN_OFF,		((DEVIATION_E << RF_DEVIATN_DEVIATION_E_SHIFT) |
				 (DEVIATION_M << RF_DEVIATN_DEVIATION_M_SHIFT)),

	/* SmartRF says set LODIV_BUF_CURRENT_TX to 0
	 * And, we're not using power ramping, so use PA_POWER 0
	 */
	RF_FREND0_OFF,		((1 << RF_FREND0_LODIV_BUF_CURRENT_TX_SHIFT) |
				 (0 << RF_FREND0_PA_POWER_SHIFT)),

	RF_FREND1_OFF,		((1 << RF_FREND1_LNA_CURRENT_SHIFT) |
				 (1 << RF_FREND1_LNA2MIX_CURRENT_SHIFT) |
				 (1 << RF_FREND1_LODIV_BUF_CURRENT_RX_SHIFT) |
				 (2 << RF_FREND1_MIX_CURRENT_SHIFT)),

	RF_FSCAL3_OFF,		0xE9,
	RF_FSCAL2_OFF,		0x0A,
	RF_FSCAL1_OFF,		0x00,
	RF_FSCAL0_OFF,		0x1F,

	RF_TEST2_OFF,		0x88,
	RF_TEST1_OFF,		0x31,
	RF_TEST0_OFF,		0x09,

	/* default sync values */
	RF_SYNC1_OFF,		0xD3,
	RF_SYNC0_OFF,		0x91,

	/* max packet length */
	RF_PKTCTRL1_OFF,	((1 << PKTCTRL1_PQT_SHIFT)|
				 PKTCTRL1_APPEND_STATUS|
				 PKTCTRL1_ADR_CHK_NONE),
	RF_PKTCTRL0_OFF,	(RF_PKTCTRL0_WHITE_DATA|
				 RF_PKTCTRL0_PKT_FORMAT_NORMAL|
				 RF_PKTCTRL0_CRC_EN|
				 RF_PKTCTRL0_LENGTH_CONFIG_FIXED),
	RF_ADDR_OFF,		0x00,
	RF_MCSM2_OFF,		(RF_MCSM2_RX_TIME_END_OF_PACKET),
	RF_MCSM1_OFF,		(RF_MCSM1_CCA_MODE_RSSI_BELOW_UNLESS_RECEIVING|
				 RF_MCSM1_RXOFF_MODE_IDLE|
				 RF_MCSM1_TXOFF_MODE_IDLE),
	RF_MCSM0_OFF,		(RF_MCSM0_FS_AUTOCAL_FROM_IDLE|
				 RF_MCSM0_MAGIC_3|
				 RF_MCSM0_CLOSE_IN_RX_0DB),
	RF_FOCCFG_OFF,		(RF_FOCCFG_FOC_PRE_K_3K,
				 RF_FOCCFG_FOC_POST_K_PRE_K,
				 RF_FOCCFG_FOC_LIMIT_BW_OVER_4),
	RF_BSCFG_OFF,		(RF_BSCFG_BS_PRE_K_2K|
				 RF_BSCFG_BS_PRE_KP_3KP|
				 RF_BSCFG_BS_POST_KI_PRE_KI|
				 RF_BSCFG_BS_POST_KP_PRE_KP|
				 RF_BSCFG_BS_LIMIT_0),
	RF_AGCCTRL2_OFF,	0x03,
	RF_AGCCTRL1_OFF,	0x40,
	RF_AGCCTRL0_OFF,	0x91,

	RF_IOCFG2_OFF,		0x00,
	RF_IOCFG1_OFF,		0x00,
	RF_IOCFG0_OFF,		0x00,
};

static __code uint8_t rdf_setup[] = {
	RF_MDMCFG4_OFF,		((CHANBW_E << RF_MDMCFG4_CHANBW_E_SHIFT) |
				 (CHANBW_M << RF_MDMCFG4_CHANBW_M_SHIFT) |
				 (RDF_DRATE_E << RF_MDMCFG4_DRATE_E_SHIFT)),
	RF_MDMCFG3_OFF,		(RDF_DRATE_M << RF_MDMCFG3_DRATE_M_SHIFT),
	RF_MDMCFG2_OFF,		(RF_MDMCFG2_DEM_DCFILT_OFF |
				 RF_MDMCFG2_MOD_FORMAT_GFSK |
				 RF_MDMCFG2_SYNC_MODE_NONE),
	RF_MDMCFG1_OFF,		(RF_MDMCFG1_FEC_DIS |
				 RF_MDMCFG1_NUM_PREAMBLE_2 |
				 (2 << RF_MDMCFG1_CHANSPC_E_SHIFT)),

	RF_DEVIATN_OFF,		((RDF_DEVIATION_E << RF_DEVIATN_DEVIATION_E_SHIFT) |
				 (RDF_DEVIATION_M << RF_DEVIATN_DEVIATION_M_SHIFT)),

	/* packet length is set in-line */
	RF_PKTCTRL1_OFF,	((1 << PKTCTRL1_PQT_SHIFT)|
				 PKTCTRL1_ADR_CHK_NONE),
	RF_PKTCTRL0_OFF,	(RF_PKTCTRL0_PKT_FORMAT_NORMAL|
				 RF_PKTCTRL0_LENGTH_CONFIG_FIXED),
};

static __code uint8_t fixed_pkt_setup[] = {
	RF_MDMCFG4_OFF,		((CHANBW_E << RF_MDMCFG4_CHANBW_E_SHIFT) |
				 (CHANBW_M << RF_MDMCFG4_CHANBW_M_SHIFT) |
				 (DRATE_E << RF_MDMCFG4_DRATE_E_SHIFT)),
	RF_MDMCFG3_OFF,		(DRATE_M << RF_MDMCFG3_DRATE_M_SHIFT),
	RF_MDMCFG2_OFF,		(RF_MDMCFG2_DEM_DCFILT_OFF |
				 RF_MDMCFG2_MOD_FORMAT_GFSK |
				 RF_MDMCFG2_SYNC_MODE_15_16_THRES),
	RF_MDMCFG1_OFF,		(RF_MDMCFG1_FEC_EN |
				 RF_MDMCFG1_NUM_PREAMBLE_4 |
				 (2 << RF_MDMCFG1_CHANSPC_E_SHIFT)),

	RF_DEVIATN_OFF,		((DEVIATION_E << RF_DEVIATN_DEVIATION_E_SHIFT) |
				 (DEVIATION_M << RF_DEVIATN_DEVIATION_M_SHIFT)),

	/* max packet length -- now set inline */
	RF_PKTCTRL1_OFF,	((1 << PKTCTRL1_PQT_SHIFT)|
				 PKTCTRL1_APPEND_STATUS|
				 PKTCTRL1_ADR_CHK_NONE),
	RF_PKTCTRL0_OFF,	(RF_PKTCTRL0_WHITE_DATA|
				 RF_PKTCTRL0_PKT_FORMAT_NORMAL|
				 RF_PKTCTRL0_CRC_EN|
				 RF_PKTCTRL0_LENGTH_CONFIG_FIXED),
};

__xdata uint8_t	ao_radio_dma;
__xdata uint8_t ao_radio_dma_done;
__xdata uint8_t ao_radio_done;
__xdata uint8_t ao_radio_abort;
__xdata uint8_t ao_radio_mutex;

#if PACKET_HAS_MASTER || HAS_AES
#define NEED_RADIO_RSSI 1
#endif

#ifndef NEED_RADIO_RSSI
#define NEED_RADIO_RSSI 0
#endif

#if NEED_RADIO_RSSI
__xdata int8_t ao_radio_rssi;
#endif

void
ao_radio_general_isr(void) __interrupt 16
{
	S1CON &= ~0x03;
	if (RFIF & RFIF_IM_TIMEOUT) {
		ao_radio_recv_abort();
		RFIF &= ~ RFIF_IM_TIMEOUT;
	} else if (RFIF & RFIF_IM_DONE) {
		ao_radio_done = 1;
		ao_wakeup(&ao_radio_done);
		RFIF &= ~RFIF_IM_DONE;
	}
}

static void
ao_radio_set_packet(void)
{
	uint8_t	i;
	for (i = 0; i < sizeof (fixed_pkt_setup); i += 2)
		RF[fixed_pkt_setup[i]] = fixed_pkt_setup[i+1];
}

static void
ao_radio_idle(void)
{
	if (RF_MARCSTATE != RF_MARCSTATE_IDLE)
	{
		do {
			RFST = RFST_SIDLE;
			ao_yield();
		} while (RF_MARCSTATE != RF_MARCSTATE_IDLE);
	}
}

#define ao_radio_put() ao_mutex_put(&ao_radio_mutex)

static void
ao_radio_get(uint8_t len)
{
	ao_config_get();
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	RF_CHANNR = 0;
	RF_FREQ2 = (uint8_t) (ao_config.radio_setting >> 16);
	RF_FREQ1 = (uint8_t) (ao_config.radio_setting >> 8);
	RF_FREQ0 = (uint8_t) (ao_config.radio_setting);
	RF_PKTLEN = len;
}


void
ao_radio_send(__xdata void *packet, uint8_t size) __reentrant
{
	ao_radio_get(size);
	ao_radio_done = 0;
	ao_dma_set_transfer(ao_radio_dma,
			    packet,
			    &RFDXADDR,
			    size,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_STX;
	__critical while (!ao_radio_done)
		ao_sleep(&ao_radio_done);
	ao_radio_put();
}

uint8_t
ao_radio_recv(__xdata void *packet, uint8_t size, uint8_t timeout) __reentrant
{
	ao_radio_abort = 0;
	ao_radio_get(size - 2);
	ao_dma_set_transfer(ao_radio_dma,
			    &RFDXADDR,
			    packet,
			    size,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_SRX;

	/* Wait for DMA to be done, for the radio receive process to
	 * get aborted or for a receive timeout to fire
	 */
	if (timeout)
		ao_alarm(timeout);
	__critical while (!ao_radio_dma_done && !ao_radio_abort)
			   if (ao_sleep(&ao_radio_dma_done))
				   break;
	if (timeout)
		ao_clear_alarm();

	/* If recv was aborted, clean up by stopping the DMA engine
	 * and idling the radio
	 */
	if (!ao_radio_dma_done) {
		ao_dma_abort(ao_radio_dma);
		ao_radio_idle();
#if NEED_RADIO_RSSI
		ao_radio_rssi = 0;
#endif
	}
#if NEED_RADIO_RSSI
	else
		ao_radio_rssi = AO_RSSI_FROM_RADIO(((uint8_t *)packet)[size - 2]);
#endif
	ao_radio_put();
	return ao_radio_dma_done;
}

/*
 * Wake up a task waiting to receive a radio packet
 * and tell them to abort the transfer
 */

void
ao_radio_recv_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_dma_done);
}

__code ao_radio_rdf_value = 0x55;

static void
ao_radio_rdf_start(void)
{
	uint8_t i;
	ao_radio_abort = 0;
	ao_radio_get(AO_RADIO_RDF_LEN);
	ao_radio_done = 0;
	for (i = 0; i < sizeof (rdf_setup); i += 2)
		RF[rdf_setup[i]] = rdf_setup[i+1];
}

static void
ao_radio_rdf_run(void)
{
	ao_dma_start(ao_radio_dma);
	RFST = RFST_STX;
	__critical while (!ao_radio_done && !ao_radio_abort)
			   ao_sleep(&ao_radio_done);
	if (!ao_radio_done) {
		ao_dma_abort(ao_radio_dma);
		ao_radio_idle();
	}
	ao_radio_set_packet();
	ao_radio_put();
}

void
ao_radio_rdf(void)
{
	ao_radio_rdf_start();

	ao_dma_set_transfer(ao_radio_dma,
			    CODE_TO_XDATA(&ao_radio_rdf_value),
			    &RFDXADDR,
			    AO_RADIO_RDF_LEN,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_radio_rdf_run();
}

#define PA	0x00
#define BE	0x55

#define CONT_PAUSE_8	PA, PA, PA, PA, PA, PA, PA, PA
#define CONT_PAUSE_16	CONT_PAUSE_8, CONT_PAUSE_8
#define CONT_PAUSE_24	CONT_PAUSE_16, CONT_PAUSE_8

#define CONT_BEEP_8	BE, BE, BE, BE, BE, BE, BE, BE

#if AO_RADIO_CONT_PAUSE_LEN == 24
#define CONT_PAUSE	CONT_PAUSE_24
#endif

#if AO_RADIO_CONT_TONE_LEN == 8
#define CONT_BEEP		CONT_BEEP_8
#define CONT_PAUSE_SHORT	CONT_PAUSE_8
#endif

#define CONT_ADDR(c)	CODE_TO_XDATA(&ao_radio_cont[(3-(c)) * (AO_RADIO_CONT_PAUSE_LEN + AO_RADIO_CONT_TONE_LEN)])

__code uint8_t ao_radio_cont[] = {
	CONT_PAUSE, CONT_BEEP,
	CONT_PAUSE, CONT_BEEP,
	CONT_PAUSE, CONT_BEEP,
	CONT_PAUSE, CONT_PAUSE_SHORT,
	CONT_PAUSE, CONT_PAUSE_SHORT,
	CONT_PAUSE,
};

void
ao_radio_continuity(uint8_t c)
{
	ao_radio_rdf_start();
	ao_dma_set_transfer(ao_radio_dma,
			    CONT_ADDR(c),
			    &RFDXADDR,
			    AO_RADIO_CONT_TOTAL_LEN,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_radio_rdf_run();
}

void
ao_radio_rdf_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_done);
}


/* Output carrier */

static __xdata	ao_radio_test_on;

void
ao_radio_test(uint8_t on)
{
	if (on) {
		if (!ao_radio_test_on) {
#if HAS_MONITOR
			ao_monitor_disable();
#endif
#if PACKET_HAS_SLAVE
			ao_packet_slave_stop();
#endif
#if HAS_PAD
			ao_pad_disable();
#endif
			ao_radio_get(0xff);
			RFST = RFST_STX;
			ao_radio_test_on = 1;
		}
	} else  {
		if (ao_radio_test_on) {
			ao_radio_idle();
			ao_radio_put();
			ao_radio_test_on = 0;
#if HAS_MONITOR
			ao_monitor_enable();
#endif
#if HAS_PAD
			ao_pad_enable();
#endif
		}
	}
}

static void
ao_radio_test_cmd(void)
{
	uint8_t	mode = 2;
	static __xdata radio_on;
	ao_cmd_white();
	if (ao_cmd_lex_c != '\n') {
		ao_cmd_decimal();
		mode = (uint8_t) ao_cmd_lex_u32;
	}
	mode++;
	if ((mode & 2))
		ao_radio_test(1);
	if (mode == 3) {
		printf ("Hit a character to stop..."); flush();
		getchar();
		putchar('\n');
	}
	if ((mode & 1))
		ao_radio_test(0);
}

__code struct ao_cmds ao_radio_cmds[] = {
	{ ao_radio_test_cmd,	"C <1 start, 0 stop, none both>\0Radio carrier test" },
	{ 0,	NULL },
};

void
ao_radio_init(void)
{
	uint8_t	i;
	for (i = 0; i < sizeof (radio_setup); i += 2)
		RF[radio_setup[i]] = radio_setup[i+1];
	ao_radio_set_packet();
	ao_radio_dma_done = 1;
	ao_radio_dma = ao_dma_alloc(&ao_radio_dma_done);
	RFIF = 0;
	RFIM = RFIM_IM_TIMEOUT|RFIM_IM_DONE;
	IEN2 |= IEN2_RFIE;
	ao_cmd_register(&ao_radio_cmds[0]);
}
