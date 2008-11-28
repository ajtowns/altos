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

void
ccdbg_quarter_clock(struct ccdbg *dbg)
{
	usleep(CC_CLOCK_US / 4);
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
	dbg->fd = open(file, 2);
	if (dbg->fd < 0) {
		perror(file);
		free(dbg);
		return NULL;
	}
	cccp_init(dbg);
	return dbg;
}

void
ccdbg_close(struct ccdbg *dbg)
{
	cccp_fini(dbg);
	close (dbg->fd);
	free (dbg);
}

void
ccdbg_clock_1_0(struct ccdbg *dbg)
{
	ccdbg_quarter_clock(dbg);
	assert(dbg->clock == 1);
	cccp_write(dbg, CC_CLOCK, 0);
	dbg->clock = 0;
	ccdbg_quarter_clock(dbg);
}

void
ccdbg_clock_0_1(struct ccdbg *dbg)
{
	ccdbg_quarter_clock(dbg);
	assert(dbg->clock == 0);
	cccp_write(dbg, CC_CLOCK, CC_CLOCK);
	dbg->clock = 1;
	ccdbg_quarter_clock(dbg);
}

/*
 * By convention, every macro function is entered with dbg->clock == 1
 */

void
ccdbg_write_bit(struct ccdbg *dbg, uint8_t bit)
{
	ccdbg_clock_1_0(dbg);
	cccp_write(dbg, CC_DATA, bit ? CC_DATA : 0);
	ccdbg_clock_0_1(dbg);
}

void
ccdbg_write_byte(struct ccdbg *dbg, uint8_t byte)
{
	int	bit;

	for (bit = 7; bit >= 0; bit--)
		ccdbg_write_bit(dbg, (byte >> bit) & 1);
}

uint8_t
ccdbg_read_bit(struct ccdbg *dbg)
{
	uint8_t	data;

	ccdbg_clock_1_0(dbg);
	data = cccp_read(dbg, CC_DATA);
	ccdbg_clock_0_1(dbg);
	return data ? 1 : 0;
}

uint8_t
ccdbg_read_byte(struct ccdbg *dbg)
{
	int	bit;
	uint8_t	byte = 0;

	for (bit = 7; bit >= 0; bit--)
		byte |= ccdbg_read_bit(dbg) << bit;
	return byte;
}

void
ccdbg_cmd_write(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	int	i;
	ccdbg_write_byte(dbg, cmd);
	for (i = 0; i < len; i++)
		ccdbg_write_byte(dbg, data[i]);
}

uint8_t
ccdbg_cmd_write_read8(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	ccdbg_cmd_write(dbg, cmd, data, len);
	return ccdbg_read_byte(dbg);
}

uint16_t
ccdbg_cmd_write_read16(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len)
{
	uint8_t	byte1, byte0;
	ccdbg_cmd_write(dbg, cmd, data, len);
	byte1 = ccdbg_read_byte(dbg); 
	byte0 = ccdbg_read_byte(dbg);
	return (byte1 << 8) | byte0;
}

