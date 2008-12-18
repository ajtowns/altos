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

/*
 * Read and write arbitrary memory through the debug port
 */

#define MOV_dptr_data16		0x90

static uint8_t	memory_init[] = {
	3,	MOV_dptr_data16,	0,	0,
#define HIGH_START	2
#define LOW_START	3
	0,
};

#define MOV_a_data	0x74
#define MOVX_atdptr_a	0xf0
#define MOVX_a_atdptr	0xe0
#define INC_dptr	0xa3

static uint8_t write8[] = {
	2,	MOV_a_data,	0,
#define DATA_BYTE	2
	1,	MOVX_atdptr_a,
	1,	INC_dptr,
	0
};

static uint8_t read8[] = {
	1,	MOVX_a_atdptr,
	1,	INC_dptr,
	0,
};

uint8_t
ccdbg_write_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes)
{
	memory_init[HIGH_START] = addr >> 8;
	memory_init[LOW_START] = addr;
	(void) ccdbg_execute(dbg, memory_init);
	while (nbytes-- > 0) {
		write8[DATA_BYTE] = *bytes++;
		ccdbg_execute(dbg, write8);
	}
	return 0;
}

uint8_t
ccdbg_read_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes)
{
	memory_init[HIGH_START] = addr >> 8;
	memory_init[LOW_START] = addr;
	(void) ccdbg_execute(dbg, memory_init);
	while (nbytes-- > 0)
		*bytes++ = ccdbg_execute(dbg, read8);
	return 0;
}
