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

struct command_function {
	char			*name;
	char			*alias;
	enum command_result	(*func)(FILE *output, int argc, char **argv);
	char			*usage;
	char			*help;
};

static struct command_function functions[] = {
	{ "help",   "?",  command_help,	"help",		"Print this list\n" },
	{ "quit",   "q",  command_quit,	"[q]uit",	"Quit\n" },
	{ "di",	    "di", command_di,	"di <start> <end>",
		"Dump imem\n" },
	{ "ds",	    "ds", command_ds,	"ds <start> <end>",
		"Dump sprs\n" },
	{ "dx",	    "dx", command_dx,	"dx <start> <end>",
		"Dump xaddr\n" },
	{ "set",    "t",  command_set,	"se[t] mem <prefix> <start> <end>",
		"Set mem {xram|rom|iram|sfr} <start> <end>\n"
		"set bit <addr>\n" },
	{ "dump",   "d",  command_dump,	"[d]ump <prefix> <start> <end>",
		"Dump {xram|rom|iram|sfr} <start> <end>\n" },
	{ "pc",	    "p",  command_pc, "[p]c [addr]",
		"Get or set pc value\n" },
	{ "break",  "b",  command_break,"[b]reak <addr>",
		"Set break point\n" },
	{ "clear",  "c",  command_clear,"[c]lear <addr>",
		"Clear break point\n" },
	{ "run",    "r",  command_run, "[r]un [start] [stop]",
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
};

#define NUM_FUNCTIONS (sizeof functions / sizeof functions[0])

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

static struct command_function *
command_string_to_function(char *name)
{
	int i;
	for (i = 0; i < NUM_FUNCTIONS; i++)
		if (!strcmp(name, functions[i].name) ||
		    !strcmp(name, functions[i].alias))
			return &functions[i];
	return NULL;
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
command_help(FILE *output, int argc, char **argv)
{
	int i;
	struct command_function *func;

	if (argc == 1) {
		for (i = 0; i < NUM_FUNCTIONS; i++)
			fprintf(output, "%-10s%s\n", functions[i].name,
			       functions[i].usage);
	} else {
		for (i = 1; i < argc; i++) {
			func = command_string_to_function(argv[i]);
			if (!func) {
				fprintf(output, "%-10s unknown command\n", argv[i]);
				return command_syntax;
			}
			fprintf(output, "%-10s %s\n%s", func->name,
			       func->usage, func->help);
		}
	}
	return command_debug;
}
    
static void
command_syntax_error(FILE *output, int argc, char **argv)
{
	fprintf(output, "Syntax error in:");
	while (*argv)
		fprintf(output, " %s", *argv++);
	fprintf(output, "\n");
}

void
command_read (FILE *input, FILE *output)
{
	int argc;
	char line[1024];
	char *argv[20];
	enum command_result result;
	struct command_function *func;

	s51_dbg = ccdbg_open ();
	if (!s51_dbg) {
		perror("ccdbg_open");
		exit(1);
	}
	ccdbg_debug_mode(s51_dbg);
	fprintf(output, "Welcome to the non-simulated processor\n");
	for (;;) {
		if (s51_prompt)
			fprintf(output, "%s", s51_prompt);
		else
			putc('\0', output);
		fflush(output);
		if (!fgets (line, sizeof line, input))
			break;
		argc = command_split_into_words(line, argv);
		if (argc > 0) {
			func = command_string_to_function(argv[0]);
			if (!func)
				command_syntax_error(output, argc, argv);
			else
			{
				result = (*func->func)(output, argc, argv);
				switch (result) {
				case command_syntax:
					command_syntax_error(output, argc, argv);
					break;
				case command_error:
					fprintf(output, "Error\n");
					break;
				case command_proceed:
					break;
				default:
					break;
				}
			}
		}
	}
	ccdbg_close(s51_dbg);
	fprintf(output, "...\n");
}

