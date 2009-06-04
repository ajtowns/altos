/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <stdint.h>

sfr at 0x80 P0;
sfr at 0x90 P1;
sbit at 0x90 P1_0;
sbit at 0x91 P1_1;
sbit at 0x92 P1_2;
sbit at 0x93 P1_3;
sbit at 0x94 P1_4;
sbit at 0x95 P1_5;
sbit at 0x96 P1_6;
sbit at 0x97 P1_7;

sfr at 0xA0 P2;
sfr at 0xC6 CLKCON;

sfr at 0xF1 PERCFG;
sfr at 0xF2 ADCCFG;
sfr at 0xF3 P0SEL;
sfr at 0xF4 P1SEL;
sfr at 0xF5 P2SEL;

sfr at 0xFD P0DIR;
sfr at 0xFE P1DIR;
sfr at 0xFF P2DIR;
sfr at 0x8F P0INP;
sfr at 0xF6 P1INP;
sfr at 0xF7 P2INP;

sfr at 0x89 P0IFG;
sfr at 0x8A P1IFG;
sfr at 0x8B P2IFG;

sfr at 0xD9 RFD;
sfr at 0xE9 RFIF;
#define RFIF_IM_TXUNF	(1 << 7)
#define RFIF_IM_RXOVF	(1 << 6)
#define RFIF_IM_TIMEOUT	(1 << 5)
#define RFIF_IM_DONE	(1 << 4)
#define RFIF_IM_CS	(1 << 3)
#define RFIF_IM_PQT	(1 << 2)
#define RFIF_IM_CCA	(1 << 1)
#define RFIF_IM_SFD	(1 << 0)

sfr at 0xE1 RFST;

sfr at 0x88 TCON;

sfr at 0xbe SLEEP;

# define SLEEP_USB_EN		(1 << 7)
# define SLEEP_XOSC_STB		(1 << 6)

sbit at 0x89 RFTXRXIF;

#define TCON_RFTXRXIF	(1 << 1)

#define RFST_SFSTXON	0x00
#define RFST_SCAL	0x01
#define RFST_SRX	0x02
#define RFST_STX	0x03
#define RFST_SIDLE	0x04

__xdata __at (0xdf00) uint8_t RF[0x3c];

__xdata __at (0xdf2f) uint8_t RF_IOCFG2;
#define RF_IOCFG2_OFF	0x2f

__xdata __at (0xdf30) uint8_t RF_IOCFG1;
#define RF_IOCFG1_OFF	0x30

__xdata __at (0xdf31) uint8_t RF_IOCFG0;
#define RF_IOCFG0_OFF	0x31

__xdata __at (0xdf00) uint8_t RF_SYNC1;
#define RF_SYNC1_OFF	0x00

__xdata __at (0xdf01) uint8_t RF_SYNC0;
#define RF_SYNC0_OFF	0x01

__xdata __at (0xdf02) uint8_t RF_PKTLEN;
#define RF_PKTLEN_OFF	0x02

__xdata __at (0xdf03) uint8_t RF_PKTCTRL1;
#define RF_PKTCTRL1_OFF	0x03
#define PKTCTRL1_PQT_MASK			(0x7 << 5)
#define PKTCTRL1_PQT_SHIFT			5
#define PKTCTRL1_APPEND_STATUS			(1 << 2)
#define PKTCTRL1_ADR_CHK_NONE			(0 << 0)
#define PKTCTRL1_ADR_CHK_NO_BROADCAST		(1 << 0)
#define PKTCTRL1_ADR_CHK_00_BROADCAST		(2 << 0)
#define PKTCTRL1_ADR_CHK_00_FF_BROADCAST	(3 << 0)

/* If APPEND_STATUS is used, two bytes will be added to the packet data */
#define PKT_APPEND_STATUS_0_RSSI_MASK		(0xff)
#define PKT_APPEND_STATUS_0_RSSI_SHIFT		0
#define PKT_APPEND_STATUS_1_CRC_OK		(1 << 7)
#define PKT_APPEND_STATUS_1_LQI_MASK		(0x7f)
#define PKT_APPEND_STATUS_1_LQI_SHIFT		0

