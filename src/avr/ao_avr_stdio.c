/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

int
stdio_put(char c, FILE *stream)
{
	putchar(c);
	return 0;
}

int
stdio_get(FILE *stream)
{
	return (int) getchar() & 0xff;
}

static FILE mystdout = FDEV_SETUP_STREAM(stdio_put, NULL, _FDEV_SETUP_WRITE);

static FILE mystdin = FDEV_SETUP_STREAM(NULL, stdio_get, _FDEV_SETUP_READ);

void
ao_avr_stdio_init(void)
{
	stdout = &mystdout;
	stdin = &mystdin;
}
