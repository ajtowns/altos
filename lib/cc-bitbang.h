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

#ifndef _CC_BITBANG_H_
#define _CC_BITBANG_H_

#include <stdint.h>

#define CC_CLOCK	0x1
#define CC_DATA		0x2
#define CC_RESET_N	0x4
#define CC_CLOCK_US	(2)

/* Telemetrum has a 10k pull-up to 3.3v, a 0.001uF cap to ground
 * and a 2.7k resistor to the reset line. This takes about 6us
 * to settle, so we'll wait longer than that after changing the reset line
 */
#define CC_RESET_US	(12)

struct cc_bitbang;

void
cc_bitbang_set_clock(uint32_t us);

void
cc_bitbang_half_clock(struct cc_bitbang *bb);

void
cc_bitbang_wait_reset(struct cc_bitbang *bb);

struct cc_bitbang *
cc_bitbang_open(void);

void
cc_bitbang_close(struct cc_bitbang *bb);

void
cc_bitbang_debug_mode(struct cc_bitbang *bb);

void
cc_bitbang_reset(struct cc_bitbang *bb);

int
cc_bitbang_write(struct cc_bitbang *bb, uint8_t mask, uint8_t value);

void
cc_bitbang_read(struct cc_bitbang *bb, uint8_t *valuep);

void
cc_bitbang_sync(struct cc_bitbang *bb);

void
cc_bitbang_print(char *format, uint8_t mask, uint8_t set);

void
cc_bitbang_print(char *format, uint8_t mask, uint8_t set);

void
cc_bitbang_send(struct cc_bitbang *bb, uint8_t mask, uint8_t set);

void
cc_bitbang_send_bit(struct cc_bitbang *bb, uint8_t bit);

void
cc_bitbang_send_byte(struct cc_bitbang *bb, uint8_t byte);

void
cc_bitbang_send_bytes(struct cc_bitbang *bb, uint8_t *bytes, int nbytes);

void
cc_bitbang_recv_bit(struct cc_bitbang *bb, int first, uint8_t *bit);

void
cc_bitbang_recv_byte(struct cc_bitbang *bb, int first, uint8_t *bytep);

void
cc_bitbang_recv_bytes(struct cc_bitbang *bb, uint8_t *bytes, int nbytes);

#endif /* _CC_BITBANG_H_ */