__xdata __at (0xdf04) uint8_t RF_PKTCTRL0;
#define RF_PKTCTRL0_OFF	0x04
#define RF_PKTCTRL0_WHITE_DATA			(1 << 6)
#define RF_PKTCTRL0_PKT_FORMAT_NORMAL		(0 << 4)
#define RF_PKTCTRL0_PKT_FORMAT_RANDOM		(2 << 4)
#define RF_PKTCTRL0_CRC_EN			(1 << 2)
#define RF_PKTCTRL0_LENGTH_CONFIG_FIXED		(0 << 0)
#define RF_PKTCTRL0_LENGTH_CONFIG_VARIABLE	(1 << 0)

__xdata __at (0xdf05) uint8_t RF_ADDR;
#define RF_ADDR_OFF	0x05

__xdata __at (0xdf06) uint8_t RF_CHANNR;
#define RF_CHANNR_OFF	0x06

__xdata __at (0xdf07) uint8_t RF_FSCTRL1;
#define RF_FSCTRL1_OFF	0x07

#define RF_FSCTRL1_FREQ_IF_SHIFT	(0)

__xdata __at (0xdf08) uint8_t RF_FSCTRL0;
#define RF_FSCTRL0_OFF	0x08

#define RF_FSCTRL0_FREQOFF_SHIFT	(0)

__xdata __at (0xdf09) uint8_t RF_FREQ2;
#define RF_FREQ2_OFF	0x09

__xdata __at (0xdf0a) uint8_t RF_FREQ1;
#define RF_FREQ1_OFF	0x0a

__xdata __at (0xdf0b) uint8_t RF_FREQ0;
#define RF_FREQ0_OFF	0x0b

__xdata __at (0xdf0c) uint8_t RF_MDMCFG4;
#define RF_MDMCFG4_OFF	0x0c

#define RF_MDMCFG4_CHANBW_E_SHIFT	6
#define RF_MDMCFG4_CHANBW_M_SHIFT	4
#define RF_MDMCFG4_DRATE_E_SHIFT	0

__xdata __at (0xdf0d) uint8_t RF_MDMCFG3;
#define RF_MDMCFG3_OFF	0x0d

#define RF_MDMCFG3_DRATE_M_SHIFT	0

__xdata __at (0xdf0e) uint8_t RF_MDMCFG2;
#define RF_MDMCFG2_OFF	0x0e

#define RF_MDMCFG2_DEM_DCFILT_OFF	(1 << 7)
#define RF_MDMCFG2_DEM_DCFILT_ON	(0 << 7)

#define RF_MDMCFG2_MOD_FORMAT_MASK	(7 << 4)
#define RF_MDMCFG2_MOD_FORMAT_2_FSK	(0 << 4)
#define RF_MDMCFG2_MOD_FORMAT_GFSK	(1 << 4)
#define RF_MDMCFG2_MOD_FORMAT_ASK_OOK	(3 << 4)
#define RF_MDMCFG2_MOD_FORMAT_MSK	(7 << 4)

#define RF_MDMCFG2_MANCHESTER_EN	(1 << 3)

#define RF_MDMCFG2_SYNC_MODE_MASK		(0x7 << 0)
#define RF_MDMCFG2_SYNC_MODE_NONE		(0x0 << 0)
#define RF_MDMCFG2_SYNC_MODE_15_16		(0x1 << 0)
#define RF_MDMCFG2_SYNC_MODE_16_16		(0x2 << 0)
#define RF_MDMCFG2_SYNC_MODE_30_32		(0x3 << 0)
#define RF_MDMCFG2_SYNC_MODE_NONE_THRES		(0x4 << 0)
#define RF_MDMCFG2_SYNC_MODE_15_16_THRES	(0x5 << 0)
#define RF_MDMCFG2_SYNC_MODE_16_16_THRES	(0x6 << 0)
#define RF_MDMCFG2_SYNC_MODE_30_32_THRES	(0x7 << 0)

__xdata __at (0xdf0f) uint8_t RF_MDMCFG1;
#define RF_MDMCFG1_OFF	0x0f

#define RF_MDMCFG1_FEC_EN			(1 << 7)
#define RF_MDMCFG1_FEC_DIS			(0 << 7)

#define RF_MDMCFG1_NUM_PREAMBLE_MASK		(7 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_2		(0 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_3		(1 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_4		(2 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_6		(3 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_8		(4 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_12		(5 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_16		(6 << 4)
#define RF_MDMCFG1_NUM_PREAMBLE_24		(7 << 4)

