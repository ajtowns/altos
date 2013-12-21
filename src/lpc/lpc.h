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

#ifndef _LPC_H_
#define _LPC_H_

#include <stdint.h>

typedef volatile uint32_t	vuint32_t;
typedef volatile uint16_t	vuint16_t;
typedef volatile uint8_t	vuint8_t;
typedef volatile void *		vvoid_t;

struct lpc_ioconf {
	vuint32_t	pio0_0;
	vuint32_t	pio0_1;
	vuint32_t	pio0_2;
	vuint32_t	pio0_3;

	vuint32_t	pio0_4;
	vuint32_t	pio0_5;
	vuint32_t	pio0_6;
	vuint32_t	pio0_7;

	vuint32_t	pio0_8;
	vuint32_t	pio0_9;
	vuint32_t	pio0_10;
	vuint32_t	pio0_11;

	vuint32_t	pio0_12;
	vuint32_t	pio0_13;
	vuint32_t	pio0_14;
	vuint32_t	pio0_15;

	vuint32_t	pio0_16;
	vuint32_t	pio0_17;
	vuint32_t	pio0_18;
	vuint32_t	pio0_19;

	vuint32_t	pio0_20;
	vuint32_t	pio0_21;
	vuint32_t	pio0_22;
	vuint32_t	pio0_23;

	vuint32_t	pio1_0;		/* 0x60 */
	vuint32_t	pio1_1;
	vuint32_t	pio1_2;
	vuint32_t	pio1_3;

	vuint32_t	pio1_4;
	vuint32_t	pio1_5;
	vuint32_t	pio1_6;
	vuint32_t	pio1_7;

	vuint32_t	pio1_8;		/* 0x80 */
	vuint32_t	pio1_9;
	vuint32_t	pio1_10;
	vuint32_t	pio1_11;

	vuint32_t	pio1_12;
	vuint32_t	pio1_13;
	vuint32_t	pio1_14;
	vuint32_t	pio1_15;

	vuint32_t	pio1_16;	/* 0xa0 */
	vuint32_t	pio1_17;
	vuint32_t	pio1_18;
	vuint32_t	pio1_19;

	vuint32_t	pio1_20;
	vuint32_t	pio1_21;
	vuint32_t	pio1_22;
	vuint32_t	pio1_23;

	vuint32_t	pio1_24;	/* 0xc0 */
	vuint32_t	pio1_25;
	vuint32_t	pio1_26;
	vuint32_t	pio1_27;

	vuint32_t	pio1_28;
	vuint32_t	pio1_29;
	vuint32_t	pio1_30;
	vuint32_t	pio1_31;
};

extern struct lpc_ioconf lpc_ioconf;

#define LPC_IOCONF_FUNC		0

/* PIO0_0 */
#define  LPC_IOCONF_FUNC_RESET		0
#define  LPC_IOCONF_FUNC_PIO0_0		1

/* PIO0_1 */
#define  LPC_IOCONF_FUNC_PIO0_1		0
#define  LPC_IOCONF_FUNC_CLKOUT		1
#define  LPC_IOCONF_FUNC_CT32B0_MAT2	2
#define  LPC_IOCONF_FUNC_USB_FTOGGLE	3

/* PIO0_2 */
#define  LPC_IOCONF_FUNC_PIO0_2		0
#define  LPC_IOCONF_FUNC_SSEL0		1
#define  LPC_IOCONF_FUNC_CT16B0_CAP0	2

/* PIO0_3 */
#define  LPC_IOCONF_FUNC_PIO0_3		0
#define  LPC_IOCONF_FUNC_USB_VBUS	1

/* PIO0_4 */
#define  LPC_IOCONF_FUNC_PIO0_4		0
#define  LPC_IOCONF_FUNC_I2C_SCL	1

/* PIO0_5 */
#define  LPC_IOCONF_FUNC_PIO0_5		0
#define  LPC_IOCONF_FUNC_I2C_SDA	1

/* PIO0_6 */
#define  LPC_IOCONF_FUNC_PIO0_6		0
#define  LPC_IOCONF_FUNC_USB_CONNECT	1
#define  LPC_IOCONF_FUNC_PIO0_6_SCK0	2

/* PIO0_7 */
#define  LPC_IOCONF_FUNC_PIO0_7		0
#define  LPC_IOCONF_FUNC_CTS		1

/* PIO0_8 */
#define  LPC_IOCONF_FUNC_PIO0_8		0
#define  LPC_IOCONF_FUNC_MISO0		1
#define  LPC_IOCONF_FUNC_CT16B0_MAT0	2

/* PIO0_9 */
#define  LPC_IOCONF_FUNC_PIO0_9		0
#define  LPC_IOCONF_FUNC_MOSI0		1
#define  LPC_IOCONF_FUNC_CT16B0_MAT1	2

/* PIO0_10 */
#define  LPC_IOCONF_FUNC_SWCLK		0
#define  LPC_IOCONF_FUNC_PIO0_10	1
#define  LPC_IOCONF_FUNC_PIO0_10_SCK0	2
#define  LPC_IOCONF_FUNC_CT16B0_MAT2	3

/* PIO0_11 */
#define  LPC_IOCONF_FUNC_TDI		0
#define  LPC_IOCONF_FUNC_PIO0_11	1
#define  LPC_IOCONF_FUNC_AD0		2
#define  LPC_IOCONF_FUNC_CT32B0_MAT3	3

/* PIO0_12 */
#define  LPC_IOCONF_FUNC_TMS		0
#define  LPC_IOCONF_FUNC_PIO0_12	1
#define  LPC_IOCONF_FUNC_AD1		2
#define  LPC_IOCONF_FUNC_CT32B1_CAP0	3

/* PIO0_13 */
#define  LPC_IOCONF_FUNC_TD0		0
#define  LPC_IOCONF_FUNC_PIO0_13	1
#define  LPC_IOCONF_FUNC_AD2		2
#define  LPC_IOCONF_FUNC_CT32B1_MAT0	3

/* PIO0_14 */
#define  LPC_IOCONF_FUNC_TRST		0
#define  LPC_IOCONF_FUNC_PIO0_14	1
#define  LPC_IOCONF_FUNC_AD3		2
#define  LPC_IOCONF_FUNC_PIO0_14_CT32B1_MAT1	3

/* PIO0_15 */
#define  LPC_IOCONF_FUNC_SWDIO		0
#define  LPC_IOCONF_FUNC_PIO0_15	1
#define  LPC_IOCONF_FUNC_AD4		2
#define  LPC_IOCONF_FUNC_CT32B1_MAT2	3

