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

#if !defined(AO_CONVERT_TEST) && !defined(AO_FLIGHT_TEST)
#include "ao.h"
#endif

#ifndef AO_CONST_ATTRIB
#define AO_CONST_ATTRIB
#endif

static const alt_t altitude_table[] AO_CONST_ATTRIB = {
#include "altitude-pa.h"
};

#ifndef FETCH_ALT
#define FETCH_ALT(o)	altitude_table[o]
#endif

#define ALT_SCALE	(1 << ALT_SHIFT)
#define ALT_MASK	(ALT_SCALE - 1)

alt_t
ao_pa_to_altitude(int32_t pa)
{
	int16_t	o;
	int16_t	part;
	int32_t low, high;

	if (pa < 0)
		pa = 0;
	if (pa > 120000L)
		pa = 120000L;
	o = pa >> ALT_SHIFT;
	part = pa & ALT_MASK;

	low = (int32_t) FETCH_ALT(o) * (ALT_SCALE - part);
	high = (int32_t) FETCH_ALT(o+1) * part + (ALT_SCALE >> 1);
	return (low + high) >> ALT_SHIFT;
}

#ifdef AO_CONVERT_TEST
int32_t
ao_altitude_to_pa(int32_t alt)
{
	int32_t 	span, sub_span;
	uint16_t	l, h, m;
	int32_t 	pa;

	l = 0;
	h = NALT - 1;
	while ((h - l) != 1) {
		m = (l + h) >> 1;
		if (altitude_table[m] < alt)
			h = m;
		else
			l = m;
	}
	span = altitude_table[l] - altitude_table[h];
	sub_span = altitude_table[l] - alt;
	pa = ((((int32_t) l * (span - sub_span) + (int32_t) h * sub_span) << ALT_SHIFT) + (span >> 1)) / span;
	if (pa > 120000)
		pa = 120000;
	if (pa < 0)
		pa = 0;
	return pa;
}
#endif
