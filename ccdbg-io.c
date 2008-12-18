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

#include "ccdbg.h"
#include <time.h>

void
ccdbg_half_clock(struct ccdbg *dbg)
{
	struct timespec	req, rem;
	req.tv_sec = (CC_CLOCK_US / 2) / 1000000;
	req.tv_nsec = ((CC_CLOCK_US / 2) % 1000000) * 1000;
//	nanosleep(&req, &rem);
}

struct ccdbg *
ccdbg_open(char *file)
{
	struct ccdbg *dbg;

	dbg = calloc(sizeof (struct ccdbg), 1);
	if (!dbg) {
		perror("calloc");
		return NULL;
	}
	dbg->clock = 1;
#ifdef USE_KERNEL
	dbg->fd = open(file, 2);
	if (dbg->fd < 0) {
		perror(file);
		free(dbg);
		return NULL;
	}
	cccp_init(dbg);
	cccp_write(dbg, CC_CLOCK, CC_CLOCK);
#else
	cp_usb_init(dbg);
#endif
	dbg->clock = 1;
	return dbg;
}

void
ccdbg_close(struct ccdbg *dbg)
{
#if USE_KERNEL
	cccp_fini(dbg);
	close (dbg->fd);
#else
	cp_usb_fini(dbg);
#endif
	free (dbg);
}

int
ccdbg_write(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
#if USE_KERNEL
	return cccp_write(dbg, mask, value);
#else
	cp_usb_write(dbg, mask, value);
	return 0;
#endif
}

uint8_t
ccdbg_read(struct ccdbg *dbg)
{
#if USE_KERNEL
	return cccp_read_all(dbg);
#else
	return cp_usb_read(dbg);
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
ccdbg_print(char *format, uint8_t mask, uint8_t set)
{
	ccdbg_debug (CC_DEBUG_BITBANG, format,
		     is_bit(set, mask, 'C', CC_CLOCK),
		     is_bit(set, mask, 'D', CC_DATA),
		     is_bit(set, mask, 'R', CC_RESET_N));
}

void
ccdbg_send(struct ccdbg *dbg, uint8_t mask, uint8_t set)
{
	ccdbg_write(dbg, mask, set);
	ccdbg_print("%c %c %c\n", mask, set);
	ccdbg_half_clock(dbg);
}

void
ccdbg_send_bit(struct ccdbg *dbg, uint8_t bit)
{
	if (bit) bit = CC_DATA;
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|bit|CC_RESET_N);
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N,          bit|CC_RESET_N);
}

void
ccdbg_send_byte(struct ccdbg *dbg, uint8_t byte)
{
	int bit;
	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Send Byte 0x%02x\n#\n", byte);
	for (bit = 7; bit >= 0; bit--) {
		ccdbg_send_bit(dbg, (byte >> bit) & 1);
		if (bit == 3)
			ccdbg_debug(CC_DEBUG_BITBANG, "\n");
	}
}

void
ccdbg_send_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes)
{
	while (nbytes--)
		ccdbg_send_byte(dbg, *bytes++);
}

uint8_t
ccdbg_recv_bit(struct ccdbg *dbg, int first)
{
	uint8_t mask = first ? CC_DATA : 0;
	uint8_t read;

	ccdbg_send(dbg, CC_CLOCK|mask|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	read = ccdbg_read(dbg);
	ccdbg_send(dbg, CC_CLOCK|     CC_RESET_N,                  CC_RESET_N);
	return (read & CC_DATA) ? 1 : 0;
}

uint8_t
ccdbg_recv_byte(struct ccdbg *dbg, int first)
{
	uint8_t byte = 0;
	int	bit;

	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Recv byte\n#\n");
	for (bit = 0; bit < 8; bit++) {
		byte = byte << 1;
		byte |= ccdbg_recv_bit(dbg, first);
		if (bit == 3)
			ccdbg_debug(CC_DEBUG_BITBANG, "\n");
		first = 0;
	}
	ccdbg_debug(CC_DEBUG_BITBANG, "#\n# Recv 0x%02x\n#\n", byte);
	return byte;
}

void
ccdbg_recv_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes)
{
	int first = 1;
	while (nbytes--) {
		*bytes++ = ccdbg_recv_byte(dbg, first);
		first = 0;
	}
}

void
ccdbg_cmd_write(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	int	i;
	ccdbg_send_byte(dbg, cmd);
	for (i = 0; i < len; i++)
		ccdbg_send_byte(dbg, data[i]);
}

uint8_t
ccdbg_cmd_write_read8(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	uint8_t	byte[1];
	ccdbg_cmd_write(dbg, cmd, data, len);
	ccdbg_recv_bytes(dbg, byte, 1);
	return byte[0];
}

uint16_t
ccdbg_cmd_write_read16(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	uint8_t	byte[2];
	ccdbg_cmd_write(dbg, cmd, data, len);
	ccdbg_recv_bytes(dbg, byte, 2);
	return (byte[0] << 8) | byte[1];
}
