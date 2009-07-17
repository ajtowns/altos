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

#ifndef AO_GPS_TEST
#include "ao.h"
#endif

struct ao_gps_split {
	uint8_t positive;
	uint8_t	degrees;
	uint8_t	minutes;
	uint16_t minutes_fraction;
};

static void
ao_gps_split(int32_t v, __xdata struct ao_gps_split *split) __reentrant
{
	uint32_t minutes_e7;

	split->positive = 1;
	if (v < 0) {
		v = -v;
		split->positive = 0;
	}
	split->degrees = v / 10000000;
	minutes_e7 = (v % 10000000) * 60;
	split->minutes = minutes_e7 / 10000000;
	split->minutes_fraction = (minutes_e7 % 10000000) / 1000;
}

void
ao_gps_print(__xdata struct ao_gps_data *gps_data) __reentrant
{
	printf("GPS %2d sat",
	       (gps_data->flags & AO_GPS_NUM_SAT_MASK) >> AO_GPS_NUM_SAT_SHIFT);
	if (gps_data->flags & AO_GPS_VALID) {
		static __xdata struct ao_gps_split	lat, lon;
		int16_t climb;
		uint8_t climb_sign;

		ao_gps_split(gps_data->latitude, &lat);
		ao_gps_split(gps_data->longitude, &lon);
		printf(" %2d:%02d:%02d",
		       gps_data->hour,
		       gps_data->minute,
		       gps_data->second);
		printf(" %2d°%02d.%04d'%c %2d°%02d.%04d'%c %5dm",
		       lat.degrees,
		       lat.minutes,
		       lat.minutes_fraction,
		       lat.positive ? 'N' : 'S',
		       lon.degrees,
		       lon.minutes,
		       lon.minutes_fraction,
		       lon.positive ? 'E' : 'W',
		       gps_data->altitude);
		if (gps_data->climb_rate >= 0) {
			climb_sign = ' ';
			climb = gps_data->climb_rate;
		} else {
			climb_sign = '-';
			climb = -gps_data->climb_rate;
		}
		printf(" %5u.%02dm/s(H) %d° %c%5d.%02dm/s(V)",
		       gps_data->ground_speed / 100,
		       gps_data->ground_speed % 100,
		       gps_data->course * 2,
		       climb_sign,
		       climb / 100,
		       climb % 100);
		printf(" %d.%d(hdop) %5d(herr) %5d(verr)\n",
		       gps_data->hdop,
		       gps_data->h_error,
		       gps_data->v_error);
	} else {
		printf(" unlocked\n");
	}
}
