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

/*
 * Basic I/O functions to support SDCC stdio package
 */

#ifndef USE_SERIAL_0_STDIN
#define USE_SERIAL_0_STDIN	0
#endif
#ifndef USE_SERIAL_1_STDIN
#define USE_SERIAL_1_STDIN	0
#endif
#ifndef USE_SERIAL_2_STDIN
#define USE_SERIAL_2_STDIN	0
#endif
#ifndef USE_SERIAL_3_STDIN
#define USE_SERIAL_3_STDIN	0
#endif
#ifndef USE_SERIAL_4_STDIN
#define USE_SERIAL_4_STDIN	0
#endif
#ifndef USE_SERIAL_5_STDIN
#define USE_SERIAL_5_STDIN	0
#endif
#ifndef USE_SERIAL_6_STDIN
#define USE_SERIAL_6_STDIN	0
#endif
#ifndef USE_SERIAL_7_STDIN
#define USE_SERIAL_7_STDIN	0
#endif
#ifndef USE_SERIAL_8_STDIN
#define USE_SERIAL_8_STDIN	0
#endif
#ifndef USE_SERIAL_9_STDIN
#define USE_SERIAL_9_STDIN	0
#endif

#define USE_SERIAL_STDIN (USE_SERIAL_0_STDIN +	\
			  USE_SERIAL_1_STDIN +	\
			  USE_SERIAL_2_STDIN +	\
			  USE_SERIAL_3_STDIN +	\
			  USE_SERIAL_4_STDIN +	\
			  USE_SERIAL_5_STDIN +	\
			  USE_SERIAL_6_STDIN +	\
			  USE_SERIAL_7_STDIN +	\
			  USE_SERIAL_8_STDIN +	\
			  USE_SERIAL_9_STDIN)

#define AO_NUM_STDIOS	(HAS_USB + PACKET_HAS_SLAVE + USE_SERIAL_STDIN)

__xdata struct ao_stdio ao_stdios[AO_NUM_STDIOS];
__pdata int8_t ao_cur_stdio;
__pdata int8_t ao_num_stdios;

void
putchar(char c)
{
#if LOW_LEVEL_DEBUG
	if (!ao_cur_task) {
		extern void ao_debug_out(char c);
		if (c == '\n')
			ao_debug_out('\r');
		ao_debug_out(c);
		return;
	}
#endif
	if (c == '\n')
		(*ao_stdios[ao_cur_stdio].putchar)('\r');
	(*ao_stdios[ao_cur_stdio].putchar)(c);
}

void
flush(void)
{
	if (ao_stdios[ao_cur_stdio].flush)
		ao_stdios[ao_cur_stdio].flush();
}

__xdata uint8_t ao_stdin_ready;

char
getchar(void) __reentrant __critical
{
	char c;
	int8_t stdio = ao_cur_stdio;

	for (;;) {
		c = ao_stdios[stdio].pollchar();
		if (c != AO_READ_AGAIN)
			break;
		if (++stdio == ao_num_stdios)
			stdio = 0;
		if (stdio == ao_cur_stdio)
			ao_sleep(&ao_stdin_ready);
	}
	ao_cur_stdio = stdio;
	return c;
}

uint8_t
ao_echo(void)
{
	return ao_stdios[ao_cur_stdio].echo;
}

int8_t
ao_add_stdio(char (*pollchar)(void),
	     void (*putchar)(char),
	     void (*flush)(void)) __reentrant
{
	if (ao_num_stdios == AO_NUM_STDIOS)
		ao_panic(AO_PANIC_STDIO);
	ao_stdios[ao_num_stdios].pollchar = pollchar;
	ao_stdios[ao_num_stdios].putchar = putchar;
	ao_stdios[ao_num_stdios].flush = flush;
	ao_stdios[ao_num_stdios].echo = 1;
	return ao_num_stdios++;
}
