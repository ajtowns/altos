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

/* XXX make serial numbers real */

uint8_t	ao_serial_number = 2;

void
ao_telemetry_send(__xdata struct ao_telemetry *telemetry) __reentrant
{
	if (ao_flight_state != ao_flight_idle && ao_flight_state != ao_flight_startup) {
		telemetry->addr = ao_serial_number;
		telemetry->flight_state = ao_flight_state;
		ao_radio_send(telemetry);
	}
}

void
ao_telemetry(void)
{
	static __xdata struct ao_radio_recv recv;
	static uint8_t state;

	while (ao_flight_state == ao_flight_startup || ao_flight_state == ao_flight_idle)
		ao_sleep(DATA_TO_XDATA(&ao_flight_state));

	recv.telemetry.type = AO_TELEMETRY_SENSOR;
	for (;;) {
		ao_adc_get(&recv.telemetry.u.adc);
		ao_telemetry_send(&recv.telemetry);
		ao_delay(AO_MS_TO_TICKS(1000));
	}
}

__xdata struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
