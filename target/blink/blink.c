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
	unsigned char i = 0, j = 0;

	n <<= 1;
	while (--n != 0)
		while (--j != 0)
			while (--i != 0)
				nop();
}

void
dit() {
	P1 = 0xff;
	delay(1);
	P1 = 0xfd;
	delay(1);
}

void
dah () {
	P1 = 0xff;
	delay(3);
	P1 = 0xfd;
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
	/* Set p1_1 to output */
	P1DIR = 0x02;
	P1INP = 0x00;
	P2INP = 0x00;
	for (;;) {
		___ _ ___ _ C ___ ___ _ ___ W	/* cq */
		___ _ _ C _ W			/* de */
		___ _ ___ C ___ _ _ C		/* kd */
		___ ___ _ _ _ C	_ _ _ C		/* 7s */
		___ ___ _ ___ C	___ ___ _ W	/* qg */
	}
}
