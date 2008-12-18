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

#define MOV_direct_data		0x75
#define LJMP			0x02
#define MOV_Rn_data(n)		(0x78 | (n))
#define DJNZ_Rn_rel(n)		(0xd8 | (n))

#if 0
static uint8_t instructions[] = {
	3, MOV_direct_data, 0xfe, 0x02,
	3, MOV_direct_data, 0x90, 0xff,
	0
};
#endif

static uint8_t mem_instr[] = {
	MOV_direct_data, 0xfe, 0x02,
	MOV_direct_data, 0x90, 0xff,
	MOV_Rn_data(2), 0x10,
	MOV_Rn_data(0), 0xff,
	MOV_Rn_data(1), 0xff,
	DJNZ_Rn_rel(1), 0xfe,
	DJNZ_Rn_rel(0), 0xfa,
	DJNZ_Rn_rel(2), 0xf6,
	MOV_direct_data, 0x90, 0xfd,
	MOV_Rn_data(2), 0x10,
	MOV_Rn_data(0), 0xff,
	MOV_Rn_data(1), 0xff,
	DJNZ_Rn_rel(1), 0xfe,
	DJNZ_Rn_rel(0), 0xfa,
	DJNZ_Rn_rel(2), 0xf6,
	LJMP, 0xf0, 0x03
};

static uint8_t jump_mem[] = {
	3, LJMP, 0xf0, 0x00,
	0
};

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	chip_id;
	uint16_t	pc;
	uint8_t		memory[0x10];
	int		i;
	struct hex_file	*hex;

	dbg = ccdbg_open("/dev/ttyUSB0");
	if (!dbg)
		exit (1);
#if 0
	ccdbg_manual(dbg, stdin);
#endif
	hex = ccdbg_hex_file_read(stdin, "<stdin>");
	if (!hex)
		exit (1);
	ccdbg_reset(dbg);
	ccdbg_debug_mode(dbg);
	status = ccdbg_read_status(dbg);
	printf("Status: 0x%02x\n", status);
	chip_id = ccdbg_get_chip_id(dbg);
	printf("Chip id: 0x%04x\n", chip_id);
	status = ccdbg_halt(dbg);
	printf ("halt status: 0x%02x\n", status);
	
	ccdbg_write_hex(dbg, hex);
	ccdbg_hex_file_free(hex);
	for (i = 0; i < sizeof (memory); i++)
		printf (" %02x", memory[i]);
	printf ("\n");
	ccdbg_execute(dbg, jump_mem);
	pc = ccdbg_get_pc(dbg);
	printf ("pc starts at 0x%04x\n", pc);
	status = ccdbg_resume(dbg);
	printf ("resume status: 0x%02x\n", status);
#if 0
/*	ccdbg_execute(dbg, instructions); */
	ccdbg_write_memory(dbg, 0xf000, mem_instr, sizeof (mem_instr));
	ccdbg_read_memory(dbg, 0xf000, memory, sizeof (memory));
#endif
	ccdbg_close(dbg);
	exit (0);
}