#define RF_MDMCFG1_CHANSPC_E_MASK		(3 << 0)
#define RF_MDMCFG1_CHANSPC_E_SHIFT		(0)

__xdata __at (0xdf10) uint8_t RF_MDMCFG0;
#define RF_MDMCFG0_OFF	0x10

#define RF_MDMCFG0_CHANSPC_M_SHIFT		(0)

__xdata __at (0xdf11) uint8_t RF_DEVIATN;
#define RF_DEVIATN_OFF	0x11

#define RF_DEVIATN_DEVIATION_E_SHIFT		4
#define RF_DEVIATN_DEVIATION_M_SHIFT		0

__xdata __at (0xdf12) uint8_t RF_MCSM2;
#define RF_MCSM2_OFF	0x12
#define RF_MCSM2_RX_TIME_RSSI			(1 << 4)
#define RF_MCSM2_RX_TIME_QUAL			(1 << 3)
#define RF_MCSM2_RX_TIME_MASK			(0x7)
#define RF_MCSM2_RX_TIME_SHIFT			0
#define RF_MCSM2_RX_TIME_END_OF_PACKET		(7)

__xdata __at (0xdf13) uint8_t RF_MCSM1;
#define RF_MCSM1_OFF	0x13
#define RF_MCSM1_CCA_MODE_ALWAYS			(0 << 4)
#define RF_MCSM1_CCA_MODE_RSSI_BELOW			(1 << 4)
#define RF_MCSM1_CCA_MODE_UNLESS_RECEIVING		(2 << 4)
#define RF_MCSM1_CCA_MODE_RSSI_BELOW_UNLESS_RECEIVING	(3 << 4)
#define RF_MCSM1_RXOFF_MODE_IDLE			(0 << 2)
#define RF_MCSM1_RXOFF_MODE_FSTXON			(1 << 2)
#define RF_MCSM1_RXOFF_MODE_TX				(2 << 2)
#define RF_MCSM1_RXOFF_MODE_RX				(3 << 2)
#define RF_MCSM1_TXOFF_MODE_IDLE			(0 << 0)
#define RF_MCSM1_TXOFF_MODE_FSTXON			(1 << 0)
#define RF_MCSM1_TXOFF_MODE_TX				(2 << 0)
#define RF_MCSM1_TXOFF_MODE_RX				(3 << 0)

__xdata __at (0xdf14) uint8_t RF_MCSM0;
#define RF_MCSM0_OFF	0x14
#define RF_MCSM0_FS_AUTOCAL_NEVER		(0 << 4)
#define RF_MCSM0_FS_AUTOCAL_FROM_IDLE		(1 << 4)
#define RF_MCSM0_FS_AUTOCAL_TO_IDLE		(2 << 4)
#define RF_MCSM0_FS_AUTOCAL_TO_IDLE_EVERY_4	(3 << 4)
#define RF_MCSM0_MAGIC_3			(1 << 3)
#define RF_MCSM0_MAGIC_2			(1 << 2)
#define RF_MCSM0_CLOSE_IN_RX_0DB		(0 << 0)
#define RF_MCSM0_CLOSE_IN_RX_6DB		(1 << 0)
#define RF_MCSM0_CLOSE_IN_RX_12DB		(2 << 0)
#define RF_MCSM0_CLOSE_IN_RX_18DB		(3 << 0)

__xdata __at (0xdf15) uint8_t RF_FOCCFG;
#define RF_FOCCFG_OFF	0x15
#define RF_FOCCFG_FOC_BS_CS_GATE		(1 << 5)
#define RF_FOCCFG_FOC_PRE_K_1K			(0 << 3)
#define RF_FOCCFG_FOC_PRE_K_2K			(1 << 3)
#define RF_FOCCFG_FOC_PRE_K_3K			(2 << 3)
#define RF_FOCCFG_FOC_PRE_K_4K			(3 << 3)
#define RF_FOCCFG_FOC_POST_K_PRE_K		(0 << 2)
#define RF_FOCCFG_FOC_POST_K_PRE_K_OVER_2	(1 << 2)
#define RF_FOCCFG_FOC_LIMIT_0			(0 << 0)
#define RF_FOCCFG_FOC_LIMIT_BW_OVER_8		(1 << 0)
#define RF_FOCCFG_FOC_LIMIT_BW_OVER_4		(2 << 0)
#define RF_FOCCFG_FOC_LIMIT_BW_OVER_2		(3 << 0)

