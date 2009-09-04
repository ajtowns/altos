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
#include <cc.h>

extern char *s51_prompt;
extern struct ccdbg *s51_dbg;
extern int s51_interrupted;
extern int s51_monitor;
extern char *s51_tty;
extern char *s51_device;

enum command_result {
	command_success, command_debug, command_syntax, command_interrupt, command_error,
};

struct command_function {
	char			*name;
	char			*alias;
	enum command_result	(*func)(int argc, char **argv);
	char			*usage;
	char			*help;
};

struct command_function *
command_string_to_function(struct command_function *functions, char *name);

enum command_result
command_function_help(struct command_function *functions, int argc, char **argv);

void
command_syntax_error(int argc, char **argv);

enum command_result
command_quit (int argc, char **argv);

enum command_result
command_help (int argc, char **argv);

enum command_result
command_stop (int argc, char **argv);

enum command_result
command_di (int argc, char **argv);

enum command_result
command_ds (int argc, char **argv);

enum command_result
command_dx (int argc, char **argv);

enum command_result
command_set (int argc, char **argv);

enum command_result
command_dump (int argc, char **argv);

enum command_result
command_file (int argc, char **argv);

enum command_result
command_pc (int argc, char **argv);

enum command_result
command_break (int argc, char **argv);

enum command_result
command_clear (int argc, char **argv);

enum command_result
command_run (int argc, char **argv);

enum command_result
command_next (int argc, char **argv);

enum command_result
command_step (int argc, char **argv);

enum command_result
command_load (int argc, char **argv);

enum command_result
command_halt (int argc, char **argv);

enum command_result
command_reset (int argc, char **argv);

enum command_result
command_status (int argc, char **argv);

enum command_result
command_info (int argc, char **argv);

enum command_result
cc_wait(void);

void
command_read (void);

void
s51_printf(char *format, ...);

void
s51_putc(int c);

int
s51_check_input(void);

int
s51_read_line(char *line, int len);
