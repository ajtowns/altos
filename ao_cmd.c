/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#include "ao.h"

__xdata uint16_t ao_cmd_lex_i;
__xdata uint8_t	ao_cmd_lex_c;
__xdata enum ao_cmd_status ao_cmd_status;
static __xdata uint8_t	lex_echo;

#define CMD_LEN	32

static __xdata uint8_t	cmd_line[CMD_LEN];
static __xdata uint8_t	cmd_len;
static __xdata uint8_t	cmd_i;

static void
put_string(char *s)
{
	__xdata uint8_t	c;
	while (c = *s++)
		putchar(c);
}

static void
readline(void)
{
	__xdata uint8_t c;
	if (lex_echo)
		put_string("> ");
	cmd_len = 0;
	for (;;) {
		flush();
		c = getchar();
		/* backspace/delete */
		if (c == '\010' || c == '\177') {
			if (cmd_len != 0) {
				if (lex_echo)
					put_string("\010 \010");
				--cmd_len;
			}
			continue;
		}

		/* ^U */
		if (c == '\025') {
			while (cmd_len != 0) {
				if (lex_echo)
					put_string("\010 \010");
				--cmd_len;
			}
			continue;
		}

		/* map CR to NL */
		if (c == '\r')
			c = '\n';
		
		if (c == '\n') {
			if (lex_echo)
				putchar('\n');
			break;
		}

		if (cmd_len >= CMD_LEN - 2) {
			if (lex_echo)
				putchar('\007');
			continue;
		}
		cmd_line[cmd_len++] = c;
		if (lex_echo)
			putchar(c);
	}
	cmd_line[cmd_len++] = '\n';
	cmd_line[cmd_len++] = '\0';
	cmd_i = 0;
}

void
ao_cmd_lex(void)
{
	ao_cmd_lex_c = '\n';
	if (cmd_i < cmd_len)
		ao_cmd_lex_c = cmd_line[cmd_i++];
}

static void
putnibble(uint8_t v)
{
	if (v < 10)
		putchar(v + '0');
	else
		putchar(v + ('a' - 10));
}

void
ao_cmd_put16(uint16_t v)
{
	int8_t i;
	for (i = 3; i >= 0; i--)
		putnibble((v >> (i << 2)) & 0xf);
}

void
ao_cmd_put8(uint8_t v)
{
	putnibble((v >> 4) & 0xf);
	putnibble(v & 0xf);
}

void
ao_cmd_white(void)
{
	while (ao_cmd_lex_c == ' ' || ao_cmd_lex_c == '\t')
		ao_cmd_lex();
}

void
ao_cmd_hex(void)
{
	__xdata uint8_t	r = ao_cmd_lex_error;
	
	ao_cmd_lex_i = 0;
	ao_cmd_white();
	for(;;) {
		if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9')
			ao_cmd_lex_i = (ao_cmd_lex_i << 4) | (ao_cmd_lex_c - '0');
		else if ('a' <= ao_cmd_lex_c && ao_cmd_lex_c <= 'f')
			ao_cmd_lex_i = (ao_cmd_lex_i << 4) | (ao_cmd_lex_c - 'a' + 10);
		else if ('A' <= ao_cmd_lex_c && ao_cmd_lex_c <= 'F')
			ao_cmd_lex_i = (ao_cmd_lex_i << 4) | (ao_cmd_lex_c - 'A' + 10);
		else
			break;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
}

void
ao_cmd_decimal(void)
{
	__xdata uint8_t	r = ao_cmd_lex_error;
	
	ao_cmd_lex_i = 0;
	ao_cmd_white();
	for(;;) {
		if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9')
			ao_cmd_lex_i = (ao_cmd_lex_i * 10) + (ao_cmd_lex_c - '0');
		else
			break;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
}

static void
eol(void)
{
	while (ao_cmd_lex_c != '\n')
		ao_cmd_lex();
}

static void
dump(void)
{
	__xdata uint16_t c;
	__xdata uint8_t * __xdata start, * __xdata end;

	ao_cmd_hex();
	start = (uint8_t __xdata *) ao_cmd_lex_i;
	ao_cmd_hex();
	end = (uint8_t __xdata *) ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	c = 0;
	while (start <= end) {
		if ((c & 7) == 0) {
			if (c)
				putchar('\n');
			ao_cmd_put16((uint16_t) start);
		}
		putchar(' ');
		ao_cmd_put8(*start);
		++c;
		start++;
	}
	putchar('\n');
}

static void
echo(void)
{
	ao_cmd_hex();
	lex_echo = ao_cmd_lex_i != 0;
}

static const uint8_t help_txt[] = "All numbers are in hex";

#define NUM_CMDS	11

static __code struct ao_cmds	*__xdata (ao_cmds[NUM_CMDS]);
static __xdata uint8_t		ao_ncmds;

static void
help(void)
{
	__xdata uint8_t	cmds;
	__xdata uint8_t cmd;
	__code struct ao_cmds * __xdata cs;
	puts(help_txt);
	for (cmds = 0; cmds < ao_ncmds; cmds++) {
		cs = ao_cmds[cmds];
		for (cmd = 0; cs[cmd].cmd != '\0'; cmd++)
			puts(cs[cmd].help);
	}
}

static void
report(void)
{
	switch(ao_cmd_status) {
	case ao_cmd_lex_error:
	case ao_cmd_syntax_error:
		puts("Syntax error");
		ao_cmd_status = 0;
		break;
	}
}

void
ao_cmd_register(__code struct ao_cmds *cmds)
{
	if (ao_ncmds >= NUM_CMDS)
		ao_panic(AO_PANIC_CMD);
	ao_cmds[ao_ncmds++] = cmds;
}

void
ao_cmd(void *parameters)
{
	__xdata uint8_t	c;
	__xdata uint8_t cmd, cmds;
	__code struct ao_cmds * __xdata cs;
	void (*__xdata func)(void);
	(void) parameters;

	lex_echo = 1;
	for (;;) {
		readline();
		ao_cmd_lex();
		ao_cmd_white();
		c = ao_cmd_lex_c;
		ao_cmd_lex();
		if (c == '\r' || c == '\n')
			continue;
		func = (void (*)(void)) NULL;
		for (cmds = 0; cmds < ao_ncmds; cmds++) {
			cs = ao_cmds[cmds];
			for (cmd = 0; cs[cmd].cmd != '\0'; cmd++)
				if (cs[cmd].cmd == c) {
					func = cs[cmd].func;
					break;
				}
			if (func)
				break;
		}
		if (func)
			(*func)();
		else
			ao_cmd_status = ao_cmd_syntax_error;
		report();
	}
}

__xdata struct ao_task ao_cmd_task;

__code struct ao_cmds	ao_base_cmds[] = {
	{ '?', help,		"?                                  Print this message" },
	{ 'T', ao_task_info,	"T                                  Show task states" },
	{ 'E', echo,		"E <0 off, 1 on>                    Set command echo mode" },
	{ 'd', dump,		"d <start> <end>                    Dump memory" },
	{ 0,    help,	NULL },
};

void
ao_cmd_init(void)
{
	ao_cmd_register(&ao_base_cmds[0]);
	ao_add_task(&ao_cmd_task, ao_cmd, "cmd");
}
