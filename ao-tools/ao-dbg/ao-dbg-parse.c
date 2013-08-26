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

static struct command_function functions[] = {
	{ "help",   "?",  command_help,	"help",		"Print this list\n" },
	{ "quit",   "q",  command_quit,	"[q]uit",	"Quit\n" },
	{ "di",	    "di", command_di,	"di <start> <end>",
		"Dump imem\n" },
	{ "ds",	    "ds", command_ds,	"ds <start> <end>",
		"Dump sprs\n" },
	{ "dx",	    "dx", command_dx,	"dx <start> <end>",
		"Dump xaddr\n" },
	{ "set",    "t",  command_set,	"se[t] mem <prefix> <address> <data> ...",
		"Set mem {xram|rom|iram|sfr}\n"
		"set bit <addr>\n" },
	{ "dump",   "d",  command_dump,	"[d]ump <prefix> <start> <end>",
		"Dump {xram|rom|iram|sfr} <start> <end>\n" },
	{ "file", "file", command_file, "file <filename>",
		"Pretend to load executable from <filename>\n" },
	{ "pc",	    "p",  command_pc, "[p]c [addr]",
		"Get or set pc value\n" },
	{ "break",  "b",  command_break,"[b]reak <addr>",
		"Set break point\n" },
	{ "clear",  "c",  command_clear,"[c]lear <addr>",
		"Clear break point\n" },
	{ "run",    "r",  command_run, "[r]un [start] [stop]",
		"Run with optional start and temp breakpoint addresses\n" },
	{ "go",     "g",  command_run, "[g]o [start] [stop]",
		"Run with optional start and temp breakpoint addresses\n" },
	{ "next",   "n",  command_next, "[n]ext",
		"Step over one instruction, past any call\n" },
	{ "step",   "s",  command_step, "[s]tep",
		"Single step\n" },
	{ "load",   "l",  command_load, "[l]oad <file>",
		"Load a hex file into memory or flash" },
	{ "halt",   "h",  command_halt, "[h]alt",
		"Halt the processor\n" },
	{ "reset","res",command_reset,	"[res]et",
		"Reset the CPU\n" },
	{ "status","status",command_status, "status",
		"Display CC1111 debug status\n" },
	{ "info",   "i",  command_info, "[i]info",
		"Get information\n" },
	{ "stop",  "stop", command_stop, "stop",
		"Ignored\n" },
	{ NULL, NULL, NULL, NULL, NULL },
};

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

static int
string_to_int(char *s, int *v)
{
	char *endptr;

	if (isdigit(s[0]) || s[0] == '-' || s[0] == '+') {
		*v = strtol(s, &endptr, 0);
		if (endptr == s)
			return FALSE;
	} else if (*s == '\'') {
		s++;
		if (*s == '\\') {
			s++;
			switch (*s) {
			case 'n':
				*v = '\n';
				break;
			case 't':
				*v = '\t';
				break;
			default:
				*v = (int) *s;
				break;
			}
		} else
			*v = (int) *s;
		s++;
		if (*s != '\'')
			return FALSE;
	}
	else
		return FALSE;
    return TRUE;
}

struct command_function *
command_string_to_function(struct command_function *functions, char *name)
{
	int i;
	for (i = 0; functions[i].name; i++)
		if (!strcmp(name, functions[i].name) ||
		    !strcmp(name, functions[i].alias))
			return &functions[i];
	return NULL;
}

enum command_result
command_function_help(struct command_function *functions, int argc, char **argv)
{
	int i;
	struct command_function *func;

	if (argc == 1) {
		for (i = 0; functions[i].name; i++)
			s51_printf("%-10s%s\n", functions[i].name,
			       functions[i].usage);
	} else {
		for (i = 1; i < argc; i++) {
			func = command_string_to_function(functions, argv[i]);
			if (!func) {
				s51_printf("%-10s unknown command\n", argv[i]);
				return command_syntax;
			}
			s51_printf("%-10s %s\n%s", func->name,
			       func->usage, func->help);
		}
	}
	return command_debug;
}

static int
command_split_into_words(char *line, char **argv)
{
	char quotechar;
	int argc;

	argc = 0;
	while (*line) {
		while (isspace(*line))
			line++;
		if (!*line)
			break;
		if (*line == '"') {
			quotechar = *line++;
			*argv++ = line;
			argc++;
			while (*line && *line != quotechar)
				line++;
			if (*line)
				*line++ = '\0';
		} else {
			*argv++ = line;
			argc++;
			while (*line && !isspace(*line))
				line++;
			if (*line)
				*line++ = '\0';
		}
	}
	*argv = 0;
	return argc;
}

enum command_result
command_help(int argc, char **argv)
{
	return command_function_help(functions, argc, argv);
}

void
command_syntax_error(int argc, char **argv)
{
	s51_printf("Syntax error in:");
	while (*argv)
		s51_printf(" %s", *argv++);
	s51_printf("\n");
}

void
command_read (void)
{
	int argc;
	char line[1024];
	char *argv[20];
	enum command_result result;
	struct command_function *func;

	if (!s51_tty) {
		if (!s51_device)
			s51_device = getenv("AO_DBG_DEVICE");
		s51_tty = cc_usbdevs_find_by_arg(s51_device, "TeleDongle");
	}
	s51_dbg = ccdbg_open (s51_tty);
	if (!s51_dbg)
		exit(1);
	ccdbg_debug_mode(s51_dbg);
	ccdbg_halt(s51_dbg);
	s51_printf("Welcome to the non-simulated processor\n");
	for (;;) {
		if (s51_read_line (line, sizeof line) == 0)
			break;
		s51_interrupted = 0;
		argc = command_split_into_words(line, argv);
		if (argc > 0) {
			func = command_string_to_function(functions, argv[0]);
			if (!func)
				command_syntax_error(argc, argv);
			else
			{
				result = (*func->func)(argc, argv);
				if (s51_interrupted)
					result = command_interrupt;
				switch (result) {
				case command_syntax:
					command_syntax_error(argc, argv);
					break;
				case command_error:
					s51_printf("Error\n");
					break;
				case command_success:
					break;
				case command_interrupt:
					ccdbg_halt(s51_dbg);
					s51_printf("Interrupted\n");
					break;
				default:
					break;
				}
			}
		}
	}
	ccdbg_close(s51_dbg);
	s51_printf("...\n");
}
