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

#ifndef _AO_CC115L_H_
#define _AO_CC115L_H_

#define CC115L_BURST		6
#define CC115L_READ		7

/* Register space */
#define CC115L_IOCFG2 		0x00	/* GDO2 Output Pin Configuration */
#define CC115L_IOCFG1 		0x01	/* GDO1 Output Pin Configuration */
#define CC115L_IOCFG0 		0x02	/* GDO0 Output Pin Configuration */

#define  CC115L_IOCFG_GPIO1_DS		7
#define  CC115L_IOCFG_GPIO_INV		6

#define  CC115L_IOCFG_GPIO_CFG		0
#define  CC115L_IOCFG_GPIO_CFG_TXFIFO_THR	2
#define  CC115L_IOCFG_GPIO_CFG_TXFIFO_THR_PKT	3
#define  CC115L_IOCFG_GPIO_CFG_TXFIFO_UNDERFLOW	5
#define  CC115L_IOCFG_GPIO_CFG_PKT_SYNC_TX	6
#define  CC115L_IOCFG_GPIO_CFG_PLL_LOCKED	10
#define  CC115L_IOCFG_GPIO_CFG_SERIAL_CLK	11
#define  CC115L_IOCFG_GPIO_CFG_SYNC_DATA	12
#define  CC115L_IOCFG_GPIO_CFG_ASYNC_DATA	13
#define  CC115L_IOCFG_GPIO_CFG_PA_PD		27
#define  CC115L_IOCFG_GPIO_CFG_CHIP_RDYn	41
#define  CC115L_IOCFG_GPIO_CFG_XOSC_STABLE	43
#define  CC115L_IOCFG_GPIO_CFG_HIGHZ		46
#define  CC115L_IOCFG_GPIO_CFG_HW_0		47
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_1	48
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_1_5	49
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_2	50
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_3	51
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_4	52
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_6	53
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_8	54
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_12	55
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_16	56
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_24	57
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_32	58
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_48	59
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_64	60
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_96	61
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_128	62
#define  CC115L_IOCFG_GPIO_CFG_CLK_XOSC_192	63
#define  CC115L_IOCFG_GPIO_CFG_MASK	0x3f

#define CC115L_FIFOTHR 		0x03	/* TX FIFO Thresholds */
#define  CC115L_FIFOTHR_THR_MASK	0x0f
#define  CC115L_FIFOTHR_THR_61		0
#define  CC115L_FIFOTHR_THR_57		1
#define  CC115L_FIFOTHR_THR_53		2
#define  CC115L_FIFOTHR_THR_49		3
#define  CC115L_FIFOTHR_THR_45		4
#define  CC115L_FIFOTHR_THR_41		5
#define  CC115L_FIFOTHR_THR_37		6
#define  CC115L_FIFOTHR_THR_33		7
#define  CC115L_FIFOTHR_THR_29		8
#define  CC115L_FIFOTHR_THR_25		9
#define  CC115L_FIFOTHR_THR_21		10
#define  CC115L_FIFOTHR_THR_17		11
#define  CC115L_FIFOTHR_THR_13		12
#define  CC115L_FIFOTHR_THR_9		13
#define  CC115L_FIFOTHR_THR_5		14
#define  CC115L_FIFOTHR_THR_1		15

