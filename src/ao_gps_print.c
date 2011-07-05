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
#include "ao_telem.h"

void
ao_gps_print(__xdata struct ao_gps_orig *gps_data) __reentrant
{
	char	state;

	if (gps_data->flags & AO_GPS_VALID)
		state = AO_TELEM_GPS_STATE_LOCKED;
	else if (gps_data->flags & AO_GPS_RUNNING)
		state = AO_TELEM_GPS_STATE_UNLOCKED;
	else
		state = AO_TELEM_GPS_STATE_ERROR;
	printf(AO_TELEM_GPS_STATE " %c "
	       AO_TELEM_GPS_NUM_SAT " %d ",
	       state,
	       (gps_data->flags & AO_GPS_NUM_SAT_MASK) >> AO_GPS_NUM_SAT_SHIFT);
	if (!(gps_data->flags & AO_GPS_VALID))
		return;
	printf(AO_TELEM_GPS_LATITUDE " %ld "
	       AO_TELEM_GPS_LONGITUDE " %ld "
	       AO_TELEM_GPS_ALTITUDE " %d ",
	       gps_data->latitude,
	       gps_data->longitude,
	       gps_data->altitude);

	if (gps_data->flags & AO_GPS_DATE_VALID)
		printf(AO_TELEM_GPS_YEAR " %d "
		       AO_TELEM_GPS_MONTH " %d "
		       AO_TELEM_GPS_DAY " %d ",
		       gps_data->year,
		       gps_data->month,
		       gps_data->day);

	printf(AO_TELEM_GPS_HOUR " %d "
	       AO_TELEM_GPS_MINUTE " %d "
	       AO_TELEM_GPS_SECOND " %d ",
	       gps_data->hour,
	       gps_data->minute,
	       gps_data->second);

	printf(AO_TELEM_GPS_HDOP " %d ",
	       gps_data->hdop * 2);

	if (gps_data->flags & AO_GPS_COURSE_VALID) {
		printf(AO_TELEM_GPS_HERROR " %d "
		       AO_TELEM_GPS_VERROR " %d "
		       AO_TELEM_GPS_VERTICAL_SPEED " %d "
		       AO_TELEM_GPS_HORIZONTAL_SPEED " %d "
		       AO_TELEM_GPS_COURSE " %d ",
		       gps_data->h_error,
		       gps_data->v_error,
		       gps_data->climb_rate,
		       gps_data->ground_speed,
		       (int) gps_data->course * 2);
	}
}

void
ao_gps_tracking_print(__xdata struct ao_gps_tracking_orig *gps_tracking_data) __reentrant
{
	uint8_t	c, n, v;
	__xdata struct ao_gps_sat_orig	*sat;

	n = gps_tracking_data->channels;
	if (n == 0)
		return;

	sat = gps_tracking_data->sats;
	v = 0;
	for (c = 0; c < n; c++) {
		if (sat->svid)
			v++;
		sat++;
	}

	printf (AO_TELEM_SAT_NUM " %d ",
		v);

	sat = gps_tracking_data->sats;
	v = 0;
	for (c = 0; c < n; c++) {
		if (sat->svid) {
			printf (AO_TELEM_SAT_SVID "%d %d "
				AO_TELEM_SAT_C_N_0 "%d %d ",
				v, sat->svid,
				v, sat->c_n_1);
			v++;
		}
		sat++;
	}
}
