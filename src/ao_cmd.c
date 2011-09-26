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

__pdata uint16_t ao_cmd_lex_i;
__pdata uint32_t ao_cmd_lex_u32;
__pdata char	ao_cmd_lex_c;
__pdata enum ao_cmd_status ao_cmd_status;

#define CMD_LEN	32

static __xdata char	cmd_line[CMD_LEN];
static __pdata uint8_t	cmd_len;
static __pdata uint8_t	cmd_i;

static void
put_string(__code char *s)
{
	char	c;
	while (c = *s++)
		putchar(c);
}

static void
readline(void)
{
	__pdata char c;
	if (ao_echo())
		put_string("> ");
	cmd_len = 0;
	for (;;) {
		flush();
		c = getchar();
		/* backspace/delete */
		if (c == '\010' || c == '\177') {
			if (cmd_len != 0) {
				if (ao_echo())
					put_string("\010 \010");
				--cmd_len;
			}
			continue;
		}

		/* ^U */
		if (c == '\025') {
			while (cmd_len != 0) {
				if (ao_echo())
					put_string("\010 \010");
				--cmd_len;
			}
			continue;
		}

		/* map CR to NL */
		if (c == '\r')
			c = '\n';

		if (c == '\n') {
			if (ao_echo())
				putchar('\n');
			break;
		}

		if (cmd_len >= CMD_LEN - 2) {
			if (ao_echo())
				putchar('\007');
			continue;
		}
		cmd_line[cmd_len++] = c;
		if (ao_echo())
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
	ao_cmd_put8(v >> 8);
	ao_cmd_put8(v);
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
	__pdata uint8_t	r = ao_cmd_lex_error;
	uint8_t	n;

	ao_cmd_lex_i = 0;
	ao_cmd_white();
	for(;;) {
		if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9')
			n = (ao_cmd_lex_c - '0');
		else if ('a' <= ao_cmd_lex_c && ao_cmd_lex_c <= 'f')
			n = (ao_cmd_lex_c - 'a' + 10);
		else if ('A' <= ao_cmd_lex_c && ao_cmd_lex_c <= 'F')
			n = (ao_cmd_lex_c - 'A' + 10);
		else
			break;
		ao_cmd_lex_i = (ao_cmd_lex_i << 4) | n;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
}

void
ao_cmd_decimal(void)
{
	__pdata uint8_t	r = ao_cmd_lex_error;

	ao_cmd_lex_u32 = 0;
	ao_cmd_white();
	for(;;) {
		if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9')
			ao_cmd_lex_u32 = (ao_cmd_lex_u32 * 10) + (ao_cmd_lex_c - '0');
		else
			break;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
	ao_cmd_lex_i = (uint16_t) ao_cmd_lex_u32;
}

uint8_t
ao_match_word(__code char *word)
{
	while (*word) {
		if (ao_cmd_lex_c != *word) {
			ao_cmd_status = ao_cmd_syntax_error;
			return 0;
		}
		word++;
		ao_cmd_lex();
	}
	return 1;
}

static void
eol(void)
{
	while (ao_cmd_lex_c != '\n')
		ao_cmd_lex();
}

static void
echo(void)
{
	ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success)
		ao_stdios[ao_cur_stdio].echo = ao_cmd_lex_i != 0;
}

static void
ao_reboot(void)
{
	ao_cmd_white();
	if (!ao_match_word("eboot"))
		return;

	/* Delay waiting for the packet master to be turned off
	 * so that we don't end up back in idle mode because we
	 * received a packet after boot.
	 */
	flush();
	ao_delay(AO_SEC_TO_TICKS(1));
	WDCTL = WDCTL_EN | WDCTL_MODE_WATCHDOG | WDCTL_INT_64;
	ao_delay(AO_SEC_TO_TICKS(2));
	ao_panic(AO_PANIC_REBOOT);
}

static void
version(void)
{
	printf("manufacturer     %s\n", ao_manufacturer);
	printf("product          %s\n", ao_product);
	printf("serial-number    %u\n", ao_serial_number);
#if HAS_EEPROM
	printf("log-format       %u\n", ao_log_format);
#endif
	printf("software-version %s\n", ao_version);
}

#define NUM_CMDS	11

static __code struct ao_cmds	*__xdata (ao_cmds[NUM_CMDS]);
static __pdata uint8_t		ao_ncmds;

static void
help(void)
{
	register uint8_t cmds;
	register uint8_t cmd;
	register __code struct ao_cmds * cs;

	for (cmds = 0; cmds < ao_ncmds; cmds++) {
		cs = ao_cmds[cmds];
		for (cmd = 0; cs[cmd].func; cmd++)
			printf("%-45s %s\n",
				cs[cmd].help,
				cs[cmd].help+1+strlen(cs[cmd].help));
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
ao_cmd(void)
{
	char	c;
	uint8_t cmd, cmds;
	__code struct ao_cmds * __xdata cs;
	void (*__xdata func)(void);

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
			for (cmd = 0; cs[cmd].func; cmd++)
				if (cs[cmd].help[0] == c) {
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
	{ help,		"?\0Help" },
	{ ao_task_info,	"T\0Show tasks" },
	{ echo,		"E <0 off, 1 on>\0Set echo mode" },
	{ ao_reboot,	"r eboot\0Reboot" },
	{ version,	"v\0Version" },
	{ 0,	NULL },
};

void
ao_cmd_init(void)
{
	ao_cmd_register(&ao_base_cmds[0]);
	ao_add_task(&ao_cmd_task, ao_cmd, "cmd");
}