__xdata __at (0xdf16) uint8_t RF_BSCFG;
#define RF_BSCFG_OFF	0x16
#define RF_BSCFG_BS_PRE_K_1K			(0 << 6)
#define RF_BSCFG_BS_PRE_K_2K			(1 << 6)
#define RF_BSCFG_BS_PRE_K_3K			(2 << 6)
#define RF_BSCFG_BS_PRE_K_4K			(3 << 6)
#define RF_BSCFG_BS_PRE_KP_1KP			(0 << 4)
#define RF_BSCFG_BS_PRE_KP_2KP			(1 << 4)
#define RF_BSCFG_BS_PRE_KP_3KP			(2 << 4)
#define RF_BSCFG_BS_PRE_KP_4KP			(3 << 4)
#define RF_BSCFG_BS_POST_KI_PRE_KI		(0 << 3)
#define RF_BSCFG_BS_POST_KI_PRE_KI_OVER_2	(1 << 3)
#define RF_BSCFG_BS_POST_KP_PRE_KP		(0 << 2)
#define RF_BSCFG_BS_POST_KP_PRE_KP_OVER_2	(1 << 2)
#define RF_BSCFG_BS_LIMIT_0			(0 << 0)
#define RF_BSCFG_BS_LIMIT_3_125			(1 << 0)
#define RF_BSCFG_BS_LIMIT_6_25			(2 << 0)
#define RF_BSCFG_BS_LIMIT_12_5			(3 << 0)

__xdata __at (0xdf17) uint8_t RF_AGCCTRL2;
#define RF_AGCCTRL2_OFF	0x17

__xdata __at (0xdf18) uint8_t RF_AGCCTRL1;
#define RF_AGCCTRL1_OFF	0x18

__xdata __at (0xdf19) uint8_t RF_AGCCTRL0;
#define RF_AGCCTRL0_OFF	0x19

__xdata __at (0xdf1a) uint8_t RF_FREND1;
#define RF_FREND1_OFF	0x1a

#define RF_FREND1_LNA_CURRENT_SHIFT		6
#define RF_FREND1_LNA2MIX_CURRENT_SHIFT		4
#define RF_FREND1_LODIV_BUF_CURRENT_RX_SHIFT	2
#define RF_FREND1_MIX_CURRENT_SHIFT		0

__xdata __at (0xdf1b) uint8_t RF_FREND0;
#define RF_FREND0_OFF	0x1b

#define RF_FREND0_LODIV_BUF_CURRENT_TX_MASK	(0x3 << 4)
#define RF_FREND0_LODIV_BUF_CURRENT_TX_SHIFT	4
#define RF_FREND0_PA_POWER_MASK			(0x7)
#define RF_FREND0_PA_POWER_SHIFT		0

__xdata __at (0xdf1c) uint8_t RF_FSCAL3;
#define RF_FSCAL3_OFF	0x1c

__xdata __at (0xdf1d) uint8_t RF_FSCAL2;
#define RF_FSCAL2_OFF	0x1d

__xdata __at (0xdf1e) uint8_t RF_FSCAL1;
#define RF_FSCAL1_OFF	0x1e

__xdata __at (0xdf1f) uint8_t RF_FSCAL0;
#define RF_FSCAL0_OFF	0x1f

__xdata __at (0xdf23) uint8_t RF_TEST2;
#define RF_TEST2_OFF	0x23

#define RF_TEST2_NORMAL_MAGIC		0x88
#define RF_TEST2_RX_LOW_DATA_RATE_MAGIC	0x81

__xdata __at (0xdf24) uint8_t RF_TEST1;
#define RF_TEST1_OFF	0x24

#define RF_TEST1_TX_MAGIC		0x31
#define RF_TEST1_RX_LOW_DATA_RATE_MAGIC	0x35

__xdata __at (0xdf25) uint8_t RF_TEST0;
#define RF_TEST0_OFF	0x25

