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

static const int16_t altitude_table[2048] = {
#include "altitude.h"
};

int16_t
ao_pres_to_altitude(int16_t pres) __reentrant
{
	pres = pres >> 4;
	if (pres < 0) pres = 0;
	if (pres > 2047) pres = 2047;
	return altitude_table[pres];
}

int16_t
ao_altitude_to_pres(int16_t alt) __reentrant
{
	int16_t pres;

	for (pres = 0; pres < 2047; pres++)
		if (altitude_table[pres] <= alt)
			break;
	return pres << 4;
}

static __xdata uint8_t	ao_temp_mutex;

int16_t
ao_temp_to_dC(int16_t temp) __reentrant
{
	int16_t	ret;

	ao_mutex_get(&ao_temp_mutex);
	/* Output voltage at 0°C = 0.755V
	 * Coefficient = 0.00247V/°C
	 * Reference voltage = 1.25V
	 *
	 * temp = ((value / 32767) * 1.25 - 0.755) / 0.00247
	 *      = (value - 19791.268) / 32768 * 1.25 / 0.00247
	 *	≃ (value - 19791) * 1012 / 65536
	 */
	ret = ((temp - 19791) * 1012L) >> 16;
	ao_mutex_put(&ao_temp_mutex);
	return ret;
}
