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

__xdata uint16_t ao_telemetry_interval = 0;
__xdata uint8_t ao_rdf = 0;
__xdata uint16_t ao_rdf_time;

#define AO_RDF_INTERVAL	AO_SEC_TO_TICKS(3)

void
ao_telemetry(void)
{
	static __xdata struct ao_telemetry telemetry;

	ao_config_get();
	memcpy(telemetry.callsign, ao_config.callsign, AO_MAX_CALLSIGN);
	telemetry.addr = ao_serial_number;
	ao_rdf_time = ao_time();
	for (;;) {
		while (ao_telemetry_interval == 0)
			ao_sleep(&ao_telemetry_interval);
		telemetry.flight_state = ao_flight_state;
		telemetry.flight_accel = ao_flight_accel;
		telemetry.ground_accel = ao_ground_accel;
		telemetry.flight_vel = ao_flight_vel;
		telemetry.flight_pres = ao_flight_pres;
		telemetry.ground_pres = ao_ground_pres;
		ao_adc_get(&telemetry.adc);
		ao_mutex_get(&ao_gps_mutex);
		memcpy(&telemetry.gps, &ao_gps_data, sizeof (struct ao_gps_data));
		memcpy(&telemetry.gps_tracking, &ao_gps_tracking_data, sizeof (struct ao_gps_tracking_data));
		ao_mutex_put(&ao_gps_mutex);
		ao_radio_send(&telemetry);
		ao_delay(ao_telemetry_interval);
		if (ao_rdf &&
		    (int16_t) (ao_time() - ao_rdf_time) >= 0)
		{
			ao_rdf_time = ao_time() + AO_RDF_INTERVAL;
			ao_radio_rdf();
			ao_delay(ao_telemetry_interval);
		}
	}
}

void
ao_telemetry_set_interval(uint16_t interval)
{
	ao_telemetry_interval = interval;
	ao_wakeup(&ao_telemetry_interval);
}

void
ao_rdf_set(uint8_t rdf)
{
	ao_rdf = rdf;
	if (rdf == 0)
		ao_radio_rdf_abort();
}

__xdata struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
