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

#include "radio.h"

main ()
{
	static uint8_t	packet[PACKET_LEN + 2];
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;
	/* Set P2_0 to output */
	P1 = 0;
	P1DIR = 0x02;
	radio_init ();
	delay(100);

	for (;;) {
		uint8_t	i;
		RFST = RFST_SIDLE;
		RFIF = 0;
		delay(100);
		RFST = RFST_SRX;
//		while (!(RFIF & RFIF_IM_CS));
//		P1 = 2;
		for (i = 0; i < PACKET_LEN + 2; i++) {
			while (!RFTXRXIF)
				;
			P1=2;
			RFTXRXIF = 0;
			packet[i] = RFD;
		}
		P1 = 0;

		/* check packet contents */
		for (i = 0; i < PACKET_LEN; i++)
			if (packet[i] != i)
				break;

		/* get excited if the packet came through correctly */
		if (i == PACKET_LEN &&
		    packet[PACKET_LEN+1] & PKT_APPEND_STATUS_1_CRC_OK)
		{
			for (i = 0; i < 5; i++){
				P1 = 2;
				delay(100);
				P1 = 0;
				delay(100);
			}
		}
		delay(100);
	}
}
