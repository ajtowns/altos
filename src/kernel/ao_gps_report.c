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
ao_gps_report(void)
{
	static __xdata struct ao_log_record		gps_log;
	static __xdata struct ao_telemetry_location	gps_data;
	static __xdata struct ao_telemetry_satellite	gps_tracking_data;
	uint8_t	date_reported = 0;
	uint8_t new;

	for (;;) {
		while ((new = ao_gps_new) == 0)
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
			gps_log.u.gps_time.hour = gps_data.hour;
			gps_log.u.gps_time.minute = gps_data.minute;
			gps_log.u.gps_time.second = gps_data.second;
			gps_log.u.gps_time.flags = gps_data.flags;
			ao_log_data(&gps_log);
			gps_log.type = AO_LOG_GPS_LAT;
			gps_log.u.gps_latitude = gps_data.latitude;
			ao_log_data(&gps_log);
			gps_log.type = AO_LOG_GPS_LON;
			gps_log.u.gps_longitude = gps_data.longitude;
			ao_log_data(&gps_log);
			gps_log.type = AO_LOG_GPS_ALT;
			gps_log.u.gps_altitude.altitude = gps_data.altitude;
			gps_log.u.gps_altitude.unused = 0xffff;
			ao_log_data(&gps_log);
			if (!date_reported && (gps_data.flags & AO_GPS_DATE_VALID)) {
				gps_log.type = AO_LOG_GPS_DATE;
				gps_log.u.gps_date.year = gps_data.year;
				gps_log.u.gps_date.month = gps_data.month;
				gps_log.u.gps_date.day = gps_data.day;
				gps_log.u.gps_date.extra = 0;
				date_reported = ao_log_data(&gps_log);
			}
		}
		if (new & AO_GPS_NEW_TRACKING) {
			uint8_t c, n;

			if ((n = gps_tracking_data.channels) != 0) {
				gps_log.type = AO_LOG_GPS_SAT;
				for (c = 0; c < n; c++)
					if ((gps_log.u.gps_sat.svid = gps_tracking_data.sats[c].svid))
					{
						gps_log.u.gps_sat.c_n = gps_tracking_data.sats[c].c_n_1;
						ao_log_data(&gps_log);
					}
			}
		}
	}
}

__xdata struct ao_task ao_gps_report_task;

void
ao_gps_report_init(void)
{
	ao_add_task(&ao_gps_report_task, ao_gps_report, "gps_report");
}
