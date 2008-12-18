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

#define MOV	0x75

static uint8_t instructions[] = {
	3, MOV, 0xfe, 0x02,
	3, MOV, 0x90, 0xff,
	0
};

static void
ccdbg_instructions(struct ccdbg *dbg, uint8_t *inst)
{
	while(inst[0] != 0) {
		uint8_t	len = inst[0];
		uint8_t status;
		status = ccdbg_debug_instr(dbg, inst+1, len);
		printf ("inst status 0x%02x\n", status);
		inst += len + 1;
	}
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	chip_id;

	dbg = ccdbg_open("/dev/ttyUSB0");
	if (!dbg)
		exit (1);
#if 0
	ccdbg_manual(dbg, stdin);
#endif
#if 1
	ccdbg_debug_mode(dbg);
	status = ccdbg_read_status(dbg);
	printf("Status: 0x%02x\n", status);
	chip_id = ccdbg_get_chip_id(dbg);
	printf("Chip id: 0x%04x\n", chip_id);
	status = ccdbg_halt(dbg);
	printf ("halt status: 0x%02x\n", status);
	ccdbg_instructions(dbg, instructions);
#endif
	ccdbg_close(dbg);
	exit (0);
}
