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
ao_gps_report_metrum(void)
{
	static __xdata struct ao_log_metrum		gps_log;
	static __xdata struct ao_telemetry_location	gps_data;
	uint8_t	date_reported = 0;

	for (;;) {
		ao_sleep(&ao_gps_data);
		ao_mutex_get(&ao_gps_mutex);
		ao_xmemcpy(&gps_data, &ao_gps_data, sizeof (ao_gps_data));
		ao_mutex_put(&ao_gps_mutex);

		if (!(gps_data.flags & AO_GPS_VALID))
			continue;

		gps_log.tick = ao_gps_tick;
		gps_log.type = AO_LOG_GPS_POS;
		gps_log.u.gps.latitude = gps_data.latitude;
		gps_log.u.gps.longitude = gps_data.longitude;
		gps_log.u.gps.altitude = gps_data.altitude;
		ao_log_metrum(&gps_log);

		gps_log.type = AO_LOG_GPS_TIME;
		gps_log.u.gps_time.hour = gps_data.hour;
		gps_log.u.gps_time.minute = gps_data.minute;
		gps_log.u.gps_time.second = gps_data.second;
		gps_log.u.gps_time.flags = gps_data.flags;
		gps_log.u.gps_time.year = gps_data.year;
		gps_log.u.gps_time.month = gps_data.month;
		gps_log.u.gps_time.day = gps_data.day;
		ao_log_metrum(&gps_log);
	}
}

void
ao_gps_tracking_report_metrum(void)
{
	static __xdata struct ao_log_metrum		gps_log;
	static __xdata struct ao_telemetry_satellite	gps_tracking_data;
	uint8_t	c, n, i, p, valid, packets;
	uint8_t svid;

	for (;;) {
		ao_sleep(&ao_gps_tracking_data);
		ao_mutex_get(&ao_gps_mutex);
		ao_xmemcpy(&gps_tracking_data, &ao_gps_tracking_data, sizeof (ao_gps_tracking_data));
		ao_mutex_put(&ao_gps_mutex);

		if (!(n = gps_tracking_data.channels))
			continue;

		gps_log.type = AO_LOG_GPS_SAT;
		gps_log.tick = ao_gps_tick;
		i = 0;
		for (c = 0; c < n; c++) {
			svid = gps_tracking_data.sats[c].svid;
			if (svid != 0) {
				if (i == 4) {
					gps_log.u.gps_sat.channels = i;
					gps_log.u.gps_sat.more = 1;
					ao_log_metrum(&gps_log);
					i = 0;
				}
				gps_log.u.gps_sat.sats[i].svid = svid;
				gps_log.u.gps_sat.sats[i].c_n = gps_tracking_data.sats[c].c_n_1;
				i++;
			}
		}
		if (i) {
			gps_log.u.gps_sat.channels = i;
			gps_log.u.gps_sat.more = 0;
			ao_log_metrum(&gps_log);
		}
	}
}

__xdata struct ao_task ao_gps_report_metrum_task;
__xdata struct ao_task ao_gps_tracking_report_metrum_task;

void
ao_gps_report_metrum_init(void)
{
	ao_add_task(&ao_gps_report_metrum_task,
		    ao_gps_report_metrum,
		    "gps_report");
	ao_add_task(&ao_gps_tracking_report_metrum_task,
		    ao_gps_tracking_report_metrum,
		    "gps_tracking_report");
}
