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
_ao_xmemcpy(__xdata void *dst, __xdata void *src, uint16_t count)
{
	while (count--) {
		*(__xdata uint8_t *) dst = *(__xdata uint8_t *) src;
		dst = (__xdata uint8_t *) dst + 1;
		src = (__xdata uint8_t *) src + 1;
	}
}

void
_ao_xmemset(__xdata void *dst, uint8_t v, uint16_t count)
{
	while (count--) {
		*(__xdata uint8_t *) dst = v;
		dst = (__xdata uint8_t *) dst + 1;
	}
}

int8_t
_ao_xmemcmp(__xdata void *a, __xdata void *b, uint16_t count)
{
	while (count--) {
		int8_t	d = *(__xdata int8_t *) a - *(__xdata int8_t *) b;
		if (d)
			return d;
		a = (__xdata int8_t *) a + 1;
		b = (__xdata int8_t *) b + 1;
	}
	return 0;
}
