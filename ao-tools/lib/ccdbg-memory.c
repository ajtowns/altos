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

static uint8_t	memory_init[] = {
	3,	MOV_DPTR_data16,	0,	0,
#define HIGH_START	2
#define LOW_START	3
	0,
};


static uint8_t write8[] = {
	2,	MOV_A_data,	0,
#define DATA_BYTE	2
	1,	MOVX_atDPTR_A,
	1,	INC_DPTR,
	0
};

static uint8_t read8[] = {
	1,	MOVX_A_atDPTR,
	1,	INC_DPTR,
	0,
};

uint8_t
ccdbg_write_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes)
{
	int i, nl = 0;
	struct ccstate state;

	if (dbg->usb)
		return cc_usb_write_memory(dbg->usb, addr, bytes, nbytes);
	ccdbg_state_save(dbg, &state, CC_STATE_ACC | CC_STATE_PSW | CC_STATE_DP);
	memory_init[HIGH_START] = addr >> 8;
	memory_init[LOW_START] = addr;
	(void) ccdbg_execute(dbg, memory_init);
	for (i = 0; i < nbytes; i++) {
		write8[DATA_BYTE] = *bytes++;
		ccdbg_execute(dbg, write8);
		if ((i & 0xf) == 0xf) {
			ccdbg_debug(CC_DEBUG_MEMORY, ".");
			ccdbg_flush(CC_DEBUG_MEMORY);
			nl = 1;
		}
		if ((i & 0xff) == 0xff) {
			ccdbg_debug(CC_DEBUG_MEMORY, "\n");
			nl = 0;
		}
	}
	ccdbg_state_restore(dbg, &state);
	if (nl)
		ccdbg_debug(CC_DEBUG_MEMORY, "\n");
	return 0;
}

uint8_t
ccdbg_read_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes)
{
	int i, nl = 0;
	struct ccstate state;

	if (ccdbg_rom_contains(dbg, addr, nbytes)) {
		ccdbg_rom_replace_xmem(dbg, addr, bytes, nbytes);
		return 0;
	}
	if (dbg->usb)
		return cc_usb_read_memory(dbg->usb, addr, bytes, nbytes);
	ccdbg_state_save(dbg, &state, CC_STATE_ACC | CC_STATE_PSW | CC_STATE_DP);
	memory_init[HIGH_START] = addr >> 8;
	memory_init[LOW_START] = addr;
	(void) ccdbg_execute(dbg, memory_init);
	for (i = 0; i < nbytes; i++) {
		*bytes++ = ccdbg_execute(dbg, read8);
		if ((i & 0xf) == 0xf) {
			ccdbg_debug(CC_DEBUG_MEMORY, ".");
			ccdbg_flush(CC_DEBUG_MEMORY);
			nl = 1;
		}
		if ((i & 0xff) == 0xff) {
			ccdbg_debug(CC_DEBUG_MEMORY, "\n");
			nl = 0;
		}
	}
	ccdbg_state_replace_xmem(dbg, &state, addr, bytes, nbytes);
	ccdbg_state_restore(dbg, &state);
	if (nl)
		ccdbg_debug(CC_DEBUG_MEMORY, "\n");
	return 0;
}

uint8_t
ccdbg_write_uint8(struct ccdbg *dbg, uint16_t addr, uint8_t byte)
{
	return ccdbg_write_memory(dbg, addr, &byte, 1);
}

uint8_t
ccdbg_write_hex_image(struct ccdbg *dbg, struct ao_hex_image *image, uint16_t offset)
{
	ccdbg_write_memory(dbg, image->address + offset, image->data, image->length);
	return 0;
}

struct ao_hex_image *
ccdbg_read_hex_image(struct ccdbg *dbg, uint16_t address, uint16_t length)
{
	struct ao_hex_image *image;

	image = calloc(sizeof(struct ao_hex_image) + length, 1);
	image->address = address;
	image->length = length;
	memset(image->data, 0xff, length);
	ccdbg_read_memory(dbg, address, image->data, length);
	return image;
}

static uint8_t sfr_read[] = {
	2,	MOV_A_direct,		0,
#define SFR_READ_ADDR	2
	0,
};

static uint8_t sfr_write[] = {
	3,	MOV_direct_data,	0,	0,
#define SFR_WRITE_ADDR	2
#define SFR_WRITE_DATA	3
	0,
};

uint8_t
ccdbg_read_sfr(struct ccdbg *dbg, uint8_t addr, uint8_t *bytes, int nbytes)
{
	int	i;
	struct ccstate state;

	ccdbg_state_save(dbg, &state, CC_STATE_ACC);
	for (i = 0; i < nbytes; i++) {
		sfr_read[SFR_READ_ADDR] = addr + i;
		*bytes++ = ccdbg_execute(dbg, sfr_read);
	}
	ccdbg_state_replace_sfr(dbg, &state, addr, bytes, nbytes);
	ccdbg_state_restore(dbg, &state);
	return 0;
}

uint8_t
ccdbg_write_sfr(struct ccdbg *dbg, uint8_t addr, uint8_t *bytes, int nbytes)
{
	int	i;

	for (i = 0; i < nbytes; i++) {
		sfr_write[SFR_WRITE_ADDR] = addr + i;
		sfr_write[SFR_WRITE_DATA] = *bytes++;
		ccdbg_execute(dbg, sfr_write);
	}
	return 0;
}
