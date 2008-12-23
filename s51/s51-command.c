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

#include "s51.h"

static enum command_result
parse_int(char *value, int *result)
{
	char *endptr;

	*result = strtol(value, &endptr, 0);
	if (endptr == value)
		return command_syntax;
	return command_proceed;
}

static enum command_result
parse_uint16(char *value, uint16_t *uint16)
{
	int	v;
	enum command_result result;

	result = parse_int(value, &v);
	if (result != command_proceed)
		return command_error;
	if (v < 0 || v > 0xffff)
		return command_error;
	*uint16 = v;
	return command_proceed;
}

enum command_result
command_quit (FILE *output, int argc, char **argv)
{
	exit(0);
	return command_error;
}

static void
dump_bytes(FILE *output, uint8_t *memory, int length, uint16_t start)
{
	int group, i;
	
	for (group = 0; group < length; group += 8) {
		fprintf(output, "0x%04x ", start + group);
		for (i = group; i < length && i < group + 8; i++)
			fprintf(output, "%02x ", memory[i]);
		for (; i < group + 8; i++)
			fprintf(output, "   ");
		for (i = group; i < length && i < group + 8; i++) {
			if (isascii(memory[i]) && isprint(memory[i]))
				fprintf(output, "%c", memory[i]);
			else
				fprintf(output, ".");
		}
		fprintf(output, "\n");
	}
}

enum command_result
command_di (FILE *output, int argc, char **argv)
{
	uint16_t start, end;
	uint8_t	memory[65536];
	uint8_t status;
	int length;
	
	if (argc != 3)
		return command_error;
	if (parse_uint16(argv[1], &start) != command_proceed)
		return command_error;
	if (parse_uint16(argv[2], &end) != command_proceed)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_memory(s51_dbg, start + 0xff00, memory, length);
	dump_bytes(output, memory, length, start);
	return command_proceed;
}

enum command_result
command_ds (FILE *output, int argc, char **argv)
{
	uint16_t start, end;
	uint8_t	memory[65536];
	uint8_t status;
	int length;
	
	if (argc != 3)
		return command_error;
	if (parse_uint16(argv[1], &start) != command_proceed)
		return command_error;
	if (parse_uint16(argv[2], &end) != command_proceed)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_memory(s51_dbg, start + 0xdf00, memory, length);
	dump_bytes(output, memory, length, start);
	return command_proceed;
}

enum command_result
command_dx (FILE *output, int argc, char **argv)
{
	uint16_t start, end;
	uint8_t	memory[65536];
	uint8_t status;
	int length;
	
	if (argc != 3)
		return command_error;
	if (parse_uint16(argv[1], &start) != command_proceed)
		return command_error;
	if (parse_uint16(argv[2], &end) != command_proceed)
		return command_error;
	length = (int) end - (int) start + 1;
	status = ccdbg_read_memory(s51_dbg, start, memory, length);
	dump_bytes(output, memory, length, start);
	return command_proceed;
}

enum command_result
command_set (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_dump (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_pc (FILE *output, int argc, char **argv)
{
	uint16_t	pc;
	if (argv[1]) {
		enum command_result result;
		result = parse_uint16(argv[1], &pc);
		if (result != command_proceed)
			return result;
		ccdbg_set_pc(s51_dbg, pc);
	} else {
		pc = ccdbg_get_pc(s51_dbg);
		printf (" 0x%04x\n", pc);
	}
	return command_proceed;
}

struct cc_break {
	int		enabled;
	int		temporary;
	uint16_t	address;
};

#define CC_NUM_BREAKPOINTS 4

static struct cc_break	breakpoints[CC_NUM_BREAKPOINTS];

enum command_result
set_breakpoint(FILE *output, uint16_t address, int temporary)
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
		fprintf(output, "Error: too many breakpoints requested\n");
		return command_proceed;
	}
	if (breakpoints[b].enabled == 0) {
		breakpoints[b].address = address;
		status = ccdbg_set_hw_brkpnt(s51_dbg, b, 1, address);
		fprintf(output, "set_hw_brkpnt status 0x%02x\n", status);
	}
	++breakpoints[b].enabled;
	fprintf(output, "Breakpoint %d at 0x%04x\n", b, address);
	breakpoints[b].temporary += temporary;
	return command_proceed;
}