/* PIO0_16 */
#define  LPC_IOCONF_FUNC_PIO0_16	0
#define  LPC_IOCONF_FUNC_AD5		1
#define  LPC_IOCONF_FUNC_CT32B1_MAT3	2

/* PIO0_17 */
#define  LPC_IOCONF_FUNC_PIO0_17	0
#define  LPC_IOCONF_FUNC_RTS		1
#define  LPC_IOCONF_FUNC_CT32B0_CAP0	2
#define  LPC_IOCONF_FUNC_SCLK		3

/* PIO0_18 */
#define  LPC_IOCONF_FUNC_PIO0_18		0
#define  LPC_IOCONF_FUNC_PIO0_18_RXD		1
#define  LPC_IOCONF_FUNC_PIO0_18_CT32B0_MAT0	2

/* PIO0_19 */
#define  LPC_IOCONF_FUNC_PIO0_19		0
#define  LPC_IOCONF_FUNC_PIO0_19_TXD		1
#define  LPC_IOCONF_FUNC_PIO0_19_CT32B0_MAT1	2

/* PIO0_20 */
#define  LPC_IOCONF_FUNC_PIO0_20	0
#define  LPC_IOCONF_FUNC_CT16B1_CAP0	1

/* PIO0_21 */
#define  LPC_IOCONF_FUNC_PIO0_21	0
#define  LPC_IOCONF_FUNC_CT16B1_MAT0	1
#define  LPC_IOCONF_FUNC_PIO0_21_MOSI1	2

/* PIO0_22 */
#define  LPC_IOCONF_FUNC_PIO0_22	0
#define  LPC_IOCONF_FUNC_AD6		1
#define  LPC_IOCONF_FUNC_CT16B1_MAT1	2
#define  LPC_IOCONF_FUNC_PIO0_22_MISO1	3

/* PIO0_23 */
#define  LPC_IOCONF_FUNC_PIO0_23	0
#define  LPC_IOCONF_FUNC_AD7		1

/* PIO1_0 */
#define  LPC_IOCONF_FUNC_PIO1_0		0
#define  LPC_IOCONF_FUNC_CT32B1_MAT1	1

/* PIO1_1 */
#define  LPC_IOCONF_FUNC_PIO1_1		0
#define  LPC_IOCONF_FUNC_CT32B1_MAT1	1

/* PIO1_2 */
#define  LPC_IOCONF_FUNC_PIO1_2		0
#define  LPC_IOCONF_FUNC_PIO1_2_CT32B1_MAT2	1

/* PIO1_3*/
#define  LPC_IOCONF_FUNC_PIO1_3		0
#define  LPC_IOCONF_FUNC_PIO1_3_CT32B1_MAT3	1

/* PIO1_4 */
#define  LPC_IOCONF_FUNC_PIO1_4		0
#define  LPC_IOCONF_FUNC_PIO1_4_CT32B1_CAP0	1

/* PIO1_5 */
#define  LPC_IOCONF_FUNC_PIO1_5		0
#define  LPC_IOCONF_FUNC_CT32B1_CAP1	1

/* PIO1_6 */
#define  LPC_IOCONF_FUNC_PIO1_6		0

/* PIO1_7 */
#define  LPC_IOCONF_FUNC_PIO1_7		0

/* PIO1_8 */
#define  LPC_IOCONF_FUNC_PIO1_8		0

/* PIO1_9 */
#define  LPC_IOCONF_FUNC_PIO1_9		0

/* PIO1_10 */
#define  LPC_IOCONF_FUNC_PIO1_10	0

/* PIO1_11 */
#define  LPC_IOCONF_FUNC_PIO1_11	0

/* PIO1_12 */
#define  LPC_IOCONF_FUNC_PIO1_12	0

/* PIO1_13 */
#define  LPC_IOCONF_FUNC_PIO1_13	0
#define  LPC_IOCONF_FUNC_DTR		1
#define  LPC_IOCONF_FUNC_CT16B0_MAT0	2
#define  LPC_IOCONF_FUNC_PIO1_13_TXD		3

/* PIO1_14 */
#define  LPC_IOCONF_FUNC_PIO1_14	0
#define  LPC_IOCONF_FUNC_DSR		1
#define  LPC_IOCONF_FUNC_CT16B0_MAT1	2
#define  LPC_IOCONF_FUNC_PIO1_13_RXD		3

/* PIO1_15 */
#define  LPC_IOCONF_FUNC_PIO1_15	0
#define  LPC_IOCONF_FUNC_DCD		1
#define  LPC_IOCONF_FUNC_PIO1_15_CT16B0_MAT2	2
#define  LPC_IOCONF_FUNC_PIO1_15_SCK1	3

/* PIO1_16 */
#define  LPC_IOCONF_FUNC_PIO1_16	0
#define  LPC_IOCONF_FUNC_RI		1
#define  LPC_IOCONF_FUNC_CT16B0_CAP0	2

/* PIO1_17 */
#define  LPC_IOCONF_FUNC_PIO1_17	0
#define  LPC_IOCONF_FUNC_CT16B0_CAP1	1
#define  LPC_IOCONF_FUNC_PIO1_17_RXD		2

/* PIO1_18 */
#define  LPC_IOCONF_FUNC_PIO1_18	0
#define  LPC_IOCONF_FUNC_CT16B1_CAP1	1
#define  LPC_IOCONF_FUNC_PIO1_18_TXD		2

/* PIO1_19 */
#define  LPC_IOCONF_FUNC_PIO1_19	0
#define  LPC_IOCONF_FUNC_DTR		1
#define  LPC_IOCONF_FUNC_SSEL1		2

/* PIO1_20 */
#define  LPC_IOCONF_FUNC_PIO1_20	0
#define  LPC_IOCONF_FUNC_DSR		1
#define  LPC_IOCONF_FUNC_PIO1_20_SCK1		2

/* PIO1_21 */
#define  LPC_IOCONF_FUNC_PIO1_21	0
#define  LPC_IOCONF_FUNC_DCD		1
#define  LPC_IOCONF_FUNC_PIO1_21_MISO1		2

/* PIO1_22 */
#define  LPC_IOCONF_FUNC_PIO1_22	0
#define  LPC_IOCONF_FUNC_RI		1
#define  LPC_IOCONF_FUNC_PIO1_22_MOSI1	2

/* PIO1_23 */
#define  LPC_IOCONF_FUNC_PIO1_23	0
#define  LPC_IOCONF_FUNC_PIO1_23_CT16B1_MAT1	1
#define  LPC_IOCONF_FUNC_SSEL1		2

