/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

static __xdata uint8_t	ao_log_mutex;
static __xdata struct ao_log_record log;

static uint8_t
ao_log_csum(__xdata uint8_t *b) __reentrant
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_record); i++)
		sum += *b++;
	return -sum;
}

uint8_t
ao_log_data(__xdata struct ao_log_record *log) __reentrant
{
	uint8_t wrote = 0;
	/* set checksum */
	log->csum = 0;
	log->csum = ao_log_csum((__xdata uint8_t *) log);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_running) {
			if (ao_log_current_pos >= ao_log_end_pos) {
				ao_log_stop();
			} else {
				wrote = 1;
				ao_storage_write(ao_log_current_pos,
						 log,
						 sizeof (struct ao_log_record));
				ao_log_current_pos += sizeof (struct ao_log_record);
			}
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

static uint8_t
ao_log_dump_check_data(void)
{
	if (ao_log_csum((uint8_t *) &log) != 0)
		return 0;
	return 1;
}

static __xdata uint8_t	ao_log_adc_pos;

/* a hack to make sure that ao_log_records fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_record))] ;

#define AO_SENSOR_INTERVAL_RIDE		50
#define AO_OTHER_INTERVAL		32

void
ao_log(void)
{
	uint16_t	next_sensor, next_other;

	ao_storage_setup();

	ao_log_scan();

	while (!ao_log_running)
		ao_sleep(&ao_log_running);

	log.type = AO_LOG_FLIGHT;
	log.tick = ao_sample_tick;
#if HAS_ACCEL
	log.u.flight.ground_accel = ao_ground_accel;
#endif
	log.u.flight.flight = ao_flight_number;
	ao_log_data(&log);

	/* Write the whole contents of the ring to the log
	 * when starting up.
	 */
	ao_log_adc_pos = ao_adc_ring_next(ao_sample_adc);
	next_other = next_sensor = ao_adc_ring[ao_log_adc_pos].tick;
	for (;;) {
		/* Write samples to EEPROM */
		while (ao_log_adc_pos != ao_sample_adc) {
			log.tick = ao_adc_ring[ao_log_adc_pos].tick;
			if ((int16_t) (log.tick - next_sensor) >= 0) {
				log.type = AO_LOG_SENSOR;
				log.u.sensor.accel = ao_adc_ring[ao_log_adc_pos].accel;
				log.u.sensor.pres = ao_adc_ring[ao_log_adc_pos].pres;
				ao_log_data(&log);
				next_sensor = log.tick + AO_SENSOR_INTERVAL_RIDE;
			}
			if ((int16_t) (log.tick - next_other) >= 0) {
				log.type = AO_LOG_TEMP_VOLT;
				log.u.temp_volt.temp = ao_adc_ring[ao_log_adc_pos].temp;
				log.u.temp_volt.v_batt = ao_adc_ring[ao_log_adc_pos].v_batt;
				ao_log_data(&log);
				log.type = AO_LOG_DEPLOY;
				log.u.deploy.drogue = ao_adc_ring[ao_log_adc_pos].sense_d;
				log.u.deploy.main = ao_adc_ring[ao_log_adc_pos].sense_m;
				ao_log_data(&log);
				next_other = log.tick + AO_OTHER_INTERVAL;
			}
			ao_log_adc_pos = ao_adc_ring_next(ao_log_adc_pos);
		}

		/* Wait for a while */
		ao_delay(AO_MS_TO_TICKS(100));

		/* Stop logging when told to */
		while (!ao_log_running)
			ao_sleep(&ao_log_running);
	}
}

uint16_t
ao_log_flight(uint8_t slot)
{
	if (!ao_storage_read(ao_log_pos(slot),
			     &log,
			     sizeof (struct ao_log_record)))
		return 0;

	if (ao_log_dump_check_data() && log.type == AO_LOG_FLIGHT)
		return log.u.flight.flight;
	return 0;
}

static void
ao_gps_report(void)
{
	static __xdata struct ao_log_record	gps_log;
	static __xdata struct ao_gps_data	gps_data;
	uint8_t	date_reported = 0;

	for (;;) {
		ao_sleep(&ao_gps_data);
		ao_mutex_get(&ao_gps_mutex);
		memcpy(&gps_data, &ao_gps_data, sizeof (struct ao_gps_data));
		ao_mutex_put(&ao_gps_mutex);

		if (!(gps_data.flags & AO_GPS_VALID))
			continue;

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
}

static void
ao_gps_tracking_report(void)
{
	static __xdata struct ao_log_record	gps_log;
	static __xdata struct ao_gps_tracking_data	gps_tracking_data;
	uint8_t	c, n;

	for (;;) {
		ao_sleep(&ao_gps_tracking_data);
		ao_mutex_get(&ao_gps_mutex);
		gps_log.tick = ao_gps_tick;
		memcpy(&gps_tracking_data, &ao_gps_tracking_data, sizeof (struct ao_gps_tracking_data));
		ao_mutex_put(&ao_gps_mutex);

		if (!(n = gps_tracking_data.channels))
			continue;

		gps_log.type = AO_LOG_GPS_SAT;
		for (c = 0; c < n; c++)
			if ((gps_log.u.gps_sat.svid = gps_tracking_data.sats[c].svid))
			{
				gps_log.u.gps_sat.c_n = gps_tracking_data.sats[c].c_n_1;
				ao_log_data(&gps_log);
			}
	}
}

static __xdata struct ao_task ao_gps_report_task;
static __xdata struct ao_task ao_gps_tracking_report_task;

void
ao_gps_report_init(void)
{
	ao_add_task(&ao_gps_report_task, ao_gps_report, "gps_report");
	ao_add_task(&ao_gps_tracking_report_task, ao_gps_tracking_report, "gps_tracking_report");
}
