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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "ccdbg-debug.h"
#include "cc-bitbang.h"

#define CP_USB_ASYNC

#ifdef CP_USB_ASYNC
#include "cp-usb-async.h"
#else
#include "cp-usb.h"
#endif

struct cc_bitbang {
#ifdef CP_USB_ASYNC
	struct cp_usb_async *cp_async;
#else
	struct cp_usb *cp;
#endif
};

static uint32_t	cc_clock_us = CC_CLOCK_US;
static uint32_t	cc_reset_us = CC_RESET_US;

void
cc_bitbang_set_clock(uint32_t us)
{
	cc_clock_us = us;
}

void
cc_bitbang_half_clock(struct cc_bitbang *bb)
{
	struct timespec	req, rem;
	req.tv_sec = (cc_clock_us / 2) / 1000000;
	req.tv_nsec = ((cc_clock_us / 2) % 1000000) * 1000;
	nanosleep(&req, &rem);
}

void
cc_bitbang_wait_reset(struct cc_bitbang *bb)
{
	struct timespec	req, rem;

	cc_bitbang_sync(bb);
	req.tv_sec = (cc_reset_us) / 1000000;
	req.tv_nsec = ((cc_reset_us) % 1000000) * 1000;
	nanosleep(&req, &rem);
}

struct cc_bitbang *
cc_bitbang_open(void)
{
	struct cc_bitbang *bb;

	bb = calloc(sizeof (struct cc_bitbang), 1);
	if (!bb) {
		perror("calloc");
		return NULL;
	}
#ifdef CP_USB_ASYNC
	bb->cp_async = cp_usb_async_open();
	if (!bb->cp_async) {
		free (bb);
		return NULL;
	}
#else
	bb->cp = cp_usb_open ();
	if (!bb->cp) {
		free (bb);
		return NULL;
	}
#endif
	return bb;
}

void
cc_bitbang_close(struct cc_bitbang *bb)
{
#ifdef CP_USB_ASYNC
	cp_usb_async_close(bb->cp_async);
#else
	cp_usb_close(bb->cp);
#endif
	free (bb);
}

void
cc_bitbang_debug_mode(struct cc_bitbang *bb)
{
	/* force two rising clocks while holding RESET_N low */
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "# Debug mode\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA           );
	cc_bitbang_wait_reset(bb);
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA|CC_RESET_N);
	cc_bitbang_wait_reset(bb);
}

void
cc_bitbang_reset(struct cc_bitbang *bb)
{
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "# Reset\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_wait_reset(bb);
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	cc_bitbang_wait_reset(bb);
}

int
cc_bitbang_write(struct cc_bitbang *bb, uint8_t mask, uint8_t value)
{
#ifdef CP_USB_ASYNC
	cp_usb_async_write(bb->cp_async, mask, value);
#else
	cp_usb_write(bb->cp, mask, value);
#endif
	return 0;
}

void
cc_bitbang_read(struct cc_bitbang *bb, uint8_t *valuep)
{
#ifdef CP_USB_ASYNC
	cp_usb_async_read(bb->cp_async, valuep);
#else
	*valuep = cp_usb_read(bb->cp);
#endif
}

void
cc_bitbang_sync(struct cc_bitbang *bb)
{
#ifdef CP_USB_ASYNC
	cp_usb_async_sync(bb->cp_async);
#endif
}

static char
is_bit(uint8_t get, uint8_t mask, char on, uint8_t bit)
{
	if (mask&bit) {
		if (get&bit)
			return on;
		else
			return '.';
	} else
		return '-';
}

void
cc_bitbang_print(char *format, uint8_t mask, uint8_t set)
{
	ccdbg_debug (CC_DEBUG_BITBANG, format,
		     is_bit(set, mask, 'C', CC_CLOCK),
		     is_bit(set, mask, 'D', CC_DATA),
		     is_bit(set, mask, 'R', CC_RESET_N));
}

void
cc_bitbang_send(struct cc_bitbang *bb, uint8_t mask, uint8_t set)
{
	cc_bitbang_write(bb, mask, set);
	cc_bitbang_print("%c %c %c\n", mask, set);
	cc_bitbang_half_clock(bb);
}

void
cc_bitbang_send_bit(struct cc_bitbang *bb, uint8_t bit)
{
	if (bit) bit = CC_DATA;
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|bit|CC_RESET_N);
	cc_bitbang_send(bb, CC_CLOCK|CC_DATA|CC_RESET_N,          bit|CC_RESET_N);
}

void
cc_bitbang_send_byte(struct cc_bitbang *bb, uint8_t byte)
{
	int bit;
	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Send Byte 0x%02x\n#\n", byte);
	for (bit = 7; bit >= 0; bit--) {
		cc_bitbang_send_bit(bb, (byte >> bit) & 1);
		if (bit == 3)
			ccdbg_debug(CC_DEBUG_BITBANG, "\n");
	}
	cc_bitbang_sync(bb);
}

void
cc_bitbang_send_bytes(struct cc_bitbang *bb, uint8_t *bytes, int nbytes)
{
	while (nbytes--)
		cc_bitbang_send_byte(bb, *bytes++);
}

void
cc_bitbang_recv_bit(struct cc_bitbang *bb, int first, uint8_t *bit)
{
	uint8_t mask = first ? CC_DATA : 0;

	cc_bitbang_send(bb, CC_CLOCK|mask|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	cc_bitbang_read(bb, bit);
	cc_bitbang_send(bb, CC_CLOCK|     CC_RESET_N,                  CC_RESET_N);
}

void
cc_bitbang_recv_byte(struct cc_bitbang *bb, int first, uint8_t *bytep)
{
	uint8_t byte = 0;
	uint8_t bits[8];
	int	bit;

	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Recv byte\n#\n");
	for (bit = 0; bit < 8; bit++) {
		cc_bitbang_recv_bit(bb, first, &bits[bit]);
		first = 0;
	}
	cc_bitbang_sync(bb);
	for (bit = 0; bit < 8; bit++) {
		byte = byte << 1;
		byte |= (bits[bit] & CC_DATA) ? 1 : 0;
		cc_bitbang_print("#\t%c %c %c\n", CC_DATA, bits[bit]);
		if (bit == 3)
			ccdbg_debug(CC_DEBUG_BITBANG, "\n");
	}
	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Recv 0x%02x\n#\n", byte);
	*bytep = byte;
}

void
cc_bitbang_recv_bytes(struct cc_bitbang *bb, uint8_t *bytes, int nbytes)
{
	int i;
	int first = 1;
	for (i = 0; i < nbytes; i++) {
		cc_bitbang_recv_byte(bb, first, &bytes[i]);
		first = 0;
	}
}