/* PIO1_24 */
#define  LPC_IOCONF_FUNC_PIO1_24	0
#define  LPC_IOCONF_FUNC_PIO1_24_CT32B0_MAT0	1

/* PIO1_25 */
#define  LPC_IOCONF_FUNC_PIO1_25	0
#define  LPC_IOCONF_FUNC_PIO1_25_CT32B0_MAT1	1

/* PIO1_26 */
#define  LPC_IOCONF_FUNC_PIO1_26	0
#define  LPC_IOCONF_FUNC_PIO1_26_CT32B0_MAT2	1
#define  LPC_IOCONF_FUNC_PIO1_26_RXD		2

/* PIO1_27 */
#define  LPC_IOCONF_FUNC_PIO1_27	0
#define  LPC_IOCONF_FUNC_PIO1_27_CT32B0_MAT3	1
#define  LPC_IOCONF_FUNC_PIO1_27_TXD		2

/* PIO1_28 */
#define  LPC_IOCONF_FUNC_PIO1_28	0
#define  LPC_IOCONF_FUNC_PIO1_28_CT32B0_CAP0	1
#define  LPC_IOCONF_FUNC_PIO1_28_SCLK		2

/* PIO1_29 */
#define  LPC_IOCONF_FUNC_PIO1_29		0
#define  LPC_IOCONF_FUNC_PIO1_29_SCK0		1
#define  LPC_IOCONF_FUNC_PIO1_29_CT32B0_CAP1	2

/* PIO1_31 */
#define  LPC_IOCONF_FUNC_PIO1_31	0

#define  LPC_IOCONF_FUNC_MASK		0x7

#define ao_lpc_alternate(func) (((func) << LPC_IOCONF_FUNC) | \
				(LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) | \
				(0 << LPC_IOCONF_HYS) |			\
				(0 << LPC_IOCONF_INV) |			\
				(0 << LPC_IOCONF_OD) |			\
				0x80)

#define LPC_IOCONF_MODE			3
#define  LPC_IOCONF_MODE_INACTIVE		0
#define  LPC_IOCONF_MODE_PULL_DOWN		1
#define  LPC_IOCONF_MODE_PULL_UP		2
#define  LPC_IOCONF_MODE_REPEATER		3
#define  LPC_IOCONF_MODE_MASK			3

#define LPC_IOCONF_HYS			5

#define LPC_IOCONF_INV			6
#define LPC_IOCONF_ADMODE		7
#define LPC_IOCONF_FILTR		8
#define LPC_IOCONF_OD			10

struct lpc_scb {
	vuint32_t	sysmemremap;	/* 0x00 */
	vuint32_t	presetctrl;
	vuint32_t	syspllctrl;
	vuint32_t	syspllstat;

	vuint32_t	usbpllctrl;	/* 0x10 */
	vuint32_t	usbpllstat;
	uint32_t	r18;
	uint32_t	r1c;

	vuint32_t	sysoscctrl;	/* 0x20 */
	vuint32_t	wdtoscctrl;
	uint32_t	r28;
	uint32_t	r2c;

	vuint32_t	sysrststat;	/* 0x30 */
	uint32_t	r34;
	uint32_t	r38;
	uint32_t	r3c;

	vuint32_t	syspllclksel;	/* 0x40 */
	vuint32_t	syspllclkuen;
	vuint32_t	usbpllclksel;
	vuint32_t	usbpllclkuen;

	uint32_t	r50[8];

	vuint32_t	mainclksel;	/* 0x70 */
	vuint32_t	mainclkuen;
	vuint32_t	sysahbclkdiv;
	uint32_t	r7c;		

	vuint32_t	sysahbclkctrl;	/* 0x80 */
	uint32_t	r84[3];

	uint32_t	r90;		/* 0x90 */
	vuint32_t	ssp0clkdiv;
	vuint32_t	uartclkdiv;
	vuint32_t	ssp1clkdiv;

	uint32_t	ra0[8];

	vuint32_t	usbclksel;	/* 0xc0 */
	vuint32_t	usbclkuen;
	vuint32_t	usbclkdiv;
	uint32_t	rcc;

	uint32_t	rd0[4];
	
	vuint32_t	clkoutsel;	/* 0xe0 */
	vuint32_t	clkoutuen;
	vuint32_t	clkoutdiv;
	uint32_t	rec;
	
	uint32_t	rf0[4];		/* 0xf0 */
	
	vuint32_t	pioporcap0;	/* 0x100 */
	vuint32_t	pioporcap1;
	uint32_t	r102[2];

	uint32_t	r110[4];	/* 0x110 */
	uint32_t	r120[4];	/* 0x120 */
	uint32_t	r130[4];	/* 0x130 */
	uint32_t	r140[4];	/* 0x140 */
	
	vuint32_t	bodctrl;	/* 0x150 */
	vuint32_t	systckcal;
	uint32_t	r158[2];

	uint32_t	r160[4];	/* 0x160 */

	vuint32_t	irqlatency;	/* 0x170 */
	vuint32_t	nmisrc;
	vuint32_t	pintsel[8];

	vuint32_t	usbclkctrl;	/* 0x198 */
	vuint32_t	usbclkst;

	uint32_t	r1a0[6*4];	/* 0x1a0 */

	uint32_t	r200;		/* 0x200 */
	vuint32_t	starterp0;
	uint32_t	r208[2];

	uint32_t	r210;		/* 0x210 */
	vuint32_t	starterp1;
	uint32_t	r218[2];

	uint32_t	r220[4];	/* 0x220 */

	vuint32_t	pdsleepcfg;	/* 0x230 */
	vuint32_t	pdawakecfg;
	vuint32_t	pdruncfg;
	uint32_t	r23c;

	uint32_t	r240[12 * 4];	/* 0x240 */

	uint32_t	r300[15 * 4];	/* 0x300 */
			     
	uint32_t	r3f0;		/* 0x3f0 */
	vuint32_t	device_id;
};

extern struct lpc_scb lpc_scb;

#define LPC_SCB_SYSMEMREMAP_MAP		0
# define LPC_SCB_SYSMEMREMAP_MAP_BOOT_LOADER	0
# define LPC_SCB_SYSMEMREMAP_MAP_RAM		1
# define LPC_SCB_SYSMEMREMAP_MAP_FLASH		2

#define LPC_SCB_PRESETCTRL_SSP0_RST_N	0
#define LPC_SCB_PRESETCTRL_I2C_RST_N	1
#define LPC_SCB_PRESETCTRL_SSP1_RST_N	2

