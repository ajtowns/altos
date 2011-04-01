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

#define AO_NUM_STDIOS	(HAS_USB + PACKET_HAS_SLAVE + USE_SERIAL_STDIN)

static __xdata struct ao_stdio stdios[AO_NUM_STDIOS];
__data int8_t ao_cur_stdio;
__data int8_t ao_num_stdios;

void
putchar(char c)
{
	if (c == '\n')
		(*stdios[ao_cur_stdio].putchar)('\r');
	(*stdios[ao_cur_stdio].putchar)(c);
}

void
flush(void)
{
	if (stdios[ao_cur_stdio].flush)
		stdios[ao_cur_stdio].flush();
}

__xdata uint8_t ao_stdin_ready;

char
getchar(void) __reentrant __critical
{
	char c;
	int8_t stdio = ao_cur_stdio;

	for (;;) {
		c = stdios[stdio].pollchar();
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

void
ao_add_stdio(char (*pollchar)(void),
	     void (*putchar)(char),
	     void (*flush)(void)) __reentrant
{
	if (ao_num_stdios == AO_NUM_STDIOS)
		ao_panic(AO_PANIC_STDIO);
	stdios[ao_num_stdios].pollchar = pollchar;
	stdios[ao_num_stdios].putchar = putchar;
	stdios[ao_num_stdios].flush = flush;
	ao_num_stdios++;
}
