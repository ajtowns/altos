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

#include "ao-dbg.h"

static uint16_t start_address;

static enum command_result
parse_int(char *value, int *result)
{
	char *endptr;

	*result = strtol(value, &endptr, 0);
	if (endptr == value)
		return command_syntax;
	return command_success;
}

static enum command_result
parse_uint16(char *value, uint16_t *uint16)
{
	int	v;
	enum command_result result;

	result = parse_int(value, &v);
	if (result != command_success)
		return command_error;
	if (v < 0 || v > 0xffff)
		return command_error;
	*uint16 = v;
	return command_success;
}

static enum command_result
parse_uint8(char *value, uint8_t *uint8)
{
	int	v;
	enum command_result result;

	result = parse_int(value, &v);
	if (result != command_success)
		return command_error;
	if (v < 0 || v > 0xff)
		return command_error;
	*uint8 = v;
	return command_success;
}

enum command_result
command_quit (int argc, char **argv)
{
	ccdbg_reset(s51_dbg);
	exit(0);
	return command_error;
}

static void
dump_bytes(uint8_t *memory, int length, uint16_t start, char *format)
{
	int group, i;

	for (group = 0; group < length; group += 8) {
		s51_printf(format, start + group);
		for (i = group; i < length && i < group + 8; i++)
			s51_printf("%02x ", memory[i]);
		for (; i < group + 8; i++)
			s51_printf("   ");
		for (i = group; i < length && i < group + 8; i++) {
			if (isascii(memory[i]) && isprint(memory[i]))
				s51_printf("%c", memory[i]);
			else
				s51_printf(".");
		}
		s51_printf("\n");
	}
}

enum command_result
command_di (int argc, char **argv)
{
	uint16_t start, end;
	uint8_t	memory[65536];
	uint8_t status;
	int length;

	if (argc != 3)
		return command_error;
	if (parse_uint16(argv[1], &start) != command_success)
		return command_error;
	if (parse_uint16(argv[2], &end) != command_success)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_memory(s51_dbg, start + 0xff00, memory, length);
	dump_bytes(memory, length, start, "0x%02x ");
	return command_success;
}

enum command_result
command_ds (int argc, char **argv)
{
	uint8_t start, end;
	uint8_t	memory[0x100];
	uint8_t status;
	int length;

	if (argc != 3)
		return command_error;
	if (parse_uint8(argv[1], &start) != command_success)
		return command_error;
	if (parse_uint8(argv[2], &end) != command_success)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_sfr(s51_dbg, start, memory, length);
	dump_bytes(memory, length, start, "0x%02x ");
	return command_success;
}

enum command_result
command_dx (int argc, char **argv)
{
	uint16_t start, end;
	uint8_t	memory[65536];
	uint8_t status;
	int length;

	if (argc != 3)
		return command_error;
	if (parse_uint16(argv[1], &start) != command_success)
		return command_error;
	if (parse_uint16(argv[2], &end) != command_success)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_memory(s51_dbg, start, memory, length);
	dump_bytes(memory, length, start, "0x%04x ");
	return command_success;
}

enum command_result
command_set (int argc, char **argv)
{
	uint16_t address;
	uint8_t *data;
	int len = argc - 3;
	int i;
	enum command_result ret = command_success;

	if (len < 0)
		return command_error;
	if (parse_uint16(argv[2], &address) != command_success)
		return command_error;
	if (len == 0)
		return command_success;
	data = malloc(len);
	if (!data)
		return command_error;
	for (i = 0; i < len; i++)
		if (parse_uint8(argv[i+3], &data[i]) != command_success)
			return command_error;

	if (strcmp(argv[1], "xram") == 0) {
		ccdbg_write_memory(s51_dbg, address, data, len);
	} else if (strcmp(argv[1], "iram") == 0) {
		ccdbg_write_memory(s51_dbg, address + 0xff00, data, len);
	} else if (strcmp(argv[1], "sfr") == 0) {
		ccdbg_write_sfr(s51_dbg, (uint8_t) address, data, len);
	} else
		ret = command_error;
	free(data);
	return ret;
}