#define LPC_SCB_SYSPLLCTRL_MSEL		0
#define LPC_SCB_SYSPLLCTRL_PSEL		5
#define  LPC_SCB_SYSPLLCTRL_PSEL_1		0
#define  LPC_SCB_SYSPLLCTRL_PSEL_2		1
#define  LPC_SCB_SYSPLLCTRL_PSEL_4		2
#define  LPC_SCB_SYSPLLCTRL_PSEL_8		3
#define  LPC_SCB_SYSPLLCTRL_PSEL_MASK		3

#define LPC_SCB_SYSPLLSTAT_LOCK		0

#define LPC_SCB_USBPLLCTRL_MSEL		0
#define LPC_SCB_USBPLLCTRL_PSEL		5
#define  LPC_SCB_USBPLLCTRL_PSEL_1		0
#define  LPC_SCB_USBPLLCTRL_PSEL_2		1
#define  LPC_SCB_USBPLLCTRL_PSEL_4		2
#define  LPC_SCB_USBPLLCTRL_PSEL_8		3
#define  LPC_SCB_USBPLLCTRL_PSEL_MASK		3

#define LPC_SCB_USBPLLSTAT_LOCK		0

#define LPC_SCB_SYSOSCCTRL_BYPASS	0
#define LPC_SCB_SYSOSCCTRL_FREQRANGE	1
#define  LPC_SCB_SYSOSCCTRL_FREQRANGE_1_20	0
#define  LPC_SCB_SYSOSCCTRL_FREQRANGE_15_25	1

#define LPC_SCB_WDTOSCCTRL_DIVSEL		0
#define  LPC_SCB_WDTOSCCTRL_DIVSEL_MASK			0x1f
#define LPC_SCB_WDTOSCCTRL_FREQSEL		5
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_0_6			1
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_1_05		2
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_1_4			3
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_1_75		4
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_2_1			5
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_2_4			6
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_2_7			7
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_3_0			8
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_3_25		9
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_3_5			0x0a
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_3_75		0x0b
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_4_0			0x0c
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_4_2			0x0d
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_4_4			0x0e
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_4_6			0x0f
#define  LPC_SCB_WDTOSCCTRL_FREQSEL_MASK		0x0f

#define LPC_SCB_SYSRSTSTAT_POR		0
#define LPC_SCB_SYSRSTSTAT_EXTRST	1
#define LPC_SCB_SYSRSTSTAT_WDT		2
#define LPC_SCB_SYSRSTSTAT_BOD		3
#define LPC_SCB_SYSRSTSTAT_SYSRST	4

#define LPC_SCB_SYSPLLCLKSEL_SEL	0
#define  LPC_SCB_SYSPLLCLKSEL_SEL_IRC		0
#define  LPC_SCB_SYSPLLCLKSEL_SEL_SYSOSC	1
#define  LPC_SCB_SYSPLLCLKSEL_SEL_MASK		3

#define LPC_SCB_SYSPLLCLKUEN_ENA	0

#define LPC_SCB_USBPLLCLKSEL_SEL	0
#define  LPC_SCB_USBPLLCLKSEL_SEL_IRC		0
#define  LPC_SCB_USBPLLCLKSEL_SEL_SYSOSC	1
#define  LPC_SCB_USBPLLCLKSEL_SEL_MASK		3

#define LPC_SCB_USBPLLCLKUEN_ENA	0

#define LPC_SCB_MAINCLKSEL_SEL		0
#define  LPC_SCB_MAINCLKSEL_SEL_IRC		0
#define  LPC_SCB_MAINCLKSEL_SEL_PLL_INPUT	1
#define  LPC_SCB_MAINCLKSEL_SEL_WATCHDOG	2
#define  LPC_SCB_MAINCLKSEL_SEL_PLL_OUTPUT	3
#define  LPC_SCB_MAINCLKSEL_SEL_MASK		3

#define LPC_SCB_MAINCLKUEN_ENA		0

#define LPC_SCB_SYSAHBCLKDIV_DIV	0

#define LPC_SCB_SYSAHBCLKCTRL_SYS	0
#define LPC_SCB_SYSAHBCLKCTRL_ROM	1
#define LPC_SCB_SYSAHBCLKCTRL_RAM0	2
#define LPC_SCB_SYSAHBCLKCTRL_FLASHREG	3
#define LPC_SCB_SYSAHBCLKCTRL_FLASHARRAY	4
#define LPC_SCB_SYSAHBCLKCTRL_I2C	5
#define LPC_SCB_SYSAHBCLKCTRL_GPIO	6
#define LPC_SCB_SYSAHBCLKCTRL_CT16B0	7
#define LPC_SCB_SYSAHBCLKCTRL_CT16B1	8
#define LPC_SCB_SYSAHBCLKCTRL_CT32B0	9
#define LPC_SCB_SYSAHBCLKCTRL_CT32B1	10
#define LPC_SCB_SYSAHBCLKCTRL_SSP0	11
#define LPC_SCB_SYSAHBCLKCTRL_USART	12
#define LPC_SCB_SYSAHBCLKCTRL_ADC	13
#define LPC_SCB_SYSAHBCLKCTRL_USB	14
#define LPC_SCB_SYSAHBCLKCTRL_WWDT	15
#define LPC_SCB_SYSAHBCLKCTRL_IOCON	16
#define LPC_SCB_SYSAHBCLKCTRL_SSP1	18
#define LPC_SCB_SYSAHBCLKCTRL_PINT	19
#define LPC_SCB_SYSAHBCLKCTRL_GROUP0INT	23
#define LPC_SCB_SYSAHBCLKCTRL_GROUP1INT	24
#define LPC_SCB_SYSAHBCLKCTRL_RAM1	26
#define LPC_SCB_SYSAHBCLKCTRL_USBRAM	27

#define LPC_SCB_SSP0CLKDIV_
#define LPC_SCB_UARTCLKDIV_
#define LPC_SCB_SSP1CLKDIV_

#define LPC_SCB_USBCLKSEL_SEL		0
#define LPC_SCB_USBCLKSEL_SEL_USB_PLL		0
#define LPC_SCB_USBCLKSEL_SEL_MAIN_CLOCK	1

#define LPC_SCB_USBCLKUEN_ENA		0
#define LPC_SCB_USBCLKDIV_DIV		0

#define LPC_SCB_CLKOUTSEL_SEL		0
#define  LPC_SCB_CLKOUTSEL_SEL_IRC		0
#define  LPC_SCB_CLKOUTSEL_SEL_SYSOSC		1
#define  LPC_SCB_CLKOUTSEL_SEL_LF		2
#define  LPC_SCB_CLKOUTSEL_SEL_MAIN_CLOCK	3

