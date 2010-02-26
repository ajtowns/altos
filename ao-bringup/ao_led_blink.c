/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#include "ao_bringup.h"

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

main()
{
	ao_init();
	/* Set p1_0 and p1_1 to output */
	P1DIR = 0x03;
	P1INP = 0x00;
	for (;;) {
		P1 = 1;
		delay(5);
		P1 = 2;
		delay(5);
		P1 = 3;
		delay(5);
		P1 = 0;
		delay(5);
	}
}
