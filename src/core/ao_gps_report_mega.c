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
#include "ao_log.h"

void
ao_gps_report_mega(void)
{
	static __xdata struct ao_log_mega		gps_log;
	static __xdata struct ao_telemetry_location	gps_data;
	static __xdata struct ao_telemetry_satellite	gps_tracking_data;
	uint8_t	new;
	uint8_t	c, n, i;

	for (;;) {
		while (!(new = ao_gps_new))
			ao_sleep(&ao_gps_new);
		ao_mutex_get(&ao_gps_mutex);
		if (new & AO_GPS_NEW_DATA)
			ao_xmemcpy(&gps_data, &ao_gps_data, sizeof (ao_gps_data));
		if (new & AO_GPS_NEW_TRACKING)
			ao_xmemcpy(&gps_tracking_data, &ao_gps_tracking_data, sizeof (ao_gps_tracking_data));
		ao_gps_new = 0;
		ao_mutex_put(&ao_gps_mutex);

		if ((new & AO_GPS_NEW_DATA) && (gps_data.flags & AO_GPS_VALID)) {
			gps_log.tick = ao_gps_tick;
			gps_log.type = AO_LOG_GPS_TIME;
			gps_log.u.gps.latitude = gps_data.latitude;
			gps_log.u.gps.longitude = gps_data.longitude;
			gps_log.u.gps.altitude = gps_data.altitude;

			gps_log.u.gps.hour = gps_data.hour;
			gps_log.u.gps.minute = gps_data.minute;
			gps_log.u.gps.second = gps_data.second;
			gps_log.u.gps.flags = gps_data.flags;
			gps_log.u.gps.year = gps_data.year;
			gps_log.u.gps.month = gps_data.month;
			gps_log.u.gps.day = gps_data.day;
			gps_log.u.gps.course = gps_data.course;
			gps_log.u.gps.ground_speed = gps_data.ground_speed;
			gps_log.u.gps.climb_rate = gps_data.climb_rate;
			gps_log.u.gps.pdop = gps_data.pdop;
			gps_log.u.gps.hdop = gps_data.hdop;
			gps_log.u.gps.vdop = gps_data.vdop;
			gps_log.u.gps.mode = gps_data.mode;
			ao_log_mega(&gps_log);
		}
		if ((new & AO_GPS_NEW_TRACKING) && (n = gps_tracking_data.channels) != 0) {
			gps_log.tick = ao_gps_tick;
			gps_log.type = AO_LOG_GPS_SAT;
			i = 0;
			for (c = 0; c < n; c++)
				if ((gps_log.u.gps_sat.sats[i].svid = gps_tracking_data.sats[c].svid))
				{
					gps_log.u.gps_sat.sats[i].c_n = gps_tracking_data.sats[c].c_n_1;
					i++;
				}
			gps_log.u.gps_sat.channels = i;
			ao_log_mega(&gps_log);
		}
	}
}

__xdata struct ao_task ao_gps_report_mega_task;

void
ao_gps_report_mega_init(void)
{
	ao_add_task(&ao_gps_report_mega_task,
		    ao_gps_report_mega,
		    "gps_report");
}