#define LPC_SCB_CLKOUTUEN_ENA		0

#define LPC_SCB_BOD_BODRSTLEV		0
# define LPC_SCB_BOD_BODRSTLEV_1_46		0
# define LPC_SCB_BOD_BODRSTLEV_2_06		1
# define LPC_SCB_BOD_BODRSTLEV_2_35		2
# define LPC_SCB_BOD_BODRSTLEV_2_63		3
#define LPC_SCB_BOD_BODINTVAL		2
# define LPC_SCB_BOD_BODINTVAL_RESERVED		0
# define LPC_SCB_BOD_BODINTVAL_2_22		1
# define LPC_SCB_BOD_BODINTVAL_2_52	       	2
# define LPC_SCB_BOD_BODINTVAL_2_80		3
#define LPC_SCB_BOD_BODRSTENA		4

#define LPC_SCB_PDRUNCFG_IRCOUT_PD	0
#define LPC_SCB_PDRUNCFG_IRC_PD		1
#define LPC_SCB_PDRUNCFG_FLASH_PD	2
#define LPC_SCB_PDRUNCFG_BOD_PD		3
#define LPC_SCB_PDRUNCFG_ADC_PD		4
#define LPC_SCB_PDRUNCFG_SYSOSC_PD	5
#define LPC_SCB_PDRUNCFG_WDTOSC_PD	6
#define LPC_SCB_PDRUNCFG_SYSPLL_PD	7
#define LPC_SCB_PDRUNCFG_USBPLL_PD	8
#define LPC_SCB_PDRUNCFG_USBPAD_PD	10

struct lpc_flash {
	uint32_t	r0[4];		/* 0x0 */

	vuint32_t	flashcfg;	/* 0x10 */
};

extern struct lpc_flash lpc_flash;

struct lpc_gpio_pin {
	vuint32_t	isel;		/* 0x00 */
	vuint32_t	ienr;
	vuint32_t	sienr;
	vuint32_t	cienr;

	vuint32_t	ienf;		/* 0x10 */
	vuint32_t	sienf;
	vuint32_t	cienf;
	vuint32_t	rise;

	vuint32_t	fall;		/* 0x20 */
	vuint32_t	ist;
};

extern struct lpc_gpio_pin lpc_gpio_pin;

struct lpc_gpio_group0 {
};

extern struct lpc_gpio_group0 lpc_gpio_group0;

struct lpc_gpio_group1 {
};

extern struct lpc_gpio_group1 lpc_gpio_group1;

struct lpc_gpio {
	vuint8_t	byte[0x40];	/* 0x0000 */

	uint8_t		r0030[0x1000 - 0x40];

	vuint32_t	word[0x40];	/* 0x1000 */

	uint8_t		r1100[0x2000 - 0x1100];
	
	vuint32_t	dir[2];		/* 0x2000 */

	uint8_t		r2008[0x2080 - 0x2008];

	vuint32_t	mask[2];	/* 0x2080 */

	uint8_t		r2088[0x2100 - 0x2088];

	vuint32_t	pin[2];		/* 0x2100 */

	uint8_t		r2108[0x2200 - 0x2108];

	vuint32_t	set[2];		/* 0x2200 */

	uint8_t		r2208[0x2280 - 0x2208];

	vuint32_t	clr[2];		/* 0x2280 */

	uint8_t		r2288[0x2300 - 0x2288];

	vuint32_t	not[2];		/* 0x2300 */
};

extern struct lpc_gpio lpc_gpio;

struct lpc_systick {
	uint8_t		r0000[0x10];	/* 0x0000 */

	vuint32_t	csr;		/* 0x0010 */
	vuint32_t	rvr;
	vuint32_t	cvr;
	vuint32_t	calib;
};

extern struct lpc_systick lpc_systick;

#define LPC_SYSTICK_CSR_ENABLE		0
#define LPC_SYSTICK_CSR_TICKINT		1
#define LPC_SYSTICK_CSR_CLKSOURCE	2
#define  LPC_SYSTICK_CSR_CLKSOURCE_CPU_OVER_2		0
#define  LPC_SYSTICK_CSR_CLKSOURCE_CPU			1
#define LPC_SYSTICK_CSR_COUNTFLAG	16

struct lpc_usart {
	vuint32_t	rbr_thr;	/* 0x0000 */
	vuint32_t	ier;
	vuint32_t	iir_fcr;
	vuint32_t	lcr;

	vuint32_t	mcr;		/* 0x0010 */
	vuint32_t	lsr;
	vuint32_t	msr;
	vuint32_t	scr;

	vuint32_t	acr;		/* 0x0020 */
	vuint32_t	icr;
	vuint32_t	fdr;
	vuint32_t	osr;

	vuint32_t	ter;		/* 0x0030 */
	uint32_t	r34[3];

	vuint32_t	hden;		/* 0x0040 */
	uint32_t	r44;
	vuint32_t	scictrl;
	vuint32_t	rs485ctrl;

	vuint32_t	rs485addrmatch;	/* 0x0050 */
	vuint32_t	rs485dly;
	vuint32_t	syncctrl;
};

extern struct lpc_usart lpc_usart;

#define LPC_USART_IER_RBRINTEN	0
#define LPC_USART_IER_THREINTEN	1
#define LPC_USART_IER_RSLINTEN	2
#define LPC_USART_IER_MSINTEN	3
#define LPC_USART_IER_ABEOINTEN	8
#define LPC_USART_IER_ABTOINTEN	9

#define LPC_USART_IIR_INTSTATUS		0
#define LPC_USART_IIR_INTID		1
#define LPC_USART_IIR_INTID_RLS			3
#define LPC_USART_IIR_INTID_RDA			2
#define LPC_USART_IIR_INTID_CTI			6
#define LPC_USART_IIR_INTID_THRE		1
#define LPC_USART_IIR_INTID_MS			0
#define LPC_USART_IIR_INTID_MASK		7
#define LPC_USART_IIR_FIFOEN		6
#define LPC_USART_IIR_ABEOINT		8
#define LPC_USART_IIR_ABTOINT		9

#define LPC_USART_FCR_FIFOEN		0
#define LPC_USART_FCR_RXFIFORES		1
#define LPC_USART_FCR_TXFIFORES		2
#define LPC_USART_FCR_RXTL		6
#define LPC_USART_FCR_RXTL_1			0
#define LPC_USART_FCR_RXTL_4			1
#define LPC_USART_FCR_RXTL_8			2
#define LPC_USART_FCR_RXTL_14			3