#define CC115L_SYNC1		0x04	/* Sync Word, High Byte */
#define CC115L_SYNC0		0x05	/* Sync Word, Low Byte */
#define CC115L_PKTLEN		0x06	/* Packet Length */
#define CC115L_PKTCTRL0		0x08	/* Packet Automation Control */
#define  CC115L_PKTCTRL0_PKT_FORMAT		4
#define  CC115L_PKTCTRL0_PKT_FORMAT_NORMAL		0
#define  CC115L_PKTCTRL0_PKT_FORMAT_SYNC_SERIAL		1
#define  CC115L_PKTCTRL0_PKT_FORMAT_RANDOM		2
#define  CC115L_PKTCTRL0_PKT_FORMAT_ASYNC_SERIAL	3
#define  CC115L_PKTCTRL0_PKT_FORMAT_MASK		3
#define  CC115L_PKTCTRL0_PKT_CRC_EN		2
#define  CC115L_PKTCTRL0_PKT_LENGTH_CONFIG	0
#define  CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_FIXED	0
#define  CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_VARIABLE	1
#define  CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_INFINITE	2
#define  CC115L_PKTCTRL0_PKT_LENGTH_CONFIG_MASK		3
#define CC115L_CHANNR		0x0a	/* Channel Number */
#define CC115L_FSCTRL0		0x0c	/* Frequency Synthesizer Control */
#define CC115L_FREQ2 		0x0d	/* Frequency Control Word, High Byte */
#define CC115L_FREQ1 		0x0e	/* Frequency Control Word, Middle Byte */
#define CC115L_FREQ0 		0x0f	/* Frequency Control Word, Low Byte */
#define CC115L_MDMCFG4 		0x10	/* Modem Configuration */
#define  CC115L_MDMCFG4_DRATE_E			0
#define CC115L_MDMCFG3		0x11	/* Modem Configuration */
#define CC115L_MDMCFG2		0x12	/* Modem Configuration */
#define  CC115L_MDMCFG2_MOD_FORMAT		4
#define  CC115L_MDMCFG2_MOD_FORMAT_2FSK			0
#define  CC115L_MDMCFG2_MOD_FORMAT_GFSK			1
#define  CC115L_MDMCFG2_MOD_FORMAT_OOK			3
#define  CC115L_MDMCFG2_MOD_FORMAT_4FSK			4
#define  CC115L_MDMCFG2_MOD_FORMAT_MASK			7
#define  CC115L_MDMCFG2_MANCHESTER_EN		3
#define  CC115L_MDMCFG2_SYNC_MODE		0
#define  CC115L_MDMCFG2_SYNC_MODE_NONE			0
#define  CC115L_MDMCFG2_SYNC_MODE_16BITS		1
#define  CC115L_MDMCFG2_SYNC_MODE_32BITS		3
#define  CC115L_MDMCFG2_SYNC_MODE_MASK			3
#define CC115L_MDMCFG1		0x13	/* Modem Configuration */
#define  CC115L_MDMCFG1_NUM_PREAMBLE		4
#define  CC115L_MDMCFG1_NUM_PREAMBLE_2			0
#define  CC115L_MDMCFG1_NUM_PREAMBLE_3			1
#define  CC115L_MDMCFG1_NUM_PREAMBLE_4			2
#define  CC115L_MDMCFG1_NUM_PREAMBLE_6			3
#define  CC115L_MDMCFG1_NUM_PREAMBLE_8			4
#define  CC115L_MDMCFG1_NUM_PREAMBLE_12			5
#define  CC115L_MDMCFG1_NUM_PREAMBLE_16			6
#define  CC115L_MDMCFG1_NUM_PREAMBLE_24			7
#define  CC115L_MDMCFG1_NUM_PREAMBLE_MASK		7
#define  CC115L_MDMCFG1_CHANSPC_E		0
#define CC115L_MDMCFG0		0x14	/* Modem Configuration */
#define CC115L_DEVIATN		0x15	/* Modem Deviation Setting */
#define  CC115L_DEVIATN_DEVIATION_E		4
#define  CC115L_DEVIATN_DEVIATION_E_MASK	7
#define  CC115L_DEVIATN_DEVIATION_M		0
#define  CC115L_DEVIATN_DEVIATION_M_MASK	7
#define CC115L_MCSM1		0x17	/* Main Radio Control State Machine Configuration */
#define  CC115L_MCSM1_TXOFF_MODE		0
#define  CC115L_MCSM1_TXOFF_MODE_IDLE			0
#define  CC115L_MCSM1_TXOFF_MODE_FSTXON			1
#define  CC115L_MCSM1_TXOFF_MODE_TX			2
#define  CC115L_MCSM1_TXOFF_MODE_MASK			3
#define CC115L_MCSM0		0x18	/* Main Radio Control State Machine Configuration */
#define  CC115L_MCSM0_FS_AUTOCAL		4
#define  CC115L_MCSM0_FS_AUTOCAL_NEVER			0
#define  CC115L_MCSM0_FS_AUTOCAL_IDLE_TO_TX		1
#define  CC115L_MCSM0_FS_AUTOCAL_TX_TO_IDLE		2
#define  CC115L_MCSM0_FS_AUTOCAL_4TH_TX_TO_IDLE		3
#define  CC115L_MCSM0_FS_AUTOCAL_MASK			3
#define  CC115L_MCSM0_PO_TIMEOUT		2
#define  CC115L_MCSM0_PO_TIMEOUT_1			0
#define  CC115L_MCSM0_PO_TIMEOUT_16			1
#define  CC115L_MCSM0_PO_TIMEOUT_64			2
#define  CC115L_MCSM0_PO_TIMEOUT_256			3
#define  CC115L_MCSM0_PO_TIMEOUT_MASK			3
#define  CC115L_MCSM0_XOSC_FORCE_ON		0
#define CC115L_RESERVED_0X20	0x20	/* Use setting from SmartRF Studio */
#define CC115L_FREND0		0x22	/* Front End TX Configuration */
#define CC115L_FSCAL3		0x23	/* Frequency Synthesizer Calibration */
#define CC115L_FSCAL2		0x24	/* Frequency Synthesizer Calibration */
#define CC115L_FSCAL1		0x25	/* Frequency Synthesizer Calibration */
#define CC115L_FSCAL0		0x26	/* Frequency Synthesizer Calibration */
#define CC115L_RESERVED_0X29	0x29    /* Use setting from SmartRF Studio */
#define CC115L_RESERVED_0X2A    0x2a   	/* Use setting from SmartRF Studio */
#define CC115L_RESERVED_0X2B    0x2b    /* Use setting from SmartRF Studio */
#define CC115L_TEST2		0x2c	/* Various Test Settings */
#define CC115L_TEST1		0x2d	/* Various Test Settings */
#define CC115L_TEST0		0x2e	/* Various Test Settings */

