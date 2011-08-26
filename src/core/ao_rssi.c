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

static __xdata volatile uint16_t	ao_rssi_time;
static __pdata volatile uint16_t	ao_rssi_delay;
static __pdata uint8_t			ao_rssi_led;

void
ao_rssi(void)
{
	for (;;) {
		while ((int16_t) (ao_time() - ao_rssi_time) > AO_SEC_TO_TICKS(3))
			ao_sleep(&ao_rssi_time);
		ao_led_for(ao_rssi_led, AO_MS_TO_TICKS(100));
		ao_delay(ao_rssi_delay);
	}
}

void
ao_rssi_set(int rssi_value)
{
	if (rssi_value > 0)
		rssi_value = 0;
	ao_rssi_delay = AO_MS_TO_TICKS((-rssi_value) * 5);
	ao_rssi_time = ao_time();
	ao_wakeup(&ao_rssi_time);
}

__xdata struct ao_task ao_rssi_task;

void
ao_rssi_init(uint8_t rssi_led)
{
	ao_rssi_led = rssi_led;
	ao_rssi_delay = 0;
	ao_add_task(&ao_rssi_task, ao_rssi, "rssi");
}