#define LPC_USART_LCR_WLS	0
#define LPC_USART_LCR_WLS_5		0
#define LPC_USART_LCR_WLS_6		1
#define LPC_USART_LCR_WLS_7		2
#define LPC_USART_LCR_WLS_8		3
#define LPC_USART_LCR_WLS_MASK		3
#define LPC_USART_LCR_SBS	2
#define LPC_USART_LCR_SBS_1		0
#define LPC_USART_LCR_SBS_2		1
#define LPC_USART_LCR_SBS_MASK		1
#define LPC_USART_LCR_PE	3
#define LPC_USART_LCR_PS	4
#define LPC_USART_LCR_PS_ODD		0
#define LPC_USART_LCR_PS_EVEN		1
#define LPC_USART_LCR_PS_ONE		2
#define LPC_USART_LCR_PS_ZERO		3
#define LPC_USART_LCR_PS_MASK		3
#define LPC_USART_LCR_BC	6
#define LPC_USART_LCR_DLAB	7

#define LPC_USART_MCR_DTRCTRL	0
#define LPC_USART_MCR_RTSCTRL	1
#define LPC_USART_MCR_LMS	4
#define LPC_USART_MCR_RTSEN	6
#define LPC_USART_MCR_CTSEN	7

#define LPC_USART_LSR_RDR	0
#define LPC_USART_LSR_OE	1
#define LPC_USART_LSR_PE	2
#define LPC_USART_LSR_FE	3
#define LPC_USART_LSR_BI	4
#define LPC_USART_LSR_THRE	5
#define LPC_USART_LSR_TEMT	6
#define LPC_USART_LSR_RXFE	7
#define LPC_USART_LSR_TXERR	8

#define LPC_USART_MSR_DCTS	0
#define LPC_USART_MSR_DDSR	1
#define LPC_USART_MSR_TERI	2
#define LPC_USART_MSR_DDCD	3
#define LPC_USART_MSR_CTS	4
#define LPC_USART_MSR_DSR	5
#define LPC_USART_MSR_RI	6
#define LPC_USART_MSR_DCD	7

#define LPC_USART_ACR_START	0
#define LPC_USART_ACR_MODE	1
#define LPC_USART_ACR_AUTORESTART	2
#define LPC_USART_ACR_ABEOINTCLR	8
#define LPC_USART_ACR_ABTOINTCLR	9

#define LPC_USART_FDR_DIVADDVAL	0
#define LPC_USART_FDR_MULVAL	4

#define LPC_USART_OSR_OSFRAC	1
#define LPC_USART_OSR_OSINT	4
#define LPC_USART_OSR_FDINT	8

#define LPC_USART_TER_TXEN	7

#define LPC_USART_HDEN_HDEN	0

struct lpc_usb {
	vuint32_t	devcmdstat;
	vuint32_t	info;
	vuint32_t	epliststart;
	vuint32_t	databufstart;
	vuint32_t	lpm;
	vuint32_t	epskip;
	vuint32_t	epinuse;
	vuint32_t	epbufcfg;
	vuint32_t	intstat;
	vuint32_t	inten;
	vuint32_t	intsetstat;
	vuint32_t	introuting;
	uint32_t	r30;
	vuint32_t	eptoggle;
} lpc_usb;

extern struct lpc_usb lpc_usb;

#define LPC_USB_DEVCMDSTAT_DEV_ADDR	0
#define LPC_USB_DEVCMDSTAT_DEV_ADDR_MASK	0x7f
#define LPC_USB_DEVCMDSTAT_DEV_EN	7
#define LPC_USB_DEVCMDSTAT_SETUP	8
#define LPC_USB_DEVCMDSTAT_PLL_ON	9
#define LPC_USB_DEVCMDSTAT_LPM_SUP	11
#define LPC_USB_DEVCMDSTAT_INTONNAK_AO	12
#define LPC_USB_DEVCMDSTAT_INTONNAK_AI	13
#define LPC_USB_DEVCMDSTAT_INTONNAK_CO	14
#define LPC_USB_DEVCMDSTAT_INTONNAK_CI	15
#define LPC_USB_DEVCMDSTAT_DCON		16
#define LPC_USB_DEVCMDSTAT_DSUS		17
#define LPC_USB_DEVCMDSTAT_LPM_SUS	19
#define LPC_USB_DEVCMDSTAT_LPM_REWP	20
#define LPC_USB_DEVCMDSTAT_DCON_C	24
#define LPC_USB_DEVCMDSTAT_DSUS_C	25
#define LPC_USB_DEVCMDSTAT_DRES_C	26
#define LPC_USB_DEVCMDSTAT_VBUSDEBOUNCED	28

#define LPC_USB_INFO_FRAME_NR		0
#define LPC_USB_INFO_FRAME_NR_MASK	0x3ff
#define LPC_USB_INFO_ERR_CODE		11
#define LPC_USB_INFO_ERR_CODE_NO_ERROR			0
#define LPC_USB_INFO_ERR_CODE_PID_ENCODING_ERROR	1
#define LPC_USB_INFO_ERR_CODE_PID_UNKNOWN		2
#define LPC_USB_INFO_ERR_CODE_PACKET_UNEXPECTED		3
#define LPC_USB_INFO_ERR_CODE_TOKEN_CRC_ERROR		4
#define LPC_USB_INFO_ERR_CODE_DATA_CRC_ERROR		5
#define LPC_USB_INFO_ERR_CODE_TIME_OUT			6
#define LPC_USB_INFO_ERR_CODE_BABBLE			7
#define LPC_USB_INFO_ERR_CODE_TRUNCATED_EOP		8
#define LPC_USB_INFO_ERR_CODE_SENT_RECEIVED_NAK		9
#define LPC_USB_INFO_ERR_CODE_SENT_STALL		0xa
#define LPC_USB_INFO_ERR_CODE_OVERRUN			0xb
#define LPC_USB_INFO_ERR_CODE_SENT_EMPTY_PACKET		0xc
#define LPC_USB_INFO_ERR_CODE_BITSTUFF_ERROR		0xd
#define LPC_USB_INFO_ERR_CODE_SYNC_ERROR		0xe
#define LPC_USB_INFO_ERR_CODE_WRONG_DATA_TOGGLE		0xf
#define LPC_USB_INFO_ERR_CODE_MASK			0xf

#define LPC_USB_EPLISTSTART_EP_LIST			0

#define LPC_USB_DATABUFSTART_DA_BUF			0

#define LPC_USB_LPM_HIRD_HW		0
#define LPC_USB_LPM_HIRD_HW_MASK		0xf
#define LPC_USB_LPM_HIRD_SW		4
#define LPC_USB_LPM_HIRD_SW_MASK		0xf
#define LPC_USB_LPM_DATA_PENDING	8

