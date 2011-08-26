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
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 log,
					 sizeof (struct ao_log_record));
			ao_log_current_pos += sizeof (struct ao_log_record);
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

static __data uint8_t	ao_log_adc_pos;

/* a hack to make sure that ao_log_records fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_record))] ;

#define AO_SENSOR_INTERVAL_ASCENT	1
#define AO_SENSOR_INTERVAL_DESCENT	10
#define AO_OTHER_INTERVAL		32

void
ao_log(void)
{
	__pdata uint16_t	next_sensor, next_other;

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
	ao_log_state = ao_flight_startup;
	for (;;) {
		/* Write samples to EEPROM */
		while (ao_log_adc_pos != ao_sample_adc) {
			log.tick = ao_adc_ring[ao_log_adc_pos].tick;
			if ((int16_t) (log.tick - next_sensor) >= 0) {
				log.type = AO_LOG_SENSOR;
				log.u.sensor.accel = ao_adc_ring[ao_log_adc_pos].accel;
				log.u.sensor.pres = ao_adc_ring[ao_log_adc_pos].pres;
				ao_log_data(&log);
				if (ao_log_state <= ao_flight_coast)
					next_sensor = log.tick + AO_SENSOR_INTERVAL_ASCENT;
				else
					next_sensor = log.tick + AO_SENSOR_INTERVAL_DESCENT;
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
		/* Write state change to EEPROM */
		if (ao_flight_state != ao_log_state) {
			ao_log_state = ao_flight_state;
			log.type = AO_LOG_STATE;
			log.tick = ao_sample_tick;
			log.u.state.state = ao_log_state;
			log.u.state.reason = 0;
			ao_log_data(&log);

			if (ao_log_state == ao_flight_landed)
				ao_log_stop();
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
