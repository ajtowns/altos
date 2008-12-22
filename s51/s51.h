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

#include <ccdbg.h>

extern char *s51_prompt;

extern struct ccdbg *s51_dbg;

enum command_result {
	command_proceed, command_debug, command_syntax, command_error
};

enum command_result
command_quit (FILE *output, int argc, char **argv);

enum command_result
command_help (FILE *output, int argc, char **argv);

enum command_result
command_di (FILE *output, int argc, char **argv);

enum command_result
command_ds (FILE *output, int argc, char **argv);

enum command_result
command_dx (FILE *output, int argc, char **argv);

enum command_result
command_set (FILE *output, int argc, char **argv);

enum command_result
command_dump (FILE *output, int argc, char **argv);

enum command_result
command_pc (FILE *output, int argc, char **argv);

enum command_result
command_break (FILE *output, int argc, char **argv);

enum command_result
command_clear (FILE *output, int argc, char **argv);

enum command_result
command_run (FILE *output, int argc, char **argv);

enum command_result
command_next (FILE *output, int argc, char **argv);

enum command_result
command_step (FILE *output, int argc, char **argv);

enum command_result
command_load (FILE *output, int argc, char **argv);

enum command_result
command_halt (FILE *output, int argc, char **argv);

enum command_result
command_reset (FILE *output, int argc, char **argv);

void
command_read (FILE *input, FILE *output);
