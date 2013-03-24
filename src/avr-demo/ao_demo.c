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

void
ao_demo(void)
{
	for (;;) {
		ao_led_toggle(AO_LED_RED);
		printf ("hello %d\n", ao_time());
		ao_delay(AO_MS_TO_TICKS(200));
	}
}

int
main(void)
{
	ao_clock_init();

	ao_task_init();

	ao_serial_init();

	ao_led_init(LEDS_AVAILABLE);
	ao_avr_stdio_init();
	printf ("stdio initialized\n");
//	ao_debug_init();
	ao_timer_init();
	ao_cmd_init();
	ao_usb_init();
	ao_lcd_init();

//	ao_add_task(&demo_task, ao_demo, "demo");
	/* Turn on the LED until the system is stable */
	ao_start_scheduler();
	return 0;
}
