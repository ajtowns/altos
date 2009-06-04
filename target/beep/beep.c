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

#define nop()	_asm nop _endasm;

void
delay (unsigned char n)
{
	unsigned char i = 0;

	n <<= 1;
	while (--n != 0) {
		i = 211;
		while (--i != 0)
			nop();
	}
}

void
tone (unsigned char n, unsigned char m)
{
	unsigned char	i = 0;
	while (--m != 0) {
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

main ()
{
	CLKCON = 0;
	/* Set P2_0 to output */
	P2DIR = 0x01;
	P1INP = 0x00;
	P2INP = 0x00;
	for (;;) {
		high();
/*		low();		*/
	}
}
