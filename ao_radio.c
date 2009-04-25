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
 * For 434.550MHz, the frequency value is:
 *
 * 434.550e6 / (24e6 / 2**16) = 1186611.2
 */

#define FREQ_CONTROL	1186611

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

	RF_FREQ2_OFF,		(FREQ_CONTROL >> 16) & 0xff,
	RF_FREQ1_OFF,		(FREQ_CONTROL >> 8) & 0xff,
	RF_FREQ0_OFF,		(FREQ_CONTROL >> 0) & 0xff,

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
	RF_PKTLEN_OFF,		sizeof (struct ao_telemetry),

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
	RF_AGCCTRL2_OFF,	0x43,
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
				 RF_MDMCFG2_SYNC_MODE_15_16_THRES),
	RF_MDMCFG1_OFF,		(RF_MDMCFG1_FEC_DIS |
				 RF_MDMCFG1_NUM_PREAMBLE_2 |
				 (2 << RF_MDMCFG1_CHANSPC_E_SHIFT)),

	RF_DEVIATN_OFF,		((RDF_DEVIATION_E << RF_DEVIATN_DEVIATION_E_SHIFT) |
				 (RDF_DEVIATION_M << RF_DEVIATN_DEVIATION_M_SHIFT)),

	/* packet length */
	RF_PKTLEN_OFF,		RDF_PACKET_LEN,
	RF_PKTCTRL1_OFF,	((1 << PKTCTRL1_PQT_SHIFT)|
				 PKTCTRL1_ADR_CHK_NONE),
	RF_PKTCTRL0_OFF,	(RF_PKTCTRL0_PKT_FORMAT_NORMAL|
				 RF_PKTCTRL0_LENGTH_CONFIG_FIXED),
};

static __code uint8_t telemetry_setup[] = {
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

	/* max packet length */
	RF_PKTLEN_OFF,		sizeof (struct ao_telemetry),
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
__xdata uint8_t ao_radio_mutex;

static void
ao_radio_idle(void)
{
	if (RF_MARCSTATE != RF_MARCSTATE_IDLE)
	{
		RFST = RFST_SIDLE;
		do {
			ao_yield();
		} while (RF_MARCSTATE != RF_MARCSTATE_IDLE);
	}
}

void
ao_radio_send(__xdata struct ao_telemetry *telemetry) __reentrant
{
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	ao_dma_set_transfer(ao_radio_dma,
			    telemetry,
			    &RFDXADDR,
			    sizeof (struct ao_telemetry),
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_STX;
	__critical while (!ao_radio_dma_done)
		ao_sleep(&ao_radio_dma_done);
	ao_mutex_put(&ao_radio_mutex);
}

void
ao_radio_recv(__xdata struct ao_radio_recv *radio) __reentrant
{
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	ao_dma_set_transfer(ao_radio_dma,
			    &RFDXADDR,
			    radio,
			    sizeof (struct ao_radio_recv),
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_SRX;
	__critical while (!ao_radio_dma_done)
		ao_sleep(&ao_radio_dma_done);
	ao_mutex_put(&ao_radio_mutex);
}

__xdata ao_radio_rdf_running;
__xdata ao_radio_rdf_value = 0x55;

void
ao_radio_rdf(void)
{
	uint8_t i;
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	ao_radio_rdf_running = 1;
	for (i = 0; i < sizeof (rdf_setup); i += 2)
		RF[rdf_setup[i]] = rdf_setup[i+1];

	ao_dma_set_transfer(ao_radio_dma,
			    &ao_radio_rdf_value,
			    &RFDXADDR,
			    RDF_PACKET_LEN,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_STX;

	__critical while (!ao_radio_dma_done)
		ao_sleep(&ao_radio_dma_done);
	ao_radio_rdf_running = 0;
	ao_radio_idle();
	for (i = 0; i < sizeof (rdf_setup); i += 2)
		RF[telemetry_setup[i]] = telemetry_setup[i+1];
	ao_mutex_put(&ao_radio_mutex);
}

void
ao_radio_rdf_abort(void)
{
	if (ao_radio_rdf_running) {
		ao_dma_abort(ao_radio_dma);
		ao_radio_idle();
	}
}

void
ao_radio_init(void)
{
	uint8_t	i;
	for (i = 0; i < sizeof (radio_setup); i += 2)
		RF[radio_setup[i]] = radio_setup[i+1];
	ao_radio_dma_done = 1;
	ao_radio_dma = ao_dma_alloc(&ao_radio_dma_done);
}

