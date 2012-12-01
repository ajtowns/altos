/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include "sky_flash.h"

static int	dbg_input;
static int	dbg_newline = 1;

int
skytraq_millis(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void
skytraq_dbg_time(void)
{
	int	delta = skytraq_millis() - skytraq_open_time;

	if (!skytraq_verbose)
		return;
	printf ("%4d.%03d ", delta / 1000, delta % 1000);
}

void
skytraq_dbg_newline(void)
{
	if (!skytraq_verbose)
		return;
	if (!dbg_newline) {
		putchar('\n');
		dbg_newline = 1;
	}
}

static void
skytraq_dbg_set(int input)
{
	if (!skytraq_verbose)
		return;
	if (input != dbg_input) {
		skytraq_dbg_newline();
		if (input)
			putchar('\t');
		dbg_input = input;
	}
}

void
skytraq_dbg_char(int input, char c)
{
	if (!skytraq_verbose)
		return;
	skytraq_dbg_set(input);
	if (dbg_newline)
		skytraq_dbg_time();
	if (c < ' '  || c > '~')
		printf ("\\%02x", (unsigned char) c);
	else
		putchar(c);
	dbg_newline = 0;
	if (c == '\n')
		dbg_input = 2;
	fflush(stdout);
}

void
skytraq_dbg_buf(int input, const char *buf, int len)
{
	if (!skytraq_verbose)
		return;
	while (len--)
		skytraq_dbg_char(input, *buf++);
}

void
skytraq_dbg_printf(int input, const char *fmt, ...)
{
	va_list ap;

	if (!skytraq_verbose)
		return;
	skytraq_dbg_set(input);
	if (dbg_newline)
		skytraq_dbg_time();
	va_start (ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	dbg_newline = 0;
}