#define LPC_USB_EPSKIP_SKIP		0

#define LPC_USB_EPINUSE_BUF(ep)		(ep)

#define LPC_USB_EPBUFCFG_BUF_SB(ep)	(ep)

#define LPC_USB_INT_EPOUT(ep)		((ep) << 1)
#define LPC_USB_INT_EPIN(ep)		(((ep) << 1) + 1)

#define LPC_USB_INT_FRAME		30
#define LPC_USB_INT_DEV			31

#define LPC_USB_INTIN_EP_INT_EN(ep)	(ep)
#define LPC_USB_INTIN_FRAME_INT_EN	30
#define LPC_USB_INTIN_DEV_INT_EN	31

#define LPC_USB_INTSETSTAT_EP_SET_INT(ep)	(ep)
#define LPC_USB_INTSETSTAT_FRAME_SET_INT	30
#define LPC_USB_INTSETSTAT_DEV_SET_INT		31

#define LPC_USB_INTROUTING_ROUTE_INT(ep)	(ep)
#define LPC_USB_INTROUTING_INT30		30
#define LPC_USB_INTROUTING_INT31		31

#define LPC_USB_EPTOGGLE_TOGGLE(ep)		(ep)

struct lpc_usb_epn {
	vuint32_t		out[2];
	vuint32_t		in[2];
};

struct lpc_usb_endpoint {
	vuint32_t		ep0_out;
	vuint32_t		setup;
	vuint32_t		ep0_in;
	vuint32_t		reserved_0c;
	struct lpc_usb_epn	epn[4];
};

/* Assigned in registers.ld to point at the base
 * of USB ram
 */

extern uint8_t lpc_usb_sram[];

#define LPC_USB_EP_ACTIVE		31
#define LPC_USB_EP_DISABLED		30
#define LPC_USB_EP_STALL		29
#define LPC_USB_EP_TOGGLE_RESET		28
#define LPC_USB_EP_RATE_FEEDBACK	27
#define LPC_USB_EP_ENDPOINT_ISO		26
#define LPC_USB_EP_NBYTES		16
#define  LPC_USB_EP_NBYTES_MASK			0x3ff
#define LPC_USB_EP_OFFSET		0

#define LPC_ISR_PIN_INT0_POS	0
#define LPC_ISR_PIN_INT1_POS	1
#define LPC_ISR_PIN_INT2_POS	2
#define LPC_ISR_PIN_INT3_POS	3
#define LPC_ISR_PIN_INT4_POS	4
#define LPC_ISR_PIN_INT5_POS	5
#define LPC_ISR_PIN_INT6_POS	6
#define LPC_ISR_PIN_INT7_POS	7
#define LPC_ISR_GINT0_POS	8
#define LPC_ISR_GINT1_POS	9
#define LPC_ISR_SSP1_POS	14
#define LPC_ISR_I2C_POS		15
#define LPC_ISR_CT16B0_POS	16
#define LPC_ISR_CT16B1_POS	17
#define LPC_ISR_CT32B0_POS	18
#define LPC_ISR_CT32B1_POS	19
#define LPC_ISR_SSP0_POS	20
#define LPC_ISR_USART_POS	21
#define LPC_ISR_USB_IRQ_POS	22
#define LPC_ISR_USB_FIQ_POS	23
#define LPC_ISR_ADC_POS		24
#define LPC_ISR_WWDT_POS	25
#define LPC_ISR_BOD_POS		26
#define LPC_ISR_FLASH_POS	27
#define LPC_ISR_USB_WAKEUP_POS	30

struct lpc_nvic {
	vuint32_t	iser;		/* 0x000 0xe000e100 Set Enable Register */

	uint8_t		_unused020[0x080 - 0x004];

	vuint32_t	icer;		/* 0x080 0xe000e180 Clear Enable Register */

	uint8_t		_unused0a0[0x100 - 0x084];

	vuint32_t	ispr;		/* 0x100 0xe000e200 Set Pending Register */

	uint8_t		_unused120[0x180 - 0x104];

	vuint32_t	icpr;		/* 0x180 0xe000e280 Clear Pending Register */

	uint8_t		_unused1a0[0x300 - 0x184];

	vuint32_t	ipr[8];		/* 0x300 0xe000e400 Priority Register */
};

extern struct lpc_nvic lpc_nvic;

static inline void
lpc_nvic_set_enable(int irq) {
	lpc_nvic.iser = (1 << irq);
}

static inline void
lpc_nvic_clear_enable(int irq) {
	lpc_nvic.icer = (1 << irq);
}

static inline int
lpc_nvic_enabled(int irq) {
	return (lpc_nvic.iser >> irq) & 1;
}

	
static inline void
lpc_nvic_set_pending(int irq) {
	lpc_nvic.ispr = (1 << irq);
}

static inline void
lpc_nvic_clear_pending(int irq) {
	lpc_nvic.icpr = (1 << irq);
}

static inline int
lpc_nvic_pending(int irq) {
	return (lpc_nvic.ispr >> irq) & 1;
}

#define IRQ_PRIO_REG(irq)	((irq) >> 2)
#define IRQ_PRIO_BIT(irq)	(((irq) & 3) << 3)
#define IRQ_PRIO_MASK(irq)	(0xff << IRQ_PRIO_BIT(irq))

static inline void
lpc_nvic_set_priority(int irq, uint8_t prio) {
	int		n = IRQ_PRIO_REG(irq);
	uint32_t	v;

	v = lpc_nvic.ipr[n];
	v &= ~IRQ_PRIO_MASK(irq);
	v |= (prio) << IRQ_PRIO_BIT(irq);
	lpc_nvic.ipr[n] = v;
}

static inline uint8_t
lpc_nvic_get_priority(int irq) {
	return (lpc_nvic.ipr[IRQ_PRIO_REG(irq)] >> IRQ_PRIO_BIT(irq)) & IRQ_PRIO_MASK(0);
}

struct arm_scb {
	vuint32_t	cpuid;
	vuint32_t	icsr;
	uint32_t	reserved08;
	vuint32_t	aircr;

	vuint32_t	scr;
	vuint32_t	ccr;
	uint32_t	reserved18;
	vuint32_t	shpr2;

	vuint32_t	shpr3;
};

extern struct arm_scb arm_scb;

struct lpc_ssp {
	vuint32_t	cr0;	/* 0x00 */
	vuint32_t	cr1;
	vuint32_t	dr;
	vuint32_t	sr;

