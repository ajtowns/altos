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

static __data uint16_t	ao_log_tiny_interval;

#define AO_LOG_TINY_INTERVAL_ASCENT	AO_MS_TO_TICKS(100)
#define AO_LOG_TINY_INTERVAL_DEFAULT	AO_MS_TO_TICKS(1000)

void
ao_log_tiny_set_interval(uint16_t ticks)
{
	ao_log_tiny_interval = ticks;
}

static __xdata uint16_t ao_log_tiny_data_temp;

static void ao_log_tiny_data(uint16_t d)
{
	if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
		ao_log_stop();
	if (ao_log_running) {
		ao_log_tiny_data_temp = (d);
		ao_storage_write(ao_log_current_pos, &ao_log_tiny_data_temp, 2);
		ao_log_current_pos += 2;
	}
}

void
ao_log(void)
{
	uint16_t		last_time;
	uint16_t		now;
	enum ao_flight_state	ao_log_tiny_state;
	int32_t			sum;
	int16_t			count;
	uint8_t			ao_log_adc;

	ao_storage_setup();

	ao_log_scan();

	ao_log_tiny_state = ao_flight_invalid;
	ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_DEFAULT;
	while (!ao_log_running)
		ao_sleep(&ao_log_running);

	ao_log_tiny_data(ao_flight_number);
	ao_log_tiny_data(ao_ground_pres);
	sum = 0;
	count = 0;
	ao_log_adc = ao_sample_adc;
	last_time = ao_time();
	for (;;) {
		ao_sleep(DATA_TO_XDATA(&ao_sample_adc));
		while (ao_log_adc != ao_sample_adc) {
			sum += ao_adc_ring[ao_log_adc].pres;
			count++;
			ao_log_adc = ao_adc_ring_next(ao_log_adc);
		}
		if (ao_flight_state != ao_log_tiny_state) {
			ao_log_tiny_data(ao_flight_state | 0x8000);
			ao_log_tiny_state = ao_flight_state;
			ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_DEFAULT;
			if (ao_log_tiny_state <= ao_flight_coast)
				ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_ASCENT;
			if (ao_log_tiny_state == ao_flight_landed)
				ao_log_stop();
		}
		/* Stop logging when told to */
		if (!ao_log_running)
			ao_exit();
		now = ao_time();
		if ((int16_t) (now - (last_time + ao_log_tiny_interval)) >= 0 && count) {
			ao_log_tiny_data(sum / count);
			sum = 0;
			count = 0;
			last_time = now;
		}
	}
}

uint16_t
ao_log_flight(uint8_t slot)
{
	static __xdata uint16_t flight;

	(void) slot;
	ao_storage_read(0, &flight, 2);
	if (flight == 0xffff)
		flight = 0;
	return flight;
}
