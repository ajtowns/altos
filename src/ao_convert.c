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

#if !defined(AO_CONVERT_TEST) && !defined(AO_FLIGHT_TEST)
#include "ao.h"
#endif

static const int16_t altitude_table[] = {
#include "altitude.h"
};

#define ALT_FRAC_SCALE	(1 << ALT_FRAC_BITS)
#define ALT_FRAC_MASK	(ALT_FRAC_SCALE - 1)

int16_t
ao_pres_to_altitude(int16_t pres) __reentrant
{
	uint8_t	o;
	int16_t	part;

	if (pres < 0)
		pres = 0;
	o = pres >> ALT_FRAC_BITS;
	part = pres & ALT_FRAC_MASK;

	return ((int32_t) altitude_table[o] * (ALT_FRAC_SCALE - part) +
		(int32_t) altitude_table[o+1] * part + (ALT_FRAC_SCALE >> 1)) >> ALT_FRAC_BITS;
}

int16_t
ao_altitude_to_pres(int16_t alt) __reentrant
{
	int16_t span, sub_span;
	uint8_t	l, h, m;
	int32_t pres;

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
	pres = ((((int32_t) l * (span - sub_span) + (int32_t) h * sub_span) << ALT_FRAC_BITS) + (span >> 1)) / span;
	if (pres > 32767)
		pres = 32767;
	if (pres < 0)
		pres = 0;
	return (int16_t) pres;
}

int16_t
ao_temp_to_dC(int16_t temp) __reentrant
{
	int16_t	ret;

	/* Output voltage at 0°C = 0.755V
	 * Coefficient = 0.00247V/°C
	 * Reference voltage = 1.25V
	 *
	 * temp = ((value / 32767) * 1.25 - 0.755) / 0.00247
	 *      = (value - 19791.268) / 32768 * 1.25 / 0.00247
	 *	≃ (value - 19791) * 1012 / 65536
	 */
	ret = ((temp - 19791) * 1012L) >> 16;
	return ret;
}
