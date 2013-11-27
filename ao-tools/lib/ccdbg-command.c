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

uint16_t
ccdbg_get_pc(struct ccdbg *dbg)
{
	uint16_t	pc1, pc2;

	pc1 = ccdbg_cmd_write_read16(dbg, CC_GET_PC, NULL, 0);
	pc2 = ccdbg_cmd_write_read16(dbg, CC_GET_PC, NULL, 0);
	if (pc1 != pc2)
		fprintf (stderr, "Invalid pc %04x != %04x\n",
			 pc1, pc2);
	return pc2;
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

void
ccdbg_debug_instr_discard(struct ccdbg *dbg, uint8_t *instr, int nbytes)
{
	static uint8_t	discard;
	ccdbg_cmd_write_queue8(dbg, CC_DEBUG_INSTR(nbytes),
			       instr, nbytes, &discard);
}

void
ccdbg_debug_instr_queue(struct ccdbg *dbg, uint8_t *instr, int nbytes,
			uint8_t *reply)
{
	return ccdbg_cmd_write_queue8(dbg, CC_DEBUG_INSTR(nbytes),
				      instr, nbytes, reply);
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

/*
 * Execute a sequence of instructions
 */
uint8_t
ccdbg_execute(struct ccdbg *dbg, uint8_t *inst)
{
	uint8_t status = 0;
	while(inst[0] != 0) {
		uint8_t	len = inst[0];
		int i;
		ccdbg_debug(CC_DEBUG_INSTRUCTIONS, "\t%02x", inst[1]);
		for (i = 0; i < len - 1; i++)
			ccdbg_debug(CC_DEBUG_INSTRUCTIONS, " %02x", inst[i+2]);
		ccdbg_debug_instr_queue(dbg, inst+1, len, &status);
		for (; i < 3; i++)
			ccdbg_debug(CC_DEBUG_INSTRUCTIONS, "   ");
		ccdbg_debug(CC_DEBUG_INSTRUCTIONS, " -> %02x\n", status);
		inst += len + 1;
	}
	ccdbg_sync(dbg);
	return status;
}

static uint8_t jump_mem[] = {
	3, LJMP, 0xf0, 0x00,
#define PC_HIGH	2
#define PC_LOW	3
	0
};

uint8_t
ccdbg_set_pc(struct ccdbg *dbg, uint16_t pc)
{
	jump_mem[PC_HIGH] = pc >> 8;
	jump_mem[PC_LOW] = pc & 0xff;
	return ccdbg_execute(dbg, jump_mem);
}

uint8_t
ccdbg_execute_hex_image(struct ccdbg *dbg, struct ao_hex_image *image)
{
	uint16_t pc;
	uint8_t status;

	if (image->address < 0xf000) {
		fprintf(stderr, "Cannot execute program starting at 0x%04x\n", image->address);
		return -1;
	}
	ccdbg_write_hex_image(dbg, image, 0);
	ccdbg_set_pc(dbg, image->address);
	pc = ccdbg_get_pc(dbg);
	ccdbg_debug(CC_DEBUG_EXECUTE, "pc starts at 0x%04x\n", pc);
	status = ccdbg_resume(dbg);
	ccdbg_debug(CC_DEBUG_EXECUTE, "resume status: 0x%02x\n", status);
	return 0;
}
