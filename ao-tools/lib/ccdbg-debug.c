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

#include "ccdbg.h"
#include <stdarg.h>

int
ccdbg_level = 0;

void
ccdbg_add_debug(int level)
{
	ccdbg_level |= level;
}

void
ccdbg_clear_debug(int level)
{
	ccdbg_level &= ~level;
}

static int initialized;

void
ccdbg_debug(int level, char *format, ...)
{
	va_list	ap;

	if (!initialized) {
		char *level;
		initialized = 1;
		level = getenv("CCDEBUG");
		if (level)
			ccdbg_level |= strtoul(level, NULL, 0);
	}
	if (ccdbg_level & level) {
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
	}
}

void
ccdbg_flush(int level)
{
	if (ccdbg_level & level)
		fflush(stdout);
}
