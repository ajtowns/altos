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
#include "ao_task.h"

__pdata uint16_t ao_cmd_lex_i;
__pdata uint32_t ao_cmd_lex_u32;
__pdata char	ao_cmd_lex_c;
__pdata enum ao_cmd_status ao_cmd_status;

#if AO_PYRO_NUM
#define CMD_LEN 128
#else
#define CMD_LEN	48
#endif

static __xdata char	cmd_line[CMD_LEN];
static __pdata uint8_t	cmd_len;
static __pdata uint8_t	cmd_i;

void
ao_put_string(__code char *s)
{
	char	c;
	while ((c = *s++))
		putchar(c);
}

static void
backspace(void)
{
	ao_put_string ("\010 \010");
}

static void
readline(void)
{
	char c;
	if (ao_echo())
		ao_put_string("> ");
	cmd_len = 0;
	for (;;) {
		flush();
		c = getchar();
		/* backspace/delete */
		if (c == '\010' || c == '\177') {
			if (cmd_len != 0) {
				if (ao_echo())
					backspace();
				--cmd_len;
			}
			continue;
		}

		/* ^U */
		if (c == '\025') {
			while (cmd_len != 0) {
				if (ao_echo())
					backspace();
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

		if (cmd_len >= CMD_LEN - 2)
			continue;
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

uint8_t
ao_getnibble(void)
{
	char	c;

	c = getchar();
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - ('a' - 10);
	if ('A' <= c && c <= 'F')
		return c - ('A' - 10);
	ao_cmd_status = ao_cmd_lex_error;
	return 0;
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

uint8_t
ao_cmd_is_white(void)
{
	return ao_cmd_lex_c == ' ' || ao_cmd_lex_c == '\t';
}

void
ao_cmd_white(void)
{
	while (ao_cmd_is_white())
		ao_cmd_lex();
}

int8_t
ao_cmd_hexchar(char c)
{
	if ('0' <= c && c <= '9')
		return (c - '0');
	if ('a' <= c && c <= 'f')
		return (c - 'a' + 10);
	if ('A' <= c && c <= 'F')
		return (c - 'A' + 10);
	return -1;
}

void
ao_cmd_hexbyte(void)
{
	uint8_t i;
	int8_t	n;

	ao_cmd_lex_i = 0;
	ao_cmd_white();
	for (i = 0; i < 2; i++) {
		n = ao_cmd_hexchar(ao_cmd_lex_c);
		if (n < 0) {
			ao_cmd_status = ao_cmd_syntax_error;
			break;
		}
		ao_cmd_lex_i = (ao_cmd_lex_i << 4) | n;
		ao_cmd_lex();
	}
}

void
ao_cmd_hex(void)
{
	__pdata uint8_t	r = ao_cmd_lex_error;
	int8_t	n;

	ao_cmd_lex_i = 0;
	ao_cmd_white();
	for(;;) {
		n = ao_cmd_hexchar(ao_cmd_lex_c);
		if (n < 0)
			break;
		ao_cmd_lex_i = (ao_cmd_lex_i << 4) | n;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
}

void
ao_cmd_decimal(void) __reentrant
{
	uint8_t	r = ao_cmd_lex_error;

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
	ao_arch_reboot();
	ao_panic(AO_PANIC_REBOOT);
}

#ifndef HAS_VERSION
#define HAS_VERSION 1
#endif

#if HAS_VERSION
static void
version(void)
{
	printf("manufacturer     %s\n"
	       "product          %s\n"
	       "serial-number    %u\n"
#if HAS_FLIGHT || HAS_TRACKER
	       "current-flight   %u\n"
#endif
#if HAS_LOG
	       "log-format       %u\n"
#if !DISABLE_LOG_SPACE
	       "log-space	 %lu\n"
#endif
#endif
#if defined(AO_BOOT_APPLICATION_BASE) && defined(AO_BOOT_APPLICATION_BOUND)
	       "program-space    %u\n"
#endif
	       , ao_manufacturer
	       , ao_product
	       , ao_serial_number
#if HAS_FLIGHT || HAS_TRACKER
	       , ao_flight_number
#endif
#if HAS_LOG
	       , ao_log_format
#if !DISABLE_LOG_SPACE
	       , (unsigned long) ao_storage_log_max
#endif
#endif
#if defined(AO_BOOT_APPLICATION_BASE) && defined(AO_BOOT_APPLICATION_BOUND)
	       , (uint32_t) AO_BOOT_APPLICATION_BOUND - (uint32_t) AO_BOOT_APPLICATION_BASE
#endif
		);
	printf("software-version %s\n", ao_version);
}
#endif

#ifndef NUM_CMDS
#define NUM_CMDS	11
#endif

static __code struct ao_cmds	*__xdata (ao_cmds[NUM_CMDS]);
static __pdata uint8_t		ao_ncmds;

static void
help(void)
{
	__pdata uint8_t cmds;
	__pdata uint8_t cmd;
	__code struct ao_cmds * __pdata cs;
	__code const char *h;
	uint8_t e;

	for (cmds = 0; cmds < ao_ncmds; cmds++) {
		cs = ao_cmds[cmds];
		for (cmd = 0; cs[cmd].func; cmd++) {
			h = cs[cmd].help;
			ao_put_string(h);
			e = strlen(h);
			h += e + 1;
			e = 45 - e;
			while (e--)
				putchar(' ');
			ao_put_string(h);
			putchar('\n');
		}
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
	default:
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
	__pdata char	c;
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

#if HAS_BOOT_LOADER

#include <ao_boot.h>

static void
ao_loader(void)
{
	flush();
	ao_boot_loader();
}
#endif

__xdata struct ao_task ao_cmd_task;

__code struct ao_cmds	ao_base_cmds[] = {
	{ help,		"?\0Help" },
#if HAS_TASK_INFO
	{ ao_task_info,	"T\0Tasks" },
#endif
	{ echo,		"E <0 off, 1 on>\0Echo" },
	{ ao_reboot,	"r eboot\0Reboot" },
#if HAS_VERSION
	{ version,	"v\0Version" },
#endif
#if HAS_BOOT_LOADER
	{ ao_loader,	"X\0Switch to boot loader" },
#endif
	{ 0,	NULL },
};

void
ao_cmd_init(void)
{
	ao_cmd_register(&ao_base_cmds[0]);
	ao_add_task(&ao_cmd_task, ao_cmd, "cmd");
}
