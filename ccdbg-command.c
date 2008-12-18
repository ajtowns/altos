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
ccdbg_debug_mode(struct ccdbg *dbg)
{
	/* force two rising clocks while holding RESET_N low */
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "# Debug mode\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N,          CC_DATA|CC_RESET_N);
}

void
ccdbg_reset(struct ccdbg *dbg)
{
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "# Reset\n");
	ccdbg_debug(CC_DEBUG_COMMAND, "#\n");
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA           );
	ccdbg_send(dbg, CC_CLOCK|CC_DATA|CC_RESET_N, CC_CLOCK|CC_DATA|CC_RESET_N);
}

uint8_t
ccdbg_chip_erase(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_CHIP_ERASE, NULL, 0);
}

uint8_t
ccdbg_wr_config(struct ccdbg *dbg, uint8_t config)
{
	return ccdbg_cmd_write_read8(dbg, CC_WR_CONFIG, &config, 1);
}

uint8_t
ccdbg_rd_config(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_RD_CONFIG, NULL, 0);
}

uint8_t
ccdbg_get_pc(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read16(dbg, CC_GET_PC, NULL, 0);
}

uint8_t
ccdbg_read_status(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_READ_STATUS, NULL, 0);
}

uint8_t
ccdbg_set_hw_brkpnt(struct ccdbg *dbg, uint8_t number, uint8_t enable, uint16_t addr)
{
	uint8_t	data[3];

	data[0] = (number << 3) | (enable << 2);
	data[1] = (addr >> 8);
	data[2] = addr;
	return ccdbg_cmd_write_read8(dbg, CC_SET_HW_BRKPNT, data, 3);
}

uint8_t
ccdbg_halt(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_HALT, NULL, 0);
}

uint8_t
ccdbg_resume(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_RESUME, NULL, 0);
}

uint8_t
ccdbg_debug_instr(struct ccdbg *dbg, uint8_t *instr, int nbytes)
{
	return ccdbg_cmd_write_read8(dbg, CC_DEBUG_INSTR(nbytes), instr, nbytes);
}

uint8_t
ccdbg_step_instr(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_STEP_INSTR, NULL, 0);
}

uint8_t
ccdbg_step_replace(struct ccdbg *dbg, uint8_t *instr, int nbytes)
{
	return ccdbg_cmd_write_read8(dbg, CC_STEP_REPLACE(nbytes), instr, nbytes);
}

uint16_t
ccdbg_get_chip_id(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read16(dbg, CC_GET_CHIP_ID, NULL, 0);
}
