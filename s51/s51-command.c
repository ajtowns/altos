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

enum command_result
command_di (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_ds (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_dx (FILE *output, int argc, char **argv)
{
	return command_error;
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

enum command_result
command_break (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_clear (FILE *output, int argc, char **argv)
{
	return command_error;
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
	return command_proceed;
}

enum command_result
command_next (FILE *output, int argc, char **argv)
{
	return command_error;
}

enum command_result
command_step (FILE *output, int argc, char **argv)
{
	return command_error;
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
