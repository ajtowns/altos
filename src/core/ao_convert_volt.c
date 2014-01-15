/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#define scale(v,p,m)	((int32_t) (v) * (AO_ADC_REFERENCE_DV * ((p) + (m))) / (AO_ADC_MAX * (m)))

int16_t
ao_battery_decivolt(int16_t adc)
{
	return scale(adc, AO_BATTERY_DIV_PLUS, AO_BATTERY_DIV_MINUS);
}

int16_t
ao_ignite_decivolt(int16_t adc)
{
	return scale(adc, AO_IGNITE_DIV_PLUS, AO_IGNITE_DIV_MINUS);
}