#define RF_TEST0_7_2_MASK		(0xfc)
#define RF_TEST0_VCO_SEL_CAL_EN		(1 << 1)
#define RF_TEST0_0_MASK			(1)

/* These are undocumented, and must be computed
 * using the provided tool.
 */
__xdata __at (0xdf27) uint8_t RF_PA_TABLE7;
#define RF_PA_TABLE7_OFF	0x27

__xdata __at (0xdf28) uint8_t RF_PA_TABLE6;
#define RF_PA_TABLE6_OFF	0x28

__xdata __at (0xdf29) uint8_t RF_PA_TABLE5;
#define RF_PA_TABLE5_OFF	0x29

__xdata __at (0xdf2a) uint8_t RF_PA_TABLE4;
#define RF_PA_TABLE4_OFF	0x2a

__xdata __at (0xdf2b) uint8_t RF_PA_TABLE3;
#define RF_PA_TABLE3_OFF	0x2b

__xdata __at (0xdf2c) uint8_t RF_PA_TABLE2;
#define RF_PA_TABLE2_OFF	0x2c

__xdata __at (0xdf2d) uint8_t RF_PA_TABLE1;
#define RF_PA_TABLE1_OFF	0x2d

__xdata __at (0xdf2e) uint8_t RF_PA_TABLE0;
#define RF_PA_TABLE0_OFF	0x2e

__xdata __at (0xdf36) uint8_t RF_PARTNUM;
#define RF_PARTNUM_OFF	0x36

__xdata __at (0xdf37) uint8_t RF_VERSION;
#define RF_VERSION_OFF	0x37

__xdata __at (0xdf38) uint8_t RF_FREQEST;
#define RF_FREQEST_OFF	0x38

__xdata __at (0xdf39) uint8_t RF_LQI;
#define RF_LQI_OFF	0x39

#define RF_LQI_CRC_OK			(1 << 7)
#define RF_LQI_LQI_EST_MASK		(0x7f)

__xdata __at (0xdf3a) uint8_t RF_RSSI;
#define RF_RSSI_OFF	0x3a

__xdata __at (0xdf3b) uint8_t RF_MARCSTATE;
#define RF_MARCSTATE_OFF	0x3b

#define RF_MARCSTATE_MASK		0x0f
#define RF_MARCSTATE_SLEEP		0x00
#define RF_MARCSTATE_IDLE		0x01
#define RF_MARCSTATE_VCOON_MC		0x03
#define RF_MARCSTATE_REGON_MC		0x04
#define RF_MARCSTATE_MANCAL		0x05
#define RF_MARCSTATE_VCOON		0x06
#define RF_MARCSTATE_REGON		0x07
#define RF_MARCSTATE_STARTCAL		0x08
#define RF_MARCSTATE_BWBOOST		0x09
#define RF_MARCSTATE_FS_LOCK		0x0a
#define RF_MARCSTATE_IFADCON		0x0b
#define RF_MARCSTATE_ENDCAL		0x0c
#define RF_MARCSTATE_RX			0x0d
#define RF_MARCSTATE_RX_END		0x0e
#define RF_MARCSTATE_RX_RST		0x0f
#define RF_MARCSTATE_TXRX_SWITCH	0x10
#define RF_MARCSTATE_RX_OVERFLOW	0x11
#define RF_MARCSTATE_FSTXON		0x12
#define RF_MARCSTATE_TX			0x13
#define RF_MARCSTATE_TX_END		0x14
#define RF_MARCSTATE_RXTX_SWITCH	0x15
#define RF_MARCSTATE_TX_UNDERFLOW	0x16


__xdata __at (0xdf3c) uint8_t RF_PKTSTATUS;
#define RF_PKTSTATUS_OFF	0x3c

#define RF_PKTSTATUS_CRC_OK		(1 << 7)
#define RF_PKTSTATUS_CS			(1 << 6)
#define RF_PKTSTATUS_PQT_REACHED	(1 << 5)
#define RF_PKTSTATUS_CCA		(1 << 4)
#define RF_PKTSTATUS_SFD		(1 << 3)

__xdata __at (0xdf3d) uint8_t RF_VCO_VC_DAC;
#define RF_VCO_VC_DAC_OFF	0x3d

#define PACKET_LEN	128

void
radio_init(void);

void
delay (unsigned char n);
