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
#include <ao_ms5607.h>
#include <ao_log_micro.h>

static struct ao_ms5607_sample	sample;
static struct ao_ms5607_value	value;

static uint32_t	pa;
static uint32_t	pa_sum;
static uint32_t	pa_avg;
static int32_t	pa_diff;
static uint32_t	pa_ground;
static uint32_t	pa_min;
static uint32_t	pa_interval_min, pa_interval_max;
static alt_t	ground_alt, max_alt;
alt_t		ao_max_height;

static void
ao_pa_get(void)
{
	ao_ms5607_sample(&sample);
	ao_ms5607_convert(&sample, &value);
	pa = value.pres;
}

#define FILTER_SHIFT		3
#define SAMPLE_SLEEP		AO_MS_TO_TICKS(96)

/* 16 sample, or about two seconds worth */
#define GROUND_AVG_SHIFT	4
#define GROUND_AVG		(1 << GROUND_AVG_SHIFT)

/* Pressure change (in Pa) to detect boost */
#define BOOST_DETECT		48	/* 4m at sea level, 4.8m at 2000m */

/* Pressure change (in Pa) to detect landing */
#define LAND_DETECT		12	/* 1m at sea level, 1.2m at 2000m */

static void
ao_compute_height(void)
{
	ground_alt = ao_pa_to_altitude(pa_ground);
	max_alt = ao_pa_to_altitude(pa_min);
	ao_max_height = max_alt - ground_alt;
}

#if !HAS_EEPROM
void
ao_save_flight(void)
{
	ao_eeprom_write(0, &pa_ground, sizeof (pa_ground));
	ao_eeprom_write(sizeof (pa_ground), &pa_min, sizeof (pa_min));
}

void
ao_restore_flight(void)
{
	ao_eeprom_read(0, &pa_ground, sizeof (pa_ground));
	ao_eeprom_read(sizeof (pa_ground), &pa_min, sizeof (pa_min));
}
#endif

int
main(void)
{
	int16_t		sample_count;
	uint16_t	time;
#if HAS_EEPROM
	uint8_t	dump_eeprom = 0;
#endif
	ao_led_init(LEDS_AVAILABLE);
	ao_timer_init();

#if HAS_EEPROM

	/* Set MOSI and CLK as inputs with pull-ups */
	DDRB &= ~(1 << 0) | (1 << 2);
	PORTB |= (1 << 0) | (1 << 2);

	/* Check to see if either MOSI or CLK are pulled low by the
	 * user shorting them to ground. If so, dump the eeprom out
	 * via the LED. Wait for the shorting wire to go away before
	 * continuing.
	 */
	while ((PINB & ((1 << 0) | (1 << 2))) != ((1 << 0) | (1 << 2)))
		dump_eeprom = 1;
	PORTB &= ~(1 << 0) | (1 << 2);

	ao_i2c_init();
#endif
	ao_restore_flight();
	ao_compute_height();
	ao_report_altitude();
	
	ao_spi_init();
	ao_ms5607_init();
	ao_ms5607_setup();

#if HAS_EEPROM
	ao_storage_init();

	/* Check to see if there's a flight recorded in memory */
	if (dump_eeprom && ao_log_micro_scan())
		ao_log_micro_dump();
#endif	

	/* Wait for motion, averaging values to get ground pressure */
	time = ao_time();
	ao_pa_get();
	pa_avg = pa_ground = pa << FILTER_SHIFT;
	sample_count = 0;
	for (;;) {
		time += SAMPLE_SLEEP;
		if (sample_count == 0)
			ao_led_on(AO_LED_BLUE);
		ao_delay_until(time);
		ao_pa_get();
		if (sample_count == 0)
			ao_led_off(AO_LED_BLUE);
		pa_avg = pa_avg - (pa_avg >> FILTER_SHIFT) + pa;
		pa_diff = pa_ground - pa_avg;
		if (pa_diff < 0)
			pa_diff = -pa_diff;

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

#if HAS_EEPROM
	ao_log_micro_data(AO_LOG_MICRO_GROUND | pa_ground);
#endif

	/* Now sit around until the pressure is stable again and record the max */

	sample_count = 0;
	pa_min = pa_avg;
	pa_interval_min = pa_avg;
	pa_interval_max = pa_avg;
	for (;;) {
		time += SAMPLE_SLEEP;
		ao_delay_until(time);
		if ((sample_count & 3) == 0)
			ao_led_on(AO_LED_BLUE);
		ao_pa_get();
		if ((sample_count & 3) == 0)
			ao_led_off(AO_LED_BLUE);
#if HAS_EEPROM
		ao_log_micro_data(AO_LOG_MICRO_DATA | pa);
#endif
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
#if HAS_EEPROM
	ao_log_micro_data(AO_LOG_MICRO_DONE | pa_min);
#endif
	ao_save_flight();
	ao_compute_height();
	ao_report_altitude();
	for (;;) {
		cli();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
}