enum command_result
command_dump (int argc, char **argv)
{
	if (argv[1]) {
		if (strcmp(argv[1], "rom") == 0 ||
		    strcmp(argv[1], "xram") == 0)
			return command_dx(argc-1, argv+1);
		if (strcmp(argv[1], "iram") == 0)
			return command_di(argc-1, argv+1);
		if (strcmp(argv[1], "sfr") == 0)
			return command_ds(argc-1, argv+1);
	}
	return command_error;
}

enum command_result
command_file (int argc, char **argv)
{
	struct ao_hex_file *hex;
	struct ao_hex_image *image;
	FILE *file;

	if (argc != 2)
		return command_error;
	file = fopen (argv[1], "r");
	if (!file)
		return command_error;
	hex = ao_hex_file_read(file, argv[1]);
	fclose(file);
	if (!hex)
		return command_error;
	if (hex->nrecord == 0) {
		ao_hex_file_free(hex);
		return command_error;
	}
	image = ao_hex_image_create(hex);
	ao_hex_file_free(hex);
	start_address = image->address;
	ccdbg_set_rom(s51_dbg, image);
	return command_success;
}

enum command_result
command_pc (int argc, char **argv)
{
	uint16_t	pc;
	if (argv[1]) {
		enum command_result result;
		result = parse_uint16(argv[1], &pc);
		if (result != command_success)
			return result;
		ccdbg_set_pc(s51_dbg, pc);
	} else {
		pc = ccdbg_get_pc(s51_dbg);
		s51_printf("   0x%04x 00\n", pc);
	}
	return command_success;
}

struct cc_break {
	int		enabled;
	int		temporary;
	uint16_t	address;
};

#define CC_NUM_BREAKPOINTS 4

static struct cc_break	breakpoints[CC_NUM_BREAKPOINTS];

static void
disable_breakpoint(int b)
{
	uint8_t status;

	status = ccdbg_set_hw_brkpnt(s51_dbg, b, 0, breakpoints[b].address);
	if (status != 0x00 && status != 0xff)
		s51_printf("disable_breakpoint status 0x%02x\n", status);
}

static void
enable_breakpoint(int b)
{
	uint8_t status;

	status = ccdbg_set_hw_brkpnt(s51_dbg, b, 1, breakpoints[b].address);
	if (status != 0xff)
		s51_printf("enable_breakpoint status 0x%02x\n", status);
}

static void
enable_breakpoints(void)
{
	int b;
	for (b = 0; b < CC_NUM_BREAKPOINTS; b++)
		if (breakpoints[b].enabled)
			enable_breakpoint(b);
}

enum command_result
set_breakpoint(uint16_t address, int temporary)
{
	int b;
	uint8_t status;
	for (b = 0; b < CC_NUM_BREAKPOINTS; b++) {
		if (breakpoints[b].enabled == 0)
			break;
		if (breakpoints[b].address == address)
			break;
	}
	if (b == CC_NUM_BREAKPOINTS) {
		s51_printf("Error: too many breakpoints requested\n");
		return command_success;
	}
	if (breakpoints[b].enabled == 0) {
		breakpoints[b].address = address;
		enable_breakpoint(b);
	}
	++breakpoints[b].enabled;
	s51_printf("Breakpoint %d at 0x%04x\n", b, address);
	breakpoints[b].temporary += temporary;
	return command_success;
}

enum command_result
clear_breakpoint(uint16_t address, int temporary)
{
	int b;
	uint8_t status;

	for (b = 0; b < CC_NUM_BREAKPOINTS; b++) {
		if (breakpoints[b].enabled != 0 &&
		    ((breakpoints[b].temporary != 0) == (temporary != 0)) &&
		    breakpoints[b].address == address)
			break;
	}
	if (b == CC_NUM_BREAKPOINTS) {
		s51_printf("Error: no matching breakpoint found\n");
		return command_success;
	}
	--breakpoints[b].enabled;
	breakpoints[b].temporary -= temporary;
	if (breakpoints[b].enabled == 0) {
		disable_breakpoint(b);
		breakpoints[b].address = -1;
	}
	return command_success;
}


