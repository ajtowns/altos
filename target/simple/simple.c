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

sfr at 0x90 P1;
sfr at 0xC6 CLKCON;

sfr at 0xFE P1DIR;
sfr at 0xF6 P1INP;

void delay(int n) __reentrant
{
	while (n--)
		_asm nop _endasm;
}
int
main (void) __reentrant
{
	long i;
	CLKCON = 0;
	/* Set p1_1 to output */
	P1DIR = 0x02;
	for (;;) {
		P1 ^= 0x2;
		for (i = 0; i < 1000; i++)
			delay(1000);
	}
}
