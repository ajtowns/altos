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
	ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_RESET_N|CC_CLOCK, 0); ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_CLOCK, CC_CLOCK);	 ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_CLOCK, 0);		 ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_CLOCK, CC_CLOCK);     ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_RESET_N, CC_RESET_N); ccdbg_half_clock(dbg);
}

void
ccdbg_reset(struct ccdbg *dbg)
{
	ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_RESET_N, 0);		ccdbg_half_clock(dbg);
	cccp_write(dbg, CC_RESET_N, CC_RESET_N);
}

uint8_t
ccdbg_read_status(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_READ_STATUS, NULL, 0);
}

uint8_t
ccdbg_rd_config(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read8(dbg, CC_RD_CONFIG, NULL, 0);
}

uint16_t
ccdbg_get_chip_id(struct ccdbg *dbg)
{
	return ccdbg_cmd_write_read16(dbg, CC_GET_CHIP_ID, NULL, 0);
}

