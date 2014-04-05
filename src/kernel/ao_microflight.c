/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef AO_FLIGHT_TEST
#include <ao.h>
#endif
#include <ao_micropeak.h>
#include <ao_log_micro.h>

uint32_t	pa;
uint32_t	pa_ground;
uint32_t	pa_min;

static void
ao_microsample(void)
{
	ao_pa_get();
	ao_microkalman_predict();
	ao_microkalman_correct();
}

#define NUM_PA_HIST	(GROUND_AVG)

#define SKIP_PA_HIST(i,j)	(((i) + (j)) & (NUM_PA_HIST - 1))

static uint32_t	pa_hist[NUM_PA_HIST];

void
ao_microflight(void)
{
	int16_t		sample_count;
	uint16_t	time;
	uint32_t	pa_interval_min, pa_interval_max;
	int32_t		pa_diff;
	uint8_t		h, i;
	uint8_t		accel_lock = 0;
	uint32_t	pa_sum = 0;

	/* Wait for motion, averaging values to get ground pressure */

	time = ao_time();
	ao_pa_get();
	ao_microkalman_init();
	pa_ground = pa;
	sample_count = 0;
	h = 0;
	for (;;) {
		time += SAMPLE_SLEEP;
		if ((sample_count & 0x1f) == 0)
			ao_led_on(AO_LED_REPORT);
		ao_delay_until(time);
		ao_microsample();
		if ((sample_count & 0x1f) == 0)
			ao_led_off(AO_LED_REPORT);
		pa_hist[h] = pa;
		h = SKIP_PA_HIST(h,1);
		pa_diff = pa_ground - ao_pa;

		/* Check for a significant pressure change */
		if (pa_diff > BOOST_DETECT)
			break;

		if (sample_count < GROUND_AVG * 2) {
			if (sample_count < GROUND_AVG)
				pa_sum += pa;
			++sample_count;
		} else {
			pa_ground = pa_sum >> GROUND_AVG_SHIFT;
			pa_sum = 0;
			sample_count = 0;
		}
	}

	/* Go back and find the last sample close to the ground */
	pa_min = pa_ground - LAND_DETECT;
	for (i = SKIP_PA_HIST(h,-2); i != SKIP_PA_HIST(h,2); i = SKIP_PA_HIST(i,-2)) {
		if (pa_hist[i] >= pa_min)
			break;
	}

	/* Log the remaining samples so we get a complete history since leaving the ground */
	for (; i != h; i = SKIP_PA_HIST(i,2)) {
		pa = pa_hist[i];
		ao_log_micro_data();
	}

	/* Now sit around until the pressure is stable again and record the max */

	sample_count = 0;
	pa_min = ao_pa;
	pa_interval_min = ao_pa;
	pa_interval_max = ao_pa;
	for (;;) {
		time += SAMPLE_SLEEP;
		ao_delay_until(time);
		if ((sample_count & 3) == 0)
			ao_led_on(AO_LED_REPORT);
		ao_microsample();
		if ((sample_count & 3) == 0)
			ao_led_off(AO_LED_REPORT);
		if (sample_count & 1)
			ao_log_micro_data();

		/* If accelerating upwards, don't look for min pressure */
		if (ao_pa_accel < ACCEL_LOCK_PA)
			accel_lock = ACCEL_LOCK_TIME;
		else if (accel_lock)
			--accel_lock;
		else if (ao_pa < pa_min)
			pa_min = ao_pa;

		if (sample_count == (GROUND_AVG - 1)) {
			pa_diff = pa_interval_max - pa_interval_min;

			/* Check to see if the pressure is now stable */
			if (pa_diff < LAND_DETECT)
				break;
			sample_count = 0;
			pa_interval_min = ao_pa;
			pa_interval_max = ao_pa;
		} else {
			if (ao_pa < pa_interval_min)
				pa_interval_min = ao_pa;
			if (ao_pa > pa_interval_max)
				pa_interval_max = ao_pa;
			++sample_count;
		}
	}
}
