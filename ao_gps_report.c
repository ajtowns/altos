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
	static __xdata struct ao_log_record	gps_log;
	static __xdata struct ao_gps_data	gps_data;

	for (;;) {
		ao_sleep(&ao_gps_data);
		ao_mutex_get(&ao_gps_mutex);
		memcpy(&gps_data, &ao_gps_data, sizeof (struct ao_gps_data));
		ao_mutex_put(&ao_gps_mutex);

		gps_log.tick = ao_time();
		gps_log.type = AO_LOG_GPS_TIME;
		gps_log.u.gps_time.hour = gps_data.hour;
		gps_log.u.gps_time.minute = gps_data.minute;
		gps_log.u.gps_time.second = gps_data.second;
		gps_log.u.gps_time.flags = gps_data.flags;
		ao_log_data(&gps_log);
		gps_log.type = AO_LOG_GPS_LAT;
		gps_log.u.gps_latitude.degrees = gps_data.latitude.degrees;
		gps_log.u.gps_latitude.minutes = gps_data.latitude.minutes;
		gps_log.u.gps_latitude.minutes_fraction = gps_data.latitude.minutes_fraction;
		ao_log_data(&gps_log);
		gps_log.type = AO_LOG_GPS_LON;
		gps_log.u.gps_longitude.degrees = gps_data.longitude.degrees;
		gps_log.u.gps_longitude.minutes = gps_data.longitude.minutes;
		gps_log.u.gps_longitude.minutes_fraction = gps_data.longitude.minutes_fraction;
		ao_log_data(&gps_log);
		gps_log.type = AO_LOG_GPS_ALT;
		gps_log.u.gps_altitude.altitude = gps_data.altitude;
		gps_log.u.gps_altitude.unused = 0xffff;
		ao_log_data(&gps_log);
	}
}

__xdata struct ao_task ao_gps_report_task;

void
ao_gps_report_init(void)
{
	ao_add_task(&ao_gps_report_task, ao_gps_report, "gps_report");
}
