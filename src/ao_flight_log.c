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

#ifndef HAS_ACCEL
#error "Please define HAS_ACCEL"
#endif

__xdata uint8_t	ao_log_adc_pos;
__xdata enum flight_state ao_log_state;

/* separate record to that used in ao_log isn't /really/ needed.
 * the ao_log one is filled out in cmd queries, which aren't possible
 * in flight mode while logging is actually happening
 */
extern __xdata struct ao_log_record log;

void
ao_log(void)
{
	ao_log_waitforlogging();

	log.type = AO_LOG_FLIGHT;
	log.tick = ao_flight_tick;
#if HAS_ACCEL
	log.u.flight.ground_accel = ao_ground_accel;
#endif
	log.u.flight.flight = ao_flight_number;
	ao_log_data(&log);

	/* Write the whole contents of the ring to the log
	 * when starting up.
	 */
	ao_log_adc_pos = ao_adc_ring_next(ao_flight_adc);
	for (;;) {
		/* Write samples to EEPROM */
		while (ao_log_adc_pos != ao_flight_adc) {
			log.type = AO_LOG_SENSOR;
			log.tick = ao_adc_ring[ao_log_adc_pos].tick;
#if HAS_ACCEL
			log.u.sensor.accel = ao_adc_ring[ao_log_adc_pos].accel;
#endif
			log.u.sensor.pres = ao_adc_ring[ao_log_adc_pos].pres;
			ao_log_data(&log);
			if ((ao_log_adc_pos & 0x1f) == 0) {
				log.type = AO_LOG_TEMP_VOLT;
				log.tick = ao_adc_ring[ao_log_adc_pos].tick;
				log.u.temp_volt.temp = ao_adc_ring[ao_log_adc_pos].temp;
				log.u.temp_volt.v_batt = ao_adc_ring[ao_log_adc_pos].v_batt;
				ao_log_data(&log);
				log.type = AO_LOG_DEPLOY;
				log.tick = ao_adc_ring[ao_log_adc_pos].tick;
				log.u.deploy.drogue = ao_adc_ring[ao_log_adc_pos].sense_d;
				log.u.deploy.main = ao_adc_ring[ao_log_adc_pos].sense_m;
				ao_log_data(&log);
			}
			ao_log_adc_pos = ao_adc_ring_next(ao_log_adc_pos);
		}
		/* Write state change to EEPROM */
		if (ao_flight_state != ao_log_state) {
			ao_log_state = ao_flight_state;
			log.type = AO_LOG_STATE;
			log.tick = ao_flight_tick;
			log.u.state.state = ao_log_state;
			log.u.state.reason = 0;
			ao_log_data(&log);

			if (ao_log_state == ao_flight_landed)
				ao_log_stop();
		}

		/* Wait for a while */
		ao_delay(AO_MS_TO_TICKS(100));

		/* Stop logging when told to */
		ao_log_waitforlogging();
	}
}

static __xdata struct ao_task ao_log_task;

void
ao_log_flight_init(void)
{
	/* For now, just log the flight starting at the begining of eeprom */
	ao_log_state = ao_flight_invalid;

	/* Create a task to log events to eeprom */
	ao_add_task(&ao_log_task, ao_log, "log");
}