enum command_result
clear_breakpoint(FILE *output, uint16_t address, int temporary)
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
		fprintf(output, "Error: no matching breakpoint found\n");
		return command_proceed;
	}
	--breakpoints[b].enabled;
	--breakpoints[b].temporary;
	if (breakpoints[b].enabled == 0) {
		breakpoints[b].address = -1;
		ccdbg_set_hw_brkpnt(s51_dbg, b, 0, address);
		fprintf(output, "set_hw_brkpnt status 0x%02x\n", status);
	}
	return command_proceed;
}

enum command_result
command_break (FILE *output, int argc, char **argv)
{
	int b;
	uint16_t address;
	enum command_result result;

	if (argc == 1) {
		for (b = 0; b < CC_NUM_BREAKPOINTS; b++)
			if (breakpoints[b].enabled)
				fprintf(output, "Breakpoint %d 0x%04x\n",
					b, breakpoints[b].address);
		return command_proceed;
	}
	if (argc != 2)
		return command_error;
	result = parse_uint16(argv[1], &address);
	if (result != command_proceed)
		return result;

	return set_breakpoint(output, address, 0);
}

enum command_result
command_clear (FILE *output, int argc, char **argv)
{
	int b;
	uint16_t address;
	enum command_result result;

	if (argc != 2)
		return command_error;
	result = parse_uint16(argv[1], &address);
	if (result != command_proceed)
		return result;
	return clear_breakpoint(output, address, 0);
}

enum command_result
command_run (FILE *output, int argc, char **argv)
{
	uint16_t start, end;
	enum command_result result;
	
	if (argv[1]) {
		result = parse_uint16(argv[1], &start);
		if (result != command_proceed)
			return result;
		if (argv[2]) {
			result = parse_uint16(argv[2], &end);
			if (result != command_proceed)
				return result;
		}
		ccdbg_set_pc(s51_dbg, start);
	}
	else
		start = ccdbg_get_pc(s51_dbg);
	fprintf(output, "Resume at 0x%04x\n", start);
	ccdbg_resume(s51_dbg);
//	cc_wait(s51_dbg);
	return command_proceed;
}

enum command_result
command_next (FILE *output, int argc, char **argv)
{
	return command_step(output, argc, argv);
}

enum command_result
command_step (FILE *output, int argc, char **argv)
{
	uint16_t pc;
	uint8_t	opcode;
	uint8_t a;

	a = ccdbg_step_instr(s51_dbg);
	fprintf(output, " ACC= 0x%02x\n", a);
	pc = ccdbg_get_pc(s51_dbg);
	ccdbg_read_memory(s51_dbg, pc, &opcode, 1);
	fprintf(output, " ? 0x%04x %02x\n", pc, opcode);
	return command_proceed;
}

enum command_result
command_load (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_halt (FILE *output, int argc, char **argv)
{
	uint16_t	pc;
	ccdbg_halt(s51_dbg);
	pc = ccdbg_get_pc(s51_dbg);
	fprintf(output, "Halted at 0x%04x\n", pc);
	return command_proceed;
}

enum command_result
command_reset (FILE *output, int argc, char **argv)
{
	ccdbg_debug_mode(s51_dbg);
	return command_proceed;
}

enum command_result
command_status(FILE *output, int argc, char **argv)
{
	uint8_t	status;

	status = ccdbg_read_status(s51_dbg);
	if ((status & CC_STATUS_CHIP_ERASE_DONE) == 0)
		fprintf(output, "\tChip erase in progress\n");
	if (status & CC_STATUS_PCON_IDLE)
		fprintf(output, "\tCPU is idle (clock gated)\n");
	if (status & CC_STATUS_CPU_HALTED)
		fprintf(output, "\tCPU halted\n");
	else
		fprintf(output, "\tCPU running\n");
	if ((status & CC_STATUS_POWER_MODE_0) == 0)
		fprintf(output, "\tPower Mode 1-3 selected\n");
	if (status & CC_STATUS_HALT_STATUS)
		fprintf(output, "\tHalted by software or hw breakpoint\n");
	else
		fprintf(output, "\tHalted by debug command\n");
	if (status & CC_STATUS_DEBUG_LOCKED)
		fprintf(output, "\tDebug interface is locked\n");
	if ((status & CC_STATUS_OSCILLATOR_STABLE) == 0)
		fprintf(output, "\tOscillators are not stable\n");
	if (status & CC_STATUS_STACK_OVERFLOW)
		fprintf(output, "\tStack overflow\n");
	return command_proceed;
}

uint8_t cc_wait(struct ccdbg *dbg)
{
	uint8_t status;
	for(;;) {
		status = ccdbg_read_status(dbg);
		if (status & CC_STATUS_CPU_HALTED)
			break;
	}
	return status;
}
