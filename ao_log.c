/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

__data uint32_t		ao_log_current_pos;
__data uint32_t		ao_log_start_pos;
__xdata uint8_t		ao_log_running;
__xdata uint8_t		ao_log_mutex;

static uint8_t
ao_log_csum(uint8_t *b)
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_record); i++)
		sum += *b++;
	return -sum;
}

void
ao_log_data(struct ao_log_record *log)
{
	/* set checksum */
	log->csum = 0;
	log->csum = ao_log_csum((uint8_t *) log);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_running) {
			ao_ee_write(ao_log_current_pos,
				    (uint8_t *) log,
				    sizeof (struct ao_log_record));
			ao_log_current_pos += sizeof (struct ao_log_record);
			if (ao_log_current_pos >= AO_EE_DATA_SIZE)
				ao_log_current_pos = 0;
			if (ao_log_current_pos == ao_log_start_pos)
				ao_log_running = 0;
		}
	} ao_mutex_put(&ao_log_mutex);
}

void
ao_log_flush(void)
{
	ao_ee_flush();
}

__xdata struct ao_log_record ao_log_dump;
static __xdata uint16_t ao_log_dump_flight;
static __xdata uint32_t ao_log_dump_pos;

static uint8_t
ao_log_dump_check_data(void)
{
	if (ao_log_csum((uint8_t *) &ao_log_dump) != 0)
		return 0;
	return 1;
}

static uint8_t
ao_log_dump_scan(void)
{
	if (!ao_ee_read(0, (uint8_t *) &ao_log_dump, sizeof (struct ao_log_record)))
		ao_panic(AO_PANIC_LOG);
	if (ao_log_dump_check_data() && ao_log_dump.type == AO_LOG_FLIGHT) {
		ao_log_dump_flight = ao_log_dump.u.flight.flight;
		return 1;
	} else {
		ao_log_dump_flight = 0;
		return 0;
	}
}

uint8_t
ao_log_dump_first(void)
{
	ao_log_dump_pos = 0;
	if (!ao_log_dump_scan())
		return 0;
	return 1;
}

uint8_t
ao_log_dump_next(void)
{
	ao_log_dump_pos += sizeof (struct ao_log_record);
	if (ao_log_dump_pos >= AO_EE_DEVICE_SIZE)
		return 0;
	if (!ao_ee_read(ao_log_dump_pos, (uint8_t *) &ao_log_dump,
			sizeof (struct ao_log_record)))
		return 0;
	return ao_log_dump_check_data();
}

__xdata uint8_t	ao_log_adc_pos;
__xdata enum flight_state ao_log_state;

void
ao_log(void)
{
	static __xdata struct ao_log_record	log;
	
	ao_log_dump_scan();

	while (!ao_log_running)
		ao_sleep(&ao_log_running);
	
	log.type = AO_LOG_FLIGHT;
	log.tick = ao_flight_state_tick;
	log.u.flight.serial = 0;
	log.u.flight.flight = ao_log_dump_flight + 1;
	ao_log_data(&log);
	for (;;) {
		/* Write state change to EEPROM */
		if (ao_flight_state != ao_log_state) {
			ao_log_state = ao_flight_state;
			log.type = AO_LOG_STATE;
			log.tick = ao_flight_state_tick;
			log.u.state.state = ao_log_state;
			log.u.state.reason = 0;
			ao_log_data(&log);
		}
		/* Write samples to EEPROM */
		while (ao_log_adc_pos != ao_adc_head) {
			log.type = AO_LOG_SENSOR;
			log.tick = ao_adc_ring[ao_log_adc_pos].tick;
			log.u.sensor.accel = ao_adc_ring[ao_log_adc_pos].accel;
			log.u.sensor.pres = ao_adc_ring[ao_log_adc_pos].pres;
			ao_log_data(&log);
			if (ao_log_adc_pos == 0) {
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
			ao_log_adc_pos++;
			if (ao_log_adc_pos == AO_ADC_RING)
				ao_log_adc_pos = 0;
		}
		
		/* Wait for a while */
		ao_delay(AO_MS_TO_TICKS(100));
	}
}

void
ao_log_start(void)
{
	/* start logging */
	ao_log_running = 1;
	ao_wakeup(&ao_log_running);
}

static __xdata struct ao_task ao_log_task;

void
ao_log_init(void)
{
	ao_log_running = 0;

	/* For now, just log the flight starting at the begining of eeprom */
	ao_log_start_pos = 0;
	ao_log_current_pos = ao_log_start_pos;
	ao_log_state = ao_flight_invalid;

	/* Create a task to log events to eeprom */
	ao_add_task(&ao_log_task, ao_log);
}