int
find_breakpoint(uint16_t address)
{
	int b;

	for (b = 0; b < CC_NUM_BREAKPOINTS; b++)
		if (breakpoints[b].enabled && breakpoints[b].address == address)
			break;
	if (b == CC_NUM_BREAKPOINTS)
		return -1;
	return b;
}

enum command_result
command_break (int argc, char **argv)
{
	int b;
	uint16_t address;
	enum command_result result;

	if (argc == 1) {
		for (b = 0; b < CC_NUM_BREAKPOINTS; b++)
			if (breakpoints[b].enabled)
				s51_printf("Breakpoint %d 0x%04x\n",
					b, breakpoints[b].address);
		return command_success;
	}
	if (argc != 2)
		return command_error;
	result = parse_uint16(argv[1], &address);
	if (result != command_success)
		return result;

	return set_breakpoint(address, 0);
}

enum command_result
command_clear (int argc, char **argv)
{
	int b;
	uint16_t address;
	enum command_result result;

	if (argc != 2)
		return command_error;
	result = parse_uint16(argv[1], &address);
	if (result != command_success)
		return result;
	return clear_breakpoint(address, 0);
}

void
cc_stopped(uint8_t status)
{
	uint16_t pc;
	int b;
	int code;
	char *reason;

	pc = ccdbg_get_pc(s51_dbg);
	if (status & CC_STATUS_CPU_HALTED) {
		if ((status & CC_STATUS_HALT_STATUS) != 0) {
			pc = pc - 1;
			code = 104;
			reason = "Breakpoint";
			b = find_breakpoint(pc);
			if (b != -1 && breakpoints[b].temporary)
				clear_breakpoint(pc, 1);
			ccdbg_set_pc(s51_dbg, pc);
		} else {
			code = 105;
			reason = "Interrupt";
		}
		s51_printf("Stop at 0x%04x: (%d) %s\n",
			pc, code, reason);
	}
}

uint8_t
cc_step(uint16_t pc)
{
	int b;
	uint8_t status;

	b = find_breakpoint(pc);
	if (b != -1)
		disable_breakpoint(b);
	status = ccdbg_step_instr(s51_dbg);
	if (b != -1)
		enable_breakpoint(b);
	return status;
}

enum command_result
command_run (int argc, char **argv)
{
	uint16_t start, end;
	enum command_result result;
	uint16_t pc;
	uint8_t status;
	int b;

	if (argv[1]) {
		result = parse_uint16(argv[1], &start);
		if (result != command_success)
			return result;
		if (argv[2]) {
			result = parse_uint16(argv[2], &end);
			if (result != command_success)
				return result;
		}
		if (start_address && start == 0) {
			start = start_address;
			s51_printf("Starting at 0x%04x\n", start);
		}
		ccdbg_set_pc(s51_dbg, start);
	}
	else
		start = ccdbg_get_pc(s51_dbg);
	s51_printf("Resume at 0x%04x\n", start);
	pc = start;
	b = find_breakpoint(pc);
	if (b != -1) {
		cc_step(pc);
		pc = ccdbg_get_pc(s51_dbg);
		if (find_breakpoint(pc) != -1) {
			status = ccdbg_read_status(s51_dbg);
			cc_stopped(status);
			return command_success;
		}
	}
	ccdbg_resume(s51_dbg);
	result = cc_wait();
	return result;
}

enum command_result
command_next (int argc, char **argv)
{
	return command_step(argc, argv);
}

enum command_result
command_step (int argc, char **argv)
{
	uint16_t pc;
	uint8_t	opcode;
	uint8_t a;

	a = cc_step(ccdbg_get_pc(s51_dbg));
	s51_printf(" ACC= 0x%02x\n", a);
	pc = ccdbg_get_pc(s51_dbg);
	ccdbg_read_memory(s51_dbg, pc, &opcode, 1);
	s51_printf(" ? 0x%04x %02x\n", pc, opcode);
	return command_success;
}