/* Status registers (use BURST bit to select these) */
#define CC115L_PARTNUM		(0x30|(1<<CC115L_BURST))	/* Part number for CC115L */
#define CC115L_VERSION		(0x31|(1<<CC115L_BURST))	/* Current version number */
#define CC115L_MARCSTATE	(0x35|(1<<CC115L_BURST))	/* Control state machine state */
#define  CC115L_MARCSTATE_MASK		0x1f
#define  CC115L_MARCSTATE_SLEEP		0x00
#define  CC115L_MARCSTATE_IDLE		0x01
#define  CC115L_MARCSTATE_XOFF		0x02
#define  CC115L_MARCSTATE_VCOON_MC	0x03
#define  CC115L_MARCSTATE_REGON_MC	0x04
#define  CC115L_MARCSTATE_MANCAL	0x05
#define  CC115L_MARCSTATE_VCOON		0x06
#define  CC115L_MARCSTATE_REGON		0x07
#define  CC115L_MARCSTATE_STARTCAL	0x08
#define  CC115L_MARCSTATE_BWBOOST	0x09
#define  CC115L_MARCSTATE_FS_LOCK	0x0a
#define  CC115L_MARCSTATE_ENDCAL	0x0c
#define  CC115L_MARCSTATE_FSTXON	0x12
#define  CC115L_MARCSTATE_TX		0x13
#define  CC115L_MARCSTATE_TX_END	0x14
#define  CC115L_MARCSTATE_TX_UNDERFLOW	0x16
#define CC115L_PKTSTATUS	(0x38|(1<<CC115L_BURST))	/* Current GDOx status and packet status */
#define CC115L_TXBYTES		(0x3a|(1<<CC115L_BURST))	/* Underflow and number of bytes in the TX FIFO */
#define  CC115L_TXBYTES_TXFIFO_UNDERFLOW	7
#define  CC115L_TXBYTES_NUM_TX_BYTES		0
#define  CC115L_TXBYTES_NUM_TX_BYTES_MASK		0x7f

/* Command strobes (no BURST bit for these) */
#define CC115L_SRES		0x30
#define CC115L_SFSTXON		0x31
#define CC115L_SXOFF		0x32
#define CC115L_SCAL		0x33
#define CC115L_STX		0x35
#define CC115L_SIDLE		0x36
#define CC115L_SPWD		0x39
#define CC115L_SFTX		0x3b
#define CC115L_SNOP		0x3d

#define CC115L_PA		0x3e
#define CC115L_FIFO		0x3f

#define CC115L_FIFO_SIZE	64

/* Status byte */
#define CC115L_STATUS_CHIP_RDY	7
#define CC115L_STATUS_STATE	4
#define  CC115L_STATUS_STATE_IDLE		0
#define  CC115L_STATUS_STATE_TX			2
#define  CC115L_STATUS_STATE_FSTXON		3
#define  CC115L_STATUS_STATE_CALIBRATE		4
#define  CC115L_STATUS_STATE_SETTLING		5
#define  CC115L_STATUS_STATE_TX_FIFO_UNDERFLOW	7
#define  CC115L_STATUS_STATE_MASK		7

#define CC115L_STATUS_FIFO_BYTES_AVAILABLE	0
#endif /* _AO_CC115L_H_ */
