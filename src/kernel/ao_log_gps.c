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
#include <ao_log.h>
#include <ao_log_gps.h>
#include <ao_data.h>
#include <ao_flight.h>
#include <ao_distance.h>
#include <ao_tracker.h>

static __xdata struct ao_log_gps log;

__code uint8_t ao_log_format = AO_LOG_FORMAT_TELEGPS;

static uint8_t
ao_log_csum(__xdata uint8_t *b) __reentrant
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_gps); i++)
		sum += *b++;
	return -sum;
}

uint8_t
ao_log_gps(__xdata struct ao_log_gps *log) __reentrant
{
	uint8_t wrote = 0;
	/* set checksum */
	log->csum = 0;
	log->csum = ao_log_csum((__xdata uint8_t *) log);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 log,
					 sizeof (struct ao_log_gps));
			ao_log_current_pos += sizeof (struct ao_log_gps);
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

void
ao_log_gps_flight(void)
{
	log.type = AO_LOG_FLIGHT;
	log.tick = ao_time();
	log.u.flight.flight = ao_flight_number;
	ao_log_gps(&log);
}

void
ao_log_gps_data(uint16_t tick, struct ao_telemetry_location *gps_data)
{
	log.tick = tick;
	log.type = AO_LOG_GPS_TIME;
	log.u.gps.latitude = gps_data->latitude;
	log.u.gps.longitude = gps_data->longitude;
	log.u.gps.altitude = gps_data->altitude;

	log.u.gps.hour = gps_data->hour;
	log.u.gps.minute = gps_data->minute;
	log.u.gps.second = gps_data->second;
	log.u.gps.flags = gps_data->flags;
	log.u.gps.year = gps_data->year;
	log.u.gps.month = gps_data->month;
	log.u.gps.day = gps_data->day;
	log.u.gps.course = gps_data->course;
	log.u.gps.ground_speed = gps_data->ground_speed;
	log.u.gps.climb_rate = gps_data->climb_rate;
	log.u.gps.pdop = gps_data->pdop;
	log.u.gps.hdop = gps_data->hdop;
	log.u.gps.vdop = gps_data->vdop;
	log.u.gps.mode = gps_data->mode;
	ao_log_gps(&log);
}

void
ao_log_gps_tracking(uint16_t tick, struct ao_telemetry_satellite *gps_tracking_data)
{
	uint8_t c, n, i;

	log.tick = tick;
	log.type = AO_LOG_GPS_SAT;
	i = 0;
	n = gps_tracking_data->channels;
	for (c = 0; c < n; c++)
		if ((log.u.gps_sat.sats[i].svid = gps_tracking_data->sats[c].svid))
		{
			log.u.gps_sat.sats[i].c_n = gps_tracking_data->sats[c].c_n_1;
			i++;
			if (i >= 12)
				break;
		}
	log.u.gps_sat.channels = i;
	ao_log_gps(&log);
}

static uint8_t
ao_log_dump_check_data(void)
{
	if (ao_log_csum((uint8_t *) &log) != 0)
		return 0;
	return 1;
}

uint16_t
ao_log_flight(uint8_t slot)
{
	if (!ao_storage_read(ao_log_pos(slot),
			     &log,
			     sizeof (struct ao_log_gps)))
		return 0;

	if (ao_log_dump_check_data() && log.type == AO_LOG_FLIGHT)
		return log.u.flight.flight;
	return 0;
}
