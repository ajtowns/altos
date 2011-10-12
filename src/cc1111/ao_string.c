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

void
_ao_xmemcpy(__xdata uint8_t *dst, __xdata uint8_t *src, uint8_t count)
{
	while (count--)
		*dst++ = *src++;
}

void
_ao_xmemset(__xdata uint8_t *dst, uint8_t v, uint8_t count)
{
	while (count--)
		*dst++ = v;
}

int8_t
_ao_xmemcmp(__xdata uint8_t *a, __xdata uint8_t *b, uint8_t count)
{
	while (count--) {
		int8_t	d = *a++ - *b++;
		if (d)
			return d;
	}
	return 0;
}
