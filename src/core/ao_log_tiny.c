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

#define AO_LOG_TINY_INTERVAL_DEFAULT	AO_MS_TO_TICKS(1000)
#if USE_FAST_ASCENT_LOG
#define AO_LOG_TINY_INTERVAL_ASCENT	AO_MS_TO_TICKS(100)
#define AO_PAD_RING	8
#else
#define AO_LOG_TINY_INTERVAL_ASCENT	AO_LOG_TINY_INTERVAL_DEFAULT
#define AO_PAD_RING	2
#endif

__code uint8_t ao_log_format = AO_LOG_FORMAT_TINY;

void
ao_log_tiny_set_interval(uint16_t ticks)
{
	ao_log_tiny_interval = ticks;
}


static void ao_log_tiny_data(uint16_t d)
{
	if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
		ao_log_stop();
	if (ao_log_running) {
		ao_storage_write(ao_log_current_pos, DATA_TO_XDATA(&d), 2);
		ao_log_current_pos += 2;
	}
}

static __xdata uint16_t ao_log_pad_ring[AO_PAD_RING];
static __pdata uint8_t ao_log_pad_ring_pos;

#define ao_pad_ring_next(n)	(((n) + 1) & (AO_PAD_RING - 1))

static void ao_log_tiny_queue(uint16_t d)
{
	ao_log_pad_ring[ao_log_pad_ring_pos] = d;
	ao_log_pad_ring_pos = ao_pad_ring_next(ao_log_pad_ring_pos);
}

static void ao_log_tiny_start(void)
{
	uint8_t		p;
	uint16_t	d;

	ao_log_tiny_data(ao_flight_number);
	ao_log_tiny_data(ao_ground_pres);
	p = ao_log_pad_ring_pos;
	do {
		d = ao_log_pad_ring[p];
		/*
		 * ignore unwritten slots
		 */
		if (d)
			ao_log_tiny_data(d);
		p = ao_pad_ring_next(p);
	} while (p != ao_log_pad_ring_pos);
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
	uint8_t			ao_log_started = 0;

	ao_storage_setup();

	ao_log_scan();

	ao_log_tiny_state = ao_flight_invalid;
	ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_ASCENT;
	sum = 0;
	count = 0;
	ao_log_adc = ao_sample_adc;
	last_time = ao_time();
	for (;;) {

		/*
		 * Add in pending sample data
		 */
		ao_sleep(DATA_TO_XDATA(&ao_sample_adc));
		while (ao_log_adc != ao_sample_adc) {
			sum += ao_adc_ring[ao_log_adc].pres;
			count++;
			ao_log_adc = ao_adc_ring_next(ao_log_adc);
		}
		if (ao_log_running) {
			if (!ao_log_started) {
				ao_log_tiny_start();
				ao_log_started = 1;
			}
			if (ao_flight_state != ao_log_tiny_state) {
				ao_log_tiny_data(ao_flight_state | 0x8000);
				ao_log_tiny_state = ao_flight_state;
				ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_DEFAULT;
#if AO_LOG_TINY_INTERVAL_ASCENT != AO_LOG_TINY_INTERVAL_DEFAULT
				if (ao_log_tiny_state <= ao_flight_coast)
					ao_log_tiny_interval = AO_LOG_TINY_INTERVAL_ASCENT;
#endif
				if (ao_log_tiny_state == ao_flight_landed)
					ao_log_stop();
			}
		}

		/* Stop logging when told to */
		if (!ao_log_running && ao_log_started)
			ao_exit();

		/*
		 * Write out the sample when finished
		 */
		now = ao_time();
		if ((int16_t) (now - (last_time + ao_log_tiny_interval)) >= 0 && count) {
			count = sum / count;
			if (ao_log_started)
				ao_log_tiny_data(count);
			else
				ao_log_tiny_queue(count);
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
