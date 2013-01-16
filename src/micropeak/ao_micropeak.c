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

alt_t		ground_alt, max_alt;
alt_t		ao_max_height;

void
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

int
main(void)
{
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

	ao_microflight();

	ao_log_micro_save();
	ao_compute_height();
	ao_report_altitude();
	for (;;) {
		cli();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
	}
}
