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

#include "cc1111.h"

unsigned char irqs;

void timer1_isr(void) interrupt 9 __reentrant
{
	++irqs;
	if (irqs == 100) {
		P1 ^= 0x2;
		irqs = 0;
	}
}

int
main (void) __reentrant
{
	CLKCON = 0;
	P1DIR = 0x2;
	P1 = 0xff;

	T1CTL = 0;

	/* 30000 */
	T1CC0H = 0x75;
	T1CC0L = 0x30;
	T1CCTL0 = T1CCTL_MODE_COMPARE;
	T1CCTL1 = 0;
	T1CCTL2 = 0;

	/* clear timer value */
	T1CNTL = 0;
	OVFIM = 1;
	T1CTL = T1CTL_MODE_MODULO | T1CTL_DIV_8;
	T1IE = 1;
	EA = 1;
	for (;;);
}
