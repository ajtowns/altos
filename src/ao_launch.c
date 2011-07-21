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

static void
ao_launch(void)
{
	enum	ao_igniter_status	arm_status, ignite_status;

	ao_led_off(AO_LED_RED);
	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	for (;;) {
		arm_status = ao_igniter_status(ao_igniter_drogue);

		switch (arm_status) {
		case ao_igniter_unknown:
			break;
		case ao_igniter_active:
		case ao_igniter_open:
			break;
		case ao_igniter_ready:
			ignite_status = ao_igniter_status(ao_igniter_main);
			switch (ignite_status) {
			case ao_igniter_unknown:
				/* some kind of failure signal here */
				break;
			case ao_igniter_active:
				break;
			case ao_igniter_open:
				break;
			}
			break;
		}
		ao_delay(AO_SEC_TO_TICKS(1));
	}
}

static __xdata struct ao_task ao_launch_task;

void
ao_launch_init(void)
{
	ao_add_task(&ao_launch_task, ao_launch, "launch status");
}
