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

void
ao_gps_print(__xdata struct ao_gps_data *gps_data) __reentrant
{
	if (gps_data->flags & AO_GPS_VALID) {
		printf("GPS %2d:%02d:%02d %2d°%2d.%04d'%c %2d°%2d.%04d'%c %5dm %2d sat\n",
		       gps_data->hour,
		       gps_data->minute,
		       gps_data->second,
		       gps_data->latitude.degrees,
		       gps_data->latitude.minutes,
		       gps_data->latitude.minutes_fraction,
		       (gps_data->flags & AO_GPS_LATITUDE_MASK) == AO_GPS_LATITUDE_NORTH ?
		       'N' : 'S',
		       gps_data->longitude.degrees,
		       gps_data->longitude.minutes,
		       gps_data->longitude.minutes_fraction,
		       (gps_data->flags & AO_GPS_LONGITUDE_MASK) == AO_GPS_LONGITUDE_WEST ?
		       'W' : 'E',
		       gps_data->altitude,
		       (gps_data->flags & AO_GPS_NUM_SAT_MASK) >> AO_GPS_NUM_SAT_SHIFT);
	} else {
		printf("GPS %2d sat\n",
		       (gps_data->flags & AO_GPS_NUM_SAT_MASK) >> AO_GPS_NUM_SAT_SHIFT);;
	}
}

