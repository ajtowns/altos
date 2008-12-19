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

#if 1
static uint8_t instructions[] = {
	3, MOV_direct_data, 0xfe, 0x02,
	3, MOV_direct_data, 0x90, 0xff,
	0
};
#endif

static uint8_t mem_instr[] = {
	MOV_direct_data, 0xfe, 0x02,
	MOV_Rn_data(0), 0x00,
	MOV_Rn_data(1), 0x00,
	MOV_direct_data, 0x90, 0xff,
	MOV_Rn_data(2), 0x10,
	DJNZ_Rn_rel(1), 0xfe,
	DJNZ_Rn_rel(0), 0xfc,
	DJNZ_Rn_rel(2), 0xfa,
	MOV_direct_data, 0x90, 0xfd,
	MOV_Rn_data(2), 0x10,
	DJNZ_Rn_rel(1), 0xfe,
	DJNZ_Rn_rel(0), 0xfc,
	DJNZ_Rn_rel(2), 0xfa,
	SJMP, 0xe7,
};

static struct hex_image *
make_hex_image(uint16_t addr, uint8_t *data, uint16_t length)
{
	struct hex_image	*image;

	image = malloc(sizeof (struct hex_image) + length);
	image->address = addr;
	image->length = length;
	memcpy(image->data, data, length);
	return image;
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	chip_id;
	uint16_t	pc;
	struct hex_file	*hex;
	struct hex_image *image;

	dbg = ccdbg_open("/dev/ttyUSB0");
	if (!dbg)
		exit (1);
#if 0
	ccdbg_manual(dbg, stdin);
#endif
#if 1
	hex = ccdbg_hex_file_read(stdin, "<stdin>");
	if (!hex)
		exit (1);
	image = ccdbg_hex_image_create(hex);
	ccdbg_hex_file_free(hex);
#else
	image = make_hex_image(0xf000, mem_instr, sizeof (mem_instr));
#endif
	
	ccdbg_reset(dbg);
	ccdbg_debug_mode(dbg);
	ccdbg_halt(dbg);
	
#if 1
	if (!image) {
		fprintf(stderr, "image create failed\n");
		exit (1);
	}
	if (image->address == 0xf000) {
		printf("Loading code to execute from RAM\n");
		ccdbg_execute_hex_image(dbg, image);
	} else if (image->address == 0x0000) {
		printf("Loading code to execute from FLASH\n");
		ccdbg_flash_hex_image(dbg, image);
		ccdbg_set_pc(dbg, 0);
		ccdbg_resume(dbg);
	} else {
		printf("Cannot load code to 0x%04x\n",
		       image->address);
		ccdbg_hex_image_free(image);
		ccdbg_close(dbg);
		exit(1);
	}
#endif
	for (;;) {
		pc = ccdbg_get_pc(dbg);
		status = ccdbg_read_status(dbg);
		printf("pc: 0x%04x.  status: 0x%02x\n", pc, status);
	}
#if 0
/*	ccdbg_execute(dbg, instructions); */
	ccdbg_write_memory(dbg, 0xf000, mem_instr, sizeof (mem_instr));
	ccdbg_read_memory(dbg, 0xf000, memory, sizeof (memory));
#endif
	ccdbg_close(dbg);
	exit (0);
}
