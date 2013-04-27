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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <string.h>
#include "cc.h"
#include "cc-usb.h"
#include "ccdbg.h"
#include "ao-stmload.h"

int	ao_self_verbose;

#define TRACE(...) if (ao_self_verbose) printf (__VA_ARGS__)

void
ao_self_block_read(struct cc_usb *cc, uint32_t address, uint8_t block[256])
{
	int			byte;
	cc_usb_sync(cc);
	cc_usb_printf(cc, "R %x\n", address);
	for (byte = 0; byte < 0x100; byte++) {
		block[byte] = cc_usb_getchar(cc);
	}
	TRACE ("\nread %08x\n", address);
	for (byte = 0; byte < 0x100; byte++) {
		TRACE (" %02x", block[byte]);
		if ((byte & 0xf) == 0xf)
			TRACE ("\n");
	}
}

void
ao_self_block_write(struct cc_usb *cc, uint32_t address, uint8_t block[256])
{
	int			byte;
	cc_usb_sync(cc);
	cc_usb_printf(cc, "W %x\n", address);
	TRACE ("write %08x\n", address);
	for (byte = 0; byte < 0x100; byte++) {
		TRACE (" %02x", block[byte]);
		if ((byte & 0xf) == 0xf)
			TRACE ("\n");
	}
	for (byte = 0; byte < 0x100; byte++) {
		cc_usb_printf(cc, "%c", block[byte]);
	}
}

struct hex_image *
ao_self_read(struct cc_usb *cc, uint32_t address, uint32_t length)
{
	struct hex_image	*image;
	int			pages;
	int			page;
	uint32_t		base = address & ~0xff;
	uint32_t		bound = (address + length + 0xff) & ~0xff;

	image = calloc(sizeof (struct hex_image) + (bound - base), 1);
	image->address = base;
	image->length = bound - base;
	pages = image->length / 0x100;
	for (page = 0; page < pages; page++)
		ao_self_block_read(cc, image->address + page * 0x100, image->data + page * 0x100);
	return image;
}

int
ao_self_write(struct cc_usb *cc, struct hex_image *image)
{
	uint8_t		block[256];
	uint8_t		check[256];
	uint32_t	base, bound, length, address;
	uint32_t	pages;
	uint32_t	page;

	base = image->address & ~0xff;
	bound = (image->address + image->length + 0xff) & ~0xff;

	address = base;
	length = bound - base;

	pages = length / 0x100;
	printf ("Write %08x %d pages: ", address, length/0x100); fflush(stdout);
	for (page = 0; page < pages; page++) {
		uint32_t	start, stop;
		address = base + page * 0x100;

		if (address < image->address || address + 0x100 > image->address + image->length) {
			ao_self_block_read(cc, address, block);
		}
		start = address;
		stop = address + 0x100;
		if (start < image->address)
			start = image->address;
		if (stop > image->address + image->length)
			stop = image->address + image->length;
		memcpy(block + start - address, image->data + start - image->address, stop - start);
		ao_self_block_write(cc, address, block);
		ao_self_block_read(cc, address, check);
		if (memcmp(block, check, 0x100) != 0) {
			fprintf(stderr, "Block at 0x%08x doesn't match\n", address);
			return 0;
		}
		putchar('.'); fflush(stdout);
	}
	printf("done\n");
	cc_usb_printf(cc,"a\n");
	return 1;
}
