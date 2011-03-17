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
static __data uint32_t	ao_log_tiny_pos;

#define AO_LOG_TINY_INTERVAL_ASCENT	AO_MS_TO_TICKS(100)
#define AO_LOG_TINY_INTERVAL_DEFAULT	AO_MS_TO_TICKS(1000)

void
ao_log_tiny_set_interval(uint16_t ticks)
{
	ao_log_tiny_interval = ticks;
}

static __xdata uint16_t ao_log_tiny_data_temp;

#define ao_log_tiny_data(d) do { \
		ao_log_tiny_data_temp = (d);					\
		ao_storage_write(ao_log_tiny_pos, &ao_log_tiny_data_temp, 2);	\
		ao_log_tiny_pos += 2;						\
	} while (0)

void
ao_log(void)
{
	uint16_t		time;
	int16_t			delay;
	enum ao_flight_state	ao_log_tiny_state;

	ao_storage_setup();

	ao_log_tiny_state = ao_flight_invalid;
	ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_DEFAULT;
	while (!ao_log_running)
		ao_sleep(&ao_log_running);

	time = ao_time();
	ao_log_tiny_data(ao_flight_number);
	for (;;) {
		if (ao_flight_state != ao_log_tiny_state) {
			ao_log_tiny_data(ao_flight_state | 0x8000);
			ao_log_tiny_state = ao_flight_state;
			ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_DEFAULT;
			if (ao_log_tiny_state <= ao_flight_coast)
				ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_ASCENT;
		}
		ao_log_tiny_data(ao_flight_pres);	// XXX change to alt
		time += ao_log_tiny_interval;
		delay = time - ao_time();
		if (delay > 0)
			ao_delay(delay);
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
