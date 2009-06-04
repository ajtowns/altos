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

sfr at 0x80 P0;
sfr at 0x90 P1;
sfr at 0xA0 P2;
sfr at 0xC6 CLKCON;
sfr at 0xbe SLEEP;

# define SLEEP_USB_EN		(1 << 7)
# define SLEEP_XOSC_STB		(1 << 6)

sbit at 0x90 P1_0;
sbit at 0x91 P1_1;
sbit at 0x92 P1_2;
sbit at 0x93 P1_3;
sbit at 0x94 P1_4;
sbit at 0x95 P1_5;
sbit at 0x96 P1_6;
sbit at 0x97 P1_7;

sfr at 0xF1 PERCFG;
sfr at 0xF2 ADCCFG;
sfr at 0xF3 P0SEL;
sfr at 0xF4 P1SEL;
sfr at 0xF5 P2SEL;

#define P2SEL_PRI3P1_USART0		(0 << 6)
#define P2SEL_PRI3P1_USART1		(1 << 6)
#define P2SEL_PRI2P1_USART1		(0 << 5)
#define P2SEL_PRI2P1_TIMER3		(1 << 5)
#define P2SEL_PRI1P1_TIMER1		(0 << 4)
#define P2SEL_PRI1P1_TIMER4		(1 << 4)
#define P2SEL_PRI0P1_USART0		(0 << 3)
#define P2SEL_PRI0P1_TIMER1		(1 << 3)
#define P2SEL_SELP2_4_GPIO		(0 << 2)
#define P2SEL_SELP2_4_PERIPHERAL	(1 << 2)
#define P2SEL_SELP2_3_GPIO		(0 << 1)
#define P2SEL_SELP2_3_PERIPHERAL	(1 << 1)
#define P2SEL_SELP2_0_GPIO		(0 << 0)
#define P2SEL_SELP2_0_PERIPHERAL	(1 << 0)

sfr at 0xFD P0DIR;
sfr at 0xFE P1DIR;
sfr at 0xFF P2DIR;
sfr at 0x8F P0INP;
sfr at 0xF6 P1INP;
sfr at 0xF7 P2INP;

sfr at 0x89 P0IFG;
sfr at 0x8A P1IFG;
sfr at 0x8B P2IFG;

sfr at 0xF1 PERCFG;
#define PERCFG_T1CFG_ALT_1	(0 << 6)
#define PERCFG_T1CFG_ALT_2	(1 << 6)

#define PERCFG_T3CFG_ALT_1	(0 << 5)
#define PERCFG_T3CFG_ALT_2	(1 << 5)

#define PERCFG_T4CFG_ALT_1	(0 << 4)
#define PERCFG_T4CFG_ALT_2	(1 << 4)

#define PERCFG_U1CFG_ALT_1	(0 << 1)
#define PERCFG_U1CFG_ALT_2	(1 << 1)

#define PERCFG_U0CFG_ALT_1	(0 << 0)
#define PERCFG_U0CFG_ALT_2	(1 << 0)

/* Timer count */
sfr at 0xCA T3CNT;
sfr at 0xEA T4CNT;

/* Timer control */

sfr at 0xCB T3CTL;
sfr at 0xEB T4CTL;

#define TxCTL_DIV_1		(0 << 5)
#define TxCTL_DIV_2		(1 << 5)
#define TxCTL_DIV_4		(2 << 5)
#define TxCTL_DIV_8		(3 << 5)
#define TxCTL_DIV_16		(4 << 5)
#define TxCTL_DIV_32		(5 << 5)
#define TxCTL_DIV_64		(6 << 5)
#define TxCTL_DIV_128		(7 << 5)
#define TxCTL_START		(1 << 4)
#define TxCTL_OVFIM		(1 << 3)
#define TxCTL_CLR		(1 << 2)
#define TxCTL_MODE_FREE		(0 << 0)
#define TxCTL_MODE_DOWN		(1 << 0)
#define TxCTL_MODE_MODULO	(2 << 0)
#define TxCTL_MODE_UP_DOWN	(3 << 0)

/* Timer 4 channel 0 compare control */

sfr at 0xCC T3CCTL0;
sfr at 0xCE T3CCTL1;
sfr at 0xEC T4CCTL0;
sfr at 0xEE T4CCTL1;

#define TxCCTLy_IM			(1 << 6)
#define TxCCTLy_CMP_SET			(0 << 3)
#define TxCCTLy_CMP_CLEAR		(1 << 3)
#define TxCCTLy_CMP_TOGGLE		(2 << 3)
#define TxCCTLy_CMP_SET_UP_CLEAR_DOWN	(3 << 3)
#define TxCCTLy_CMP_CLEAR_UP_SET_DOWN	(4 << 3)
#define TxCCTLy_CMP_SET_CLEAR_FF	(5 << 3)
#define TxCCTLy_CMP_CLEAR_SET_00	(6 << 3)
#define TxCCTLy_CMP_MODE_ENABLE		(1 << 2)

/* Timer compare value */
sfr at 0xCD T3CC0;
sfr at 0xCF T3CC1;
sfr at 0xED T4CC0;
sfr at 0xEF T4CC1;

#define nop()	_asm nop _endasm;

void
delay (unsigned char n)
{
	unsigned char i = 0, j = 0;

	n <<= 1;
	while (--n != 0)
		while (--j != 0)
			while (--i != 0)
				nop();
}

void
dit() {
	T4CTL |= TxCTL_START;
	delay(1);
	T4CTL &= ~TxCTL_START;
	delay(1);
}

void
dah () {
	T4CTL |= TxCTL_START;
	delay(3);
	T4CTL &= ~TxCTL_START;
	delay(1);
}

void
charspace () {
	delay(2);
}

void
wordspace () {
	delay(8);
}

#define _ dit();
#define ___ dah();
#define C charspace();
#define W wordspace();

main ()
{
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;

	/* Use timer 4 alternate config 2 */
	PERCFG = PERCFG_T4CFG_ALT_2;
	/* Use P2_4 for timer 4 output */
	P2SEL = P2SEL_SELP2_0_PERIPHERAL;

	T4CCTL0 = TxCCTLy_CMP_TOGGLE|TxCCTLy_CMP_MODE_ENABLE;
	T4CC0 = 125;
	T4CTL = TxCTL_DIV_32 | TxCTL_MODE_MODULO;

	for (;;) {
		___ _ ___ _ C ___ ___ _ ___ W	/* cq */
		___ _ _ C _ W			/* de */
		___ _ ___ C ___ _ _ C		/* kd */
		___ ___ _ _ _ C	_ _ _ C		/* 7s */
		___ ___ _ ___ C	___ ___ _ W	/* qg */
		if (T4CC0 == 94)
			T4CC0 = 125;
		else
			T4CC0 = 94;
	}
}
