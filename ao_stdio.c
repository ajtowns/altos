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
