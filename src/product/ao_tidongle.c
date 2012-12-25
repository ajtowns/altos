/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

void
main(void)
{
	ao_clock_init();

	/* Turn on the LED until the system is stable */
	ao_led_init(AO_LED_RED);
	ao_led_on(AO_LED_RED);
	ao_timer_init();
	ao_cmd_init();
	ao_usb_init();
	ao_monitor_init();
	ao_rssi_init(AO_LED_RED);
	ao_radio_init();
	ao_dbg_init();
	ao_config_init();
	/* Bring up the USB link */
	P1DIR |= 1;
	P1 |= 1;
	ao_start_scheduler();
}
