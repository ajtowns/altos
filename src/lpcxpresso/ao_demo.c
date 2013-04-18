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

struct ao_task demo_task;

static void demo(void) {
	for (;;) {
		ao_delay(100);
		ao_led_toggle(AO_LED_RED);
	}
}

int
main(void)
{
	int	i;
	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_RED);
	ao_clock_init();
	ao_timer_init();
	
	ao_task_init();

	ao_add_task(&demo_task, demo, "demo");

	ao_start_scheduler();

	for (;;) {
		ao_led_off(AO_LED_RED);
		for (;;)
			if (ao_tick_count & 1)
				break;
		ao_led_on(AO_LED_RED);
		for (;;)
			if (!(ao_tick_count & 1))
				break;
	}
}
