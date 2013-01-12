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

#include <ao.h>
#include <ao_micropeak.h>
#include <ao_ms5607.h>
#include <ao_log_micro.h>
#include <ao_async.h>

static struct ao_ms5607_sample	sample;
static struct ao_ms5607_value	value;

uint32_t	pa;
uint32_t	pa_avg;
uint32_t	pa_ground;
uint32_t	pa_min;
alt_t		ground_alt, max_alt;
alt_t		ao_max_height;

static uint32_t	pa_sum;

static void
ao_pa_get(void)
{
	ao_ms5607_sample(&sample);
	ao_ms5607_convert(&sample, &value);
	pa = value.pres;
}

static void
ao_compute_height(void)
{
	ground_alt = ao_pa_to_altitude(pa_ground);
	max_alt = ao_pa_to_altitude(pa_min);
	ao_max_height = max_alt - ground_alt;
}

static void
ao_pips(void)
{
	uint8_t	i;
	for (i = 0; i < 10; i++) {
		ao_led_toggle(AO_LED_REPORT);
		ao_delay(AO_MS_TO_TICKS(80));
	}
	ao_delay(AO_MS_TO_TICKS(200));
}

#define NUM_PA_HIST	16

#define SKIP_PA_HIST(i,j)	(((i) + (j)) & (NUM_PA_HIST - 1))

static uint32_t	pa_hist[NUM_PA_HIST];

int
main(void)
{
	int16_t		sample_count;
	uint16_t	time;
	uint32_t	pa_interval_min, pa_interval_max;
	int32_t		pa_diff;
	uint8_t		h, i;

	ao_led_init(LEDS_AVAILABLE);
	ao_timer_init();

	/* Init external hardware */
	ao_spi_init();
	ao_ms5607_init();
	ao_ms5607_setup();

	/* Give the person a second to get their finger out of the way */
	ao_delay(AO_MS_TO_TICKS(1000));

	ao_log_micro_restore();
	ao_compute_height();
	ao_report_altitude();
	ao_pips();
	ao_log_micro_dump();
	
	ao_delay(BOOST_DELAY);
	/* Wait for motion, averaging values to get ground pressure */
	time = ao_time();
	ao_pa_get();
	pa_avg = pa_ground = pa << FILTER_SHIFT;
	sample_count = 0;
	h = 0;
	for (;;) {
		time += SAMPLE_SLEEP;
		if (sample_count == 0)
			ao_led_on(AO_LED_REPORT);
		ao_delay_until(time);
		ao_pa_get();
		if (sample_count == 0)
			ao_led_off(AO_LED_REPORT);
		pa_hist[h] = pa;
		h = SKIP_PA_HIST(h,1);
		pa_avg = pa_avg - (pa_avg >> FILTER_SHIFT) + pa;
		pa_diff = pa_ground - pa_avg;

		/* Check for a significant pressure change */
		if (pa_diff > (BOOST_DETECT << FILTER_SHIFT))
			break;

		if (sample_count < GROUND_AVG * 2) {
			if (sample_count < GROUND_AVG)
				pa_sum += pa;
			++sample_count;
		} else {
			pa_ground = pa_sum >> (GROUND_AVG_SHIFT - FILTER_SHIFT);
			pa_sum = 0;
			sample_count = 0;
		}
	}

	pa_ground >>= FILTER_SHIFT;

	/* Go back and find the first sample a decent interval above the ground */
	pa_min = pa_ground - LAND_DETECT;
	for (i = SKIP_PA_HIST(h,2); i != h; i = SKIP_PA_HIST(i,2)) {
		if (pa_hist[i] < pa_min)
			break;
	}

	/* Log the remaining samples so we get a complete history since leaving the ground */
	for (; i != h; i = SKIP_PA_HIST(i,2)) {
		pa = pa_hist[i];
		ao_log_micro_data();
	}

	/* Now sit around until the pressure is stable again and record the max */

	sample_count = 0;
	pa_min = pa_avg;
	pa_interval_min = pa_avg;
	pa_interval_max = pa_avg;
	for (;;) {
		time += SAMPLE_SLEEP;
		ao_delay_until(time);
		if ((sample_count & 3) == 0)
			ao_led_on(AO_LED_REPORT);
		ao_pa_get();
		if ((sample_count & 3) == 0)
			ao_led_off(AO_LED_REPORT);
		if (sample_count & 1)
			ao_log_micro_data();
		pa_avg = pa_avg - (pa_avg >> FILTER_SHIFT) + pa;
		if (pa_avg < pa_min)
			pa_min = pa_avg;

		if (sample_count == (GROUND_AVG - 1)) {
			pa_diff = pa_interval_max - pa_interval_min;

			/* Check to see if the pressure is now stable */
			if (pa_diff < (LAND_DETECT << FILTER_SHIFT))
				break;
			sample_count = 0;
			pa_interval_min = pa_avg;
			pa_interval_max = pa_avg;
		} else {
			if (pa_avg < pa_interval_min)
				pa_interval_min = pa_avg;
			if (pa_avg > pa_interval_max)
				pa_interval_max = pa_avg;
			++sample_count;
		}
	}
	pa_min >>= FILTER_SHIFT;
	ao_log_micro_save();
	ao_compute_height();
	ao_report_altitude();
	for (;;) {
		cli();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
}
