/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
#include <ao_data.h>
#include <ao_flight.h>

static __xdata struct ao_log_mega log;

__code uint8_t ao_log_format = AO_LOG_FORMAT_TELEMEGA;

static uint8_t
ao_log_csum(__xdata uint8_t *b) __reentrant
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_mega); i++)
		sum += *b++;
	return -sum;
}

uint8_t
ao_log_mega(__xdata struct ao_log_mega *log) __reentrant
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
					 sizeof (struct ao_log_mega));
			ao_log_current_pos += sizeof (struct ao_log_mega);
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

#if HAS_FLIGHT
static __data uint8_t	ao_log_data_pos;

/* a hack to make sure that ao_log_megas fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_mega))] ;

#ifndef AO_SENSOR_INTERVAL_ASCENT
#define AO_SENSOR_INTERVAL_ASCENT	1
#define AO_SENSOR_INTERVAL_DESCENT	10
#define AO_OTHER_INTERVAL		32
#endif

void
ao_log(void)
{
	__pdata uint16_t	next_sensor, next_other;
	uint8_t			i;

	ao_storage_setup();

	ao_log_scan();

	while (!ao_log_running)
		ao_sleep(&ao_log_running);

#if HAS_FLIGHT
	log.type = AO_LOG_FLIGHT;
	log.tick = ao_sample_tick;
#if HAS_ACCEL
	log.u.flight.ground_accel = ao_ground_accel;
#endif
#if HAS_GYRO
	log.u.flight.ground_accel_along = ao_ground_accel_along;
	log.u.flight.ground_accel_across = ao_ground_accel_across;
	log.u.flight.ground_accel_through = ao_ground_accel_through;
	log.u.flight.ground_roll = ao_ground_roll;
	log.u.flight.ground_pitch = ao_ground_pitch;
	log.u.flight.ground_yaw = ao_ground_yaw;
#endif
	log.u.flight.ground_pres = ao_ground_pres;
	log.u.flight.flight = ao_flight_number;
	ao_log_mega(&log);
#endif

	/* Write the whole contents of the ring to the log
	 * when starting up.
	 */
	ao_log_data_pos = ao_data_ring_next(ao_data_head);
	next_other = next_sensor = ao_data_ring[ao_log_data_pos].tick;
	ao_log_state = ao_flight_startup;
	for (;;) {
		/* Write samples to EEPROM */
		while (ao_log_data_pos != ao_data_head) {
			log.tick = ao_data_ring[ao_log_data_pos].tick;
			if ((int16_t) (log.tick - next_sensor) >= 0) {
				log.type = AO_LOG_SENSOR;
#if HAS_MS5607
				log.u.sensor.pres = ao_data_ring[ao_log_data_pos].ms5607_raw.pres;
				log.u.sensor.temp = ao_data_ring[ao_log_data_pos].ms5607_raw.temp;
#endif
#if HAS_MPU6000
				log.u.sensor.accel_x = ao_data_ring[ao_log_data_pos].mpu6000.accel_x;
				log.u.sensor.accel_y = ao_data_ring[ao_log_data_pos].mpu6000.accel_y;
				log.u.sensor.accel_z = ao_data_ring[ao_log_data_pos].mpu6000.accel_z;
				log.u.sensor.gyro_x = ao_data_ring[ao_log_data_pos].mpu6000.gyro_x;
				log.u.sensor.gyro_y = ao_data_ring[ao_log_data_pos].mpu6000.gyro_y;
				log.u.sensor.gyro_z = ao_data_ring[ao_log_data_pos].mpu6000.gyro_z;
#endif
#if HAS_HMC5883
				log.u.sensor.mag_x = ao_data_ring[ao_log_data_pos].hmc5883.x;
				log.u.sensor.mag_y = ao_data_ring[ao_log_data_pos].hmc5883.y;
				log.u.sensor.mag_z = ao_data_ring[ao_log_data_pos].hmc5883.z;
#endif
				log.u.sensor.accel = ao_data_accel(&ao_data_ring[ao_log_data_pos]);
				ao_log_mega(&log);
				if (ao_log_state <= ao_flight_coast)
					next_sensor = log.tick + AO_SENSOR_INTERVAL_ASCENT;
				else
					next_sensor = log.tick + AO_SENSOR_INTERVAL_DESCENT;
			}
			if ((int16_t) (log.tick - next_other) >= 0) {
				log.type = AO_LOG_TEMP_VOLT;
				log.u.volt.v_batt = ao_data_ring[ao_log_data_pos].adc.v_batt;
				log.u.volt.v_pbatt = ao_data_ring[ao_log_data_pos].adc.v_pbatt;
				log.u.volt.n_sense = AO_ADC_NUM_SENSE;
				for (i = 0; i < AO_ADC_NUM_SENSE; i++)
					log.u.volt.sense[i] = ao_data_ring[ao_log_data_pos].adc.sense[i];
				log.u.volt.pyro = ao_pyro_fired;
				ao_log_mega(&log);
				next_other = log.tick + AO_OTHER_INTERVAL;
			}
			ao_log_data_pos = ao_data_ring_next(ao_log_data_pos);
		}
#if HAS_FLIGHT
		/* Write state change to EEPROM */
		if (ao_flight_state != ao_log_state) {
			ao_log_state = ao_flight_state;
			log.type = AO_LOG_STATE;
			log.tick = ao_time();
			log.u.state.state = ao_log_state;
			log.u.state.reason = 0;
			ao_log_mega(&log);

			if (ao_log_state == ao_flight_landed)
				ao_log_stop();
		}
#endif

		ao_log_flush();

		/* Wait for a while */
		ao_delay(AO_MS_TO_TICKS(100));

		/* Stop logging when told to */
		while (!ao_log_running)
			ao_sleep(&ao_log_running);
	}
}
#endif /* HAS_FLIGHT */

uint16_t
ao_log_flight(uint8_t slot)
{
	if (!ao_storage_read(ao_log_pos(slot),
			     &log,
			     sizeof (struct ao_log_mega)))
		return 0;

	if (ao_log_dump_check_data() && log.type == AO_LOG_FLIGHT)
		return log.u.flight.flight;
	return 0;
}