enum command_result
command_load (int argc, char **argv)
{
	char *filename = argv[1];
	FILE *file;
	struct ao_hex_file	*hex;
	struct ao_hex_image *image;

	if (!filename)
		return command_error;
	file = fopen(filename, "r");
	if (!file) {
		perror(filename);
		return command_error;
	}
	hex = ao_hex_file_read(file, filename);
	fclose(file);
	if (!hex) {
		return command_error;
	}
	image = ao_hex_image_create(hex);
	ao_hex_file_free(hex);
	if (!image) {
		fprintf(stderr, "image create failed\n");
		return command_error;
	}
	if (image->address >= 0xf000) {
		printf("Loading %d bytes to RAM at 0x%04x\n",
		       image->length, image->address);
		ccdbg_write_hex_image(s51_dbg, image, 0);
	} else {
		fprintf(stderr, "Can only load to RAM\n");
	}
	ao_hex_image_free(image);
	return command_success;
}

enum command_result
command_halt (int argc, char **argv)
{
	uint16_t	pc;
	ccdbg_halt(s51_dbg);
	pc = ccdbg_get_pc(s51_dbg);
	s51_printf("Halted at 0x%04x\n", pc);
	return command_success;
}

enum command_result
command_stop (int argc, char **argv)
{
	return command_success;
}

enum command_result
command_reset (int argc, char **argv)
{
	ccdbg_debug_mode(s51_dbg);
	ccdbg_halt(s51_dbg);
	enable_breakpoints();
	return command_success;
}

enum command_result
command_status(int argc, char **argv)
{
	uint8_t	status;

	status = ccdbg_read_status(s51_dbg);
	if ((status & CC_STATUS_CHIP_ERASE_DONE) == 0)
		s51_printf("\tChip erase in progress\n");
	if (status & CC_STATUS_PCON_IDLE)
		s51_printf("\tCPU is idle (clock gated)\n");
	if (status & CC_STATUS_CPU_HALTED)
		s51_printf("\tCPU halted\n");
	else
		s51_printf("\tCPU running\n");
	if ((status & CC_STATUS_POWER_MODE_0) == 0)
		s51_printf("\tPower Mode 1-3 selected\n");
	if (status & CC_STATUS_HALT_STATUS)
		s51_printf("\tHalted by software or hw breakpoint\n");
	else
		s51_printf("\tHalted by debug command\n");
	if (status & CC_STATUS_DEBUG_LOCKED)
		s51_printf("\tDebug interface is locked\n");
	if ((status & CC_STATUS_OSCILLATOR_STABLE) == 0)
		s51_printf("\tOscillators are not stable\n");
	if (status & CC_STATUS_STACK_OVERFLOW)
		s51_printf("\tStack overflow\n");
	return command_success;
}

static enum command_result
info_breakpoints(int argc, char **argv)
{
	int b;
	uint16_t address;
	enum command_result result;

	if (argc == 1) {
		s51_printf("Num Type       Disp Hit   Cnt   Address  What\n");
		for (b = 0; b < CC_NUM_BREAKPOINTS; b++)
			if (breakpoints[b].enabled) {
				s51_printf("%-3d fetch      %s 1     1     0x%04x   uc::disass() unimplemented\n",
					   b,
					   breakpoints[b].temporary ? "del " : "keep",
					   breakpoints[b].address);
			}
		return command_success;
	}

}

static enum command_result
info_help(int argc, char **argv);

static struct command_function infos[] = {
	{ "breakpoints", "b", info_breakpoints, "[b]reakpoints",
		"List current breakpoints\n" },
	{ "help",   "?",  info_help,	"help",
		"Print this list\n" },

	{ NULL, NULL, NULL, NULL, NULL },
};

static enum command_result
info_help(int argc, char **argv)
{
	return command_function_help(infos, argc, argv);
}

enum command_result
command_info(int argc, char **argv)
{
	struct command_function *func;

	if (argc < 2)
		return command_error;
	func = command_string_to_function(infos, argv[1]);
	if (!func)
		return command_syntax;
	return (*func->func)(argc-1, argv+1);
}

enum command_result
cc_wait(void)
{
	for(;;) {
		uint8_t status;
		status = ccdbg_read_status(s51_dbg);
		if (status & CC_STATUS_CPU_HALTED) {
			cc_stopped(status);
			return command_success;
		}
		if (s51_interrupted || s51_check_input()) {

			ccdbg_halt(s51_dbg);
			status = ccdbg_read_status(s51_dbg);
			cc_stopped(status);
			return command_interrupt;
		}
	}
}
