/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "radio.h"

main ()
{
	int16_t	j;
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;
	P1 = 0;
	P1DIR = 3;
	radio_init ();
	delay(100);

	for (;;) {
		uint8_t	i;

		for (j = 0; j < 100; j++)
			delay(100);
		P1 = 2;
		RFST = RFST_SIDLE;
		delay(1);
		RFST = RFST_STX;
		for (i = 0; i < PACKET_LEN; i++) {
			while (!RFTXRXIF);
			RFTXRXIF = 0;
			RFD = i;
		}
		P1 = 0;
	}
}
