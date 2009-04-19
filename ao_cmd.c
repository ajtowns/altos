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

#define LEX_ERROR	1
#define SYNTAX_ERROR	2
#define SUCCESS		0

static __xdata uint16_t lex_i;
static __xdata uint8_t	lex_c;
static __xdata uint8_t	lex_status;
static __xdata uint8_t	lex_echo;

#define CMD_LEN	32

static __xdata uint8_t	cmd_line[CMD_LEN];
static __xdata uint8_t	cmd_len;
static __xdata uint8_t	cmd_i;

void
putchar(char c)
{
	if (c == '\n')
		ao_usb_putchar('\r');
	ao_usb_putchar((uint8_t) c);
}

void
flush(void)
{
	ao_usb_flush();
}

char
getchar(void)
{
	return (char) ao_usb_getchar();
}

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
				put_string ("\n");
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

static void
lex(void)
{
	lex_c = '\n';
	if (cmd_i < cmd_len)
		lex_c = cmd_line[cmd_i++];
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
put16(uint16_t v)
{
	int8_t i;
	for (i = 3; i >= 0; i--)
		putnibble((v >> (i << 2)) & 0xf);
}

void
put8(uint8_t v)
{
	putnibble((v >> 4) & 0xf);
	putnibble(v & 0xf);
}

#define NUM_LEN 7

void
puti(int i)
{
	static uint8_t __xdata	num_buffer[NUM_LEN];
	uint8_t __xdata * __xdata num_ptr = num_buffer + NUM_LEN;
	uint8_t __xdata neg = 0;
	
	*--num_ptr = '\0';
	if (i < 0) {
		i = -i;
		neg = 1;
	}
	do {
		*--num_ptr = '0' + i % 10;
		i /= 10;
	} while (i);
	if (neg)
		*--num_ptr = '-';
	while (num_ptr != num_buffer)
		*--num_ptr = ' ';
	put_string(num_buffer);
}


static void
white(void)
{
	while (lex_c == ' ' || lex_c == '\t')
		lex();
}

static void
hex(void)
{
	__xdata uint8_t	r = LEX_ERROR;
	
	lex_i = 0;
	white();
	for(;;) {
		if ('0' <= lex_c && lex_c <= '9')
			lex_i = (lex_i << 4) | (lex_c - '0');
		else if ('a' <= lex_c && lex_c <= 'f')
			lex_i = (lex_i << 4) | (lex_c - 'a' + 10);
		else if ('A' <= lex_c && lex_c <= 'F')
			lex_i = (lex_i << 4) | (lex_c - 'A' + 10);
		else
			break;
		r = SUCCESS;
		lex();
	}
	if (r != SUCCESS)
		lex_status = r;
}

#if 0
static void
decimal(void)
{
	__xdata uint8_t	r = LEX_ERROR;
	
	lex_i = 0;
	white();
	for(;;) {
		if ('0' <= lex_c && lex_c <= '9')
			lex_i = (lex_i * 10 ) | (lex_c - '0');
		else
			break;
		r = SUCCESS;
		lex();
	}
	if (r != SUCCESS)
		lex_status = r;
}
#endif

static void
eol(void)
{
	while (lex_c != '\n')
		lex();
}

static void
adc_dump(void)
{
	__xdata struct ao_adc	packet;
	ao_adc_get(&packet);
	put_string("tick: ");
	puti(packet.tick);
	put_string(" accel: ");
	puti(packet.accel >> 4);
	put_string(" pres: ");
	puti(packet.pres >> 4);
	put_string(" temp: ");
	puti(packet.temp >> 4);
	put_string(" batt: ");
	puti(packet.v_batt >> 4);
	put_string(" drogue: ");
	puti(packet.sense_d >> 4);
	put_string(" main: ");
	puti(packet.sense_m >> 4);
	put_string("\n");
}

static void
dump(void)
{
	__xdata uint16_t c;
	__xdata uint8_t * __xdata start, * __xdata end;

	hex();
	start = (uint8_t __xdata *) lex_i;
	hex();
	end = (uint8_t __xdata *) lex_i;
	if (lex_status != SUCCESS)
		return;
	c = 0;
	while (start <= end) {
		if ((c & 7) == 0) {
			if (c)
				put_string("\n");
			put16((uint16_t) start);
		}
		putchar(' ');
		put8(*start);
		++c;
		start++;
	}
	put_string("\n");
}

static void
ee_dump(void)
{
	__xdata uint8_t	b;
	__xdata uint16_t block;
	__xdata uint8_t i;
	
	hex();
	block = lex_i;
	if (lex_status != SUCCESS)
		return;
	i = 0;
	do {
		if ((i & 7) == 0) {
			if (i)
				put_string("\n");
			put16((uint16_t) i);
		}
		putchar(' ');
		ao_ee_read(((uint32_t) block << 8) | i, &b, 1);
		put8(b);
		++i;
	} while (i != 0);
	put_string("\n");
}

static void
ee_store(void)
{
	__xdata uint16_t block;
	__xdata uint8_t i;
	__xdata uint16_t len;
	__xdata uint8_t b;
	__xdata uint32_t addr;

	hex();
	block = lex_i;
	hex();
	i = lex_i;
	addr = ((uint32_t) block << 8) | i;
	hex();
	len = lex_i;
	if (lex_status != SUCCESS)
		return;
	while (len--) {
		hex();
		if (lex_status != SUCCESS)
			return;
		b = lex_i;
		ao_ee_write(addr, &b, 1);
		addr++;
	}
	ao_ee_flush();	
}

static void
echo(void)
{
	hex();
	lex_echo = lex_i != 0;
}

static void
debug_enable(void)
{
	ao_dbg_debug_mode();
}

static void
debug_reset(void)
{
	ao_dbg_reset();
}

static void
debug_put(void)
{
	for (;;) {
		white ();
		if (lex_c == '\n')
			break;
		hex();
		if (lex_status != SUCCESS)
			break;
		ao_dbg_send_byte(lex_i);
	}
}

static void
debug_get(void)
{
	__xdata uint16_t count;
	__xdata uint16_t i;
	__xdata uint8_t byte;
	hex();
	if (lex_status != SUCCESS)
		return;
	count = lex_i;
	if (count > 256) {
		lex_status = SYNTAX_ERROR;
		return;
	}
	for (i = 0; i < count; i++) {
		if (i && (i & 7) == 0)
			put_string("\n");
		byte = ao_dbg_recv_byte();
		put8(byte);
		putchar(' ');
	}
	put_string("\n");
}

static uint8_t
getnibble(void)
{
	__xdata uint8_t	c;

	c = getchar();
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - ('a' - 10);
	if ('A' <= c && c <= 'F')
		return c - ('A' - 10);
	lex_status = LEX_ERROR;
	return 0;
}

static void
debug_input(void)
{
	__xdata uint16_t count;
	__xdata uint16_t addr;
	__xdata uint8_t b;
	__xdata uint8_t	i;

	hex();
	count = lex_i;
	hex();
	addr = lex_i;
	if (lex_status != SUCCESS)
		return;
	ao_dbg_start_transfer(addr);
	i = 0;
	while (count--) {
		if (!(i++ & 7))
			put_string("\n");
		b = ao_dbg_read_byte();
		put8(b);
	}
	ao_dbg_end_transfer();
	put_string("\n");
}

static void
debug_output(void)
{
	__xdata uint16_t count;
	__xdata uint16_t addr;
	__xdata uint8_t b;

	hex();
	count = lex_i;
	hex();
	addr = lex_i;
	if (lex_status != SUCCESS)
		return;
	ao_dbg_start_transfer(addr);
	while (count--) {
		b = getnibble() << 4;
		b |= getnibble();
		if (lex_status != SUCCESS)
			return;
		ao_dbg_write_byte(b);
	}
	ao_dbg_end_transfer();
}

static void
dump_log(void)
{
	__xdata uint8_t	more;

	for (more = ao_log_dump_first(); more; more = ao_log_dump_next()) {
		putchar(ao_log_dump.type);
		putchar(' ');
		put16(ao_log_dump.tick);
		putchar(' ');
		put16(ao_log_dump.u.anon.d0);
		putchar(' ');
		put16(ao_log_dump.u.anon.d1);
		putchar('\n');
	}
}

static void
send_serial(void)
{
	white();
	while (lex_c != '\n') {
		ao_serial_putchar(lex_c);
		lex();
	}
}

static const uint8_t help_txt[] = 
	"All numbers are in hex\n"
	"?                                  Print this message\n"
	"a                                  Display current ADC values\n"
	"d <start> <end>                    Dump memory\n"
	"e <block>                          Dump a block of EEPROM data\n"
	"w <block> <start> <len> <data> ... Write data to EEPROM\n"
	"l                                  Dump last flight log\n"
	"E <0 off, 1 on>                    Set command echo mode\n"
	"S<data>                            Send data to serial line\n"
	"T                                  Show task states\n"
        "\n"
        "Target debug commands:\n"
	"D                                  Enable debug mode\n"
	"R                                  Reset target\n"
        "P <byte> ...                       Put data to debug port\n"
	"G <count>                          Get data from debug port\n"
	"O <count> <addr>                   Output <count> bytes to target at <addr>\n"
	"I <count> <addr>                   Input <count> bytes to target at <addr>\n"
;

static void
help(void)
{
	put_string(help_txt);
}

static void
report(void)
{
	switch(lex_status) {
	case LEX_ERROR:
	case SYNTAX_ERROR:
		put_string("Syntax error\n");
		lex_status = 0;
		break;
	}
}

void
ao_cmd(void *parameters)
{
	__xdata uint8_t	c;
	(void) parameters;

	lex_echo = 1;
	for (;;) {
		readline();
		lex();
		white();
		c = lex_c;
		lex();
		switch (c) {
		case '?':
			help();
			break;
		case 'd':
			dump();
			break;
		case 'a':
			adc_dump();
			break;
		case 'e':
			ee_dump();
			break;
		case 'w':
			ee_store();
			break;
		case 'l':
			dump_log();
			break;
		case 'T':
			ao_task_info();
			break;
		case 'S':
			send_serial();
			break;
		case 'E':
			echo();
			break;
		case 'D':
			debug_enable();
			break;
		case 'R':
			debug_reset();
			break;
		case 'P':
			debug_put();
			break;
		case 'G':
			debug_get();
			break;
		case 'I':
			debug_input();
			break;
		case 'O':
			debug_output();
			break;
		case '\r':
		case '\n':
			break;
		default:
			lex_status = SYNTAX_ERROR;
			break;
		}
		report();
	}
		
}

__xdata struct ao_task ao_cmd_task;

void
ao_cmd_init(void)
{
	ao_add_task(&ao_cmd_task, ao_cmd, "cmd");
}