	vuint32_t	cpsr;	/* 0x10 */
	vuint32_t	imsc;
	vuint32_t	ris;
	vuint32_t	mis;

	vuint32_t	icr;	/* 0x20 */
};

extern struct lpc_ssp lpc_ssp0, lpc_ssp1;

#define LPC_NUM_SPI		2

#define LPC_SSP_FIFOSIZE	8

#define LPC_SSP_CR0_DSS		0
#define  LPC_SSP_CR0_DSS_4		0x3
#define  LPC_SSP_CR0_DSS_5		0x4
#define  LPC_SSP_CR0_DSS_6		0x5
#define  LPC_SSP_CR0_DSS_7		0x6
#define  LPC_SSP_CR0_DSS_8		0x7
#define  LPC_SSP_CR0_DSS_9		0x8
#define  LPC_SSP_CR0_DSS_10		0x9
#define  LPC_SSP_CR0_DSS_11		0xa
#define  LPC_SSP_CR0_DSS_12		0xb
#define  LPC_SSP_CR0_DSS_13		0xc
#define  LPC_SSP_CR0_DSS_14		0xd
#define  LPC_SSP_CR0_DSS_15		0xe
#define  LPC_SSP_CR0_DSS_16		0xf
#define LPC_SSP_CR0_FRF		4
#define  LPC_SSP_CR0_FRF_SPI		0
#define  LPC_SSP_CR0_FRF_TI		1
#define  LPC_SSP_CR0_FRF_MICROWIRE	2
#define LPC_SSP_CR0_CPOL	6
#define  LPC_SSP_CR0_CPOL_LOW		0
#define  LPC_SSP_CR0_CPOL_HIGH		1
#define LPC_SSP_CR0_CPHA	7
#define  LPC_SSP_CR0_CPHA_FIRST		0
#define  LPC_SSP_CR0_CPHA_SECOND	1
#define LPC_SSP_CR0_SCR		8

#define LPC_SSP_CR1_LBM		0
#define LPC_SSP_CR1_SSE		1
#define LPC_SSP_CR1_MS		2
#define  LPC_SSP_CR1_MS_MASTER		0
#define  LPC_SSP_CR1_MS_SLAVE		1
#define LPC_SSP_CR1_SOD		3

#define LPC_SSP_SR_TFE		0
#define LPC_SSP_SR_TNF		1
#define LPC_SSP_SR_RNE		2
#define LPC_SSP_SR_RFF		3
#define LPC_SSP_SR_BSY		4

#define LPC_SSP_IMSC_RORIM	0
#define LPC_SSP_IMSC_RTIM	1
#define LPC_SSP_IMSC_RXIM	2
#define LPC_SSP_IMSC_TXIM	3

#define LPC_SSP_RIS_RORRIS	0
#define LPC_SSP_RIS_RTRIS	1
#define LPC_SSP_RIS_RXRIS	2
#define LPC_SSP_RIS_TXRIS	3

#define LPC_SSP_MIS_RORMIS	0
#define LPC_SSP_MIS_RTMIS	1
#define LPC_SSP_MIS_RXMIS	2
#define LPC_SSP_MIS_TXMIS	3

#define LPC_SSP_ICR_RORIC	0
#define LPC_SSP_ICR_RTIC	1

struct lpc_adc {
	vuint32_t	cr;	/* 0x00 */
	vuint32_t	gdr;
	uint32_t	r08;
	vuint32_t	inten;

	vuint32_t	dr[8];	/* 0x10 */

	vuint32_t	stat;	/* 0x30 */
};

extern struct lpc_adc lpc_adc;

#define LPC_ADC_CR_SEL		0
#define LPC_ADC_CR_CLKDIV	8
#define LPC_ADC_CR_BURST	16
#define LPC_ADC_CR_CLKS		17
#define  LPC_ADC_CR_CLKS_11		0
#define  LPC_ADC_CR_CLKS_10		1
#define  LPC_ADC_CR_CLKS_9		2
#define  LPC_ADC_CR_CLKS_8		3
#define  LPC_ADC_CR_CLKS_7		4
#define  LPC_ADC_CR_CLKS_6		5
#define  LPC_ADC_CR_CLKS_5		6
#define  LPC_ADC_CR_CLKS_4		7
#define LPC_ADC_CR_START	24
#define  LPC_ADC_CR_START_NONE		0
#define  LPC_ADC_CR_START_NOW		1

#define LPC_ADC_GDR_CHN			24
#define LPC_ADC_GDR_OVERRUN		30
#define LPC_ADC_GDR_DONE		31

#define LPC_ADC_INTEN_ADINTEN	0
#define LPC_ADC_INTEN_ADGINTEN	8

#define LPC_ADC_STAT_DONE	0
#define LPC_ADC_STAT_OVERRUN	8
#define LPC_ADC_STAT_ADINT	16

struct lpc_ct32b {
	vuint32_t	ir;	/* 0x00 */
	vuint32_t	tcr;
	vuint32_t	tc;
	vuint32_t	pr;
	
	vuint32_t	pc;	/* 0x10 */
	vuint32_t	mcr;
	vuint32_t	mr[4];	/* 0x18 */
	vuint32_t	ccr;	/* 0x28 */
	vuint32_t	cr0;
	
	vuint32_t	cr1_0;	/* 0x30 (only for ct32b0 */
	vuint32_t	cr1_1;	/* 0x34 (only for ct32b1 */
	uint32_t	r38;
	vuint32_t	emr;

	uint32_t	r40[12];

	vuint32_t	ctcr;	/* 0x70 */
	vuint32_t	pwmc;
};

extern struct lpc_ct32b lpc_ct32b0, lpc_ct32b1;

#define LPC_CT32B_TCR_CEN	0
#define LPC_CT32B_TCR_CRST	1

#define LPC_CT32B_MCR_MR0R	1

#define LPC_CT32B_PWMC_PWMEN0	0
#define LPC_CT32B_PWMC_PWMEN1	1
#define LPC_CT32B_PWMC_PWMEN2	2
#define LPC_CT32B_PWMC_PWMEN3	3

#define LPC_CT32B_EMR_EMC0	4
#define LPC_CT32B_EMR_EMC1	6
#define LPC_CT32B_EMR_EMC2	8
#define LPC_CT32B_EMR_EMC3	10

#define LPC_CT32B_EMR_EMC_NOTHING	0
#define LPC_CT32B_EMR_EMC_CLEAR		1
#define LPC_CT32B_EMR_EMC_SET		2
#define LPC_CT32B_EMR_EMC_TOGGLE	3

#endif /* _LPC_H_ */
