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

uint8_t
ccdbg_set_rom(struct ccdbg *dbg, struct ao_hex_image *rom)
{
	if (dbg->rom)
		ao_hex_image_free(dbg->rom);
	dbg->rom = rom;
	return 0;
}

uint8_t
ccdbg_rom_contains(struct ccdbg *dbg, uint16_t addr, int nbytes)
{
	struct ao_hex_image *rom = dbg->rom;
	if (!rom)
		return 0;
	if (addr < rom->address || rom->address + rom->length < addr + nbytes)
		return 0;
	return 1;
}

uint8_t
ccdbg_rom_replace_xmem(struct ccdbg *dbg,
		       uint16_t addr, uint8_t *bytes, int nbytes)
{
	struct ao_hex_image *rom = dbg->rom;
	if (!rom)
		return 0;

	if (rom->address < addr + nbytes && addr < rom->address + rom->length) {
		int	start, stop;

		start = addr;
		if (addr < rom->address)
			start = rom->address;
		stop = addr + nbytes;
		if (rom->address + rom->length < stop)
			stop = rom->address + rom->length;
		memcpy(bytes + start - addr, rom->data + start - rom->address,
		       stop - start);
		return 1;
	}
	return 0;
}
