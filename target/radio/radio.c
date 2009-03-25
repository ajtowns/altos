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

__xdata __at (0xdf04) uint8_t RF_PKTCTRL0;
#define RF_PKTCTRL0_OFF	0x04

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

__xdata __at (0xdf13) uint8_t RF_MCSM1;
#define RF_MCSM1_OFF	0x13

__xdata __at (0xdf14) uint8_t RF_MCSM0;
#define RF_MCSM0_OFF	0x14

__xdata __at (0xdf15) uint8_t RF_FOCCFG;
#define RF_FOCCFG_OFF	0x15

__xdata __at (0xdf16) uint8_t RF_BSCFG;
#define RF_BSCFG_OFF	0x16

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

#define nop()	_asm nop _endasm;

void
delay (unsigned char n)
{
	unsigned char i = 0;

	n <<= 1;
	while (--n != 0)
		while (--i != 0)
			nop();
}

void
tone (unsigned char n, unsigned char m)
{
	unsigned char	i = 0;
	while (--m != 0) {
		i = 128;
		while (--i != 0) {
			P2 = 0xff;
			delay(n);
			P2 = 0xfe;
			delay(n);
		}
	}
}

void
high() {
	tone(1, 2);
}

void
low() {
	tone(2, 1);
}

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

#define PACKET_LEN	128

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

#define RF_POWER		RF_POWER_0_DBM

static __code uint8_t radio_setup[] = {
	RF_PA_TABLE7_OFF,	RF_POWER,
	RF_PA_TABLE6_OFF,	RF_POWER,
	RF_PA_TABLE5_OFF,	RF_POWER,
	RF_PA_TABLE4_OFF,	RF_POWER,
	RF_PA_TABLE3_OFF,	RF_POWER,
	RF_PA_TABLE2_OFF,	RF_POWER,
	RF_PA_TABLE1_OFF,	RF_POWER,
	RF_PA_TABLE0_OFF,	RF_POWER,

	RF_FREQ2_OFF,		FREQ_CONTROL >> 16,
	RF_FREQ1_OFF,		FREQ_CONTROL >> 8,
	RF_FREQ0_OFF,		FREQ_CONTROL >> 0,
	
	RF_FSCTRL1_OFF,		(IF_FREQ_CONTROL << RF_FSCTRL1_FREQ_IF_SHIFT),
	RF_FSCTRL0_OFF,		(0 << RF_FSCTRL0_FREQOFF_SHIFT),

	RF_MDMCFG4_OFF,		((CHANBW_E << RF_MDMCFG4_CHANBW_E_SHIFT) |
				 (CHANBW_M << RF_MDMCFG4_CHANBW_M_SHIFT) |
				 (DRATE_E << RF_MDMCFG4_DRATE_E_SHIFT)),
	RF_MDMCFG3_OFF,		(DRATE_M << RF_MDMCFG3_DRATE_M_SHIFT),
	RF_MDMCFG2_OFF,		(RF_MDMCFG2_DEM_DCFILT_OFF |
				 RF_MDMCFG2_MOD_FORMAT_GFSK |
				 RF_MDMCFG2_SYNC_MODE_NONE),
	RF_MDMCFG1_OFF,		(RF_MDMCFG1_NUM_PREAMBLE_4 |
				 (2 << RF_MDMCFG1_CHANSPC_E_SHIFT)),
	RF_MDMCFG0_OFF,		(17 << RF_MDMCFG0_CHANSPC_M_SHIFT),

	RF_CHANNR_OFF,		0,

	RF_DEVIATN_OFF,		((3 << RF_DEVIATN_DEVIATION_E_SHIFT) |
				 (6 << RF_DEVIATN_DEVIATION_M_SHIFT)),

	/* SmartRF says set LODIV_BUF_CURRENT_TX to 0
	 * And, we're not using power ramping, so use PA_POWER 0
	 */
	RF_FREND0_OFF,		((1 << RF_FREND0_LODIV_BUF_CURRENT_TX_SHIFT) |
				 (0 << RF_FREND0_PA_POWER_SHIFT)),

	RF_FREND1_OFF,		((1 << RF_FREND1_LNA_CURRENT_SHIFT) |
				 (1 << RF_FREND1_LNA2MIX_CURRENT_SHIFT) |
				 (1 << RF_FREND1_LODIV_BUF_CURRENT_RX_SHIFT) |
				 (2 << RF_FREND1_MIX_CURRENT_SHIFT)),
	RF_MCSM0_OFF,		0x18,
	RF_FOCCFG_OFF,		0x16,
	RF_BSCFG_OFF,		0x6C,

	RF_AGCCTRL2_OFF,	0x43,
	RF_AGCCTRL1_OFF,	0x40,
	RF_AGCCTRL0_OFF,	0x91,

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
	RF_PKTLEN_OFF,		PACKET_LEN,

	RF_PKTCTRL1_OFF,	0x04,
	RF_PKTCTRL0_OFF,	0x00,
	RF_ADDR_OFF,		0x00,
	RF_MCSM2_OFF,		0x07,
	RF_MCSM1_OFF,		0x30,
	
	RF_IOCFG2_OFF,		0x00,
	RF_IOCFG1_OFF,		0x00,
	RF_IOCFG0_OFF,		0x00,
};

void
radio_init() {
	uint8_t	i;
	for (i = 0; i < sizeof (radio_setup); i += 2)
		RF[radio_setup[i]] = radio_setup[i+1];
}

main ()
{
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;
	/* Set P2_0 to output */
	radio_init ();
	delay(100);
	RFST = RFST_SIDLE;
	delay(100);
	RFST = RFST_STX;
	delay(100);
	for (;;);
#if 0
	for (;;) {
		uint8_t	i;
		for (i = 0; i < PACKET_LEN; i++) {
			while (!RFTXRXIF);
			RFTXRXIF = 0;
			RFD = 0x55;
		}
	}
#endif
}
