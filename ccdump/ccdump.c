/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include <stdio.h>
#include <stdlib.h>
#include "cc-usb.h"

#define NUM_BLOCK	512

int
main (int argc, char **argv)
{
	struct cc_usb	*cc;
	int		block;
	uint8_t		bytes[32 * (2 + 8)];
	uint8_t		*b;
	int		i, j;
	uint32_t	addr;

	cc = cc_usb_open();
	for (block = 0; block < NUM_BLOCK; block++) {
		cc_queue_read(cc, bytes, sizeof (bytes));
		cc_usb_printf(cc, "e %x\n", block);
		cc_usb_sync(cc);
		for (i = 0; i < 32; i++) {
			b = bytes + (i * 10);
			addr = block * 256 + i * 8;
			printf ("%06x", addr);
			for (j = 0; j < 8; j++) {
				printf (" %02x", b[j+2]);
			}
			printf ("\n");
		}
	}
	cc_usb_close(cc);
	exit (0);
}
