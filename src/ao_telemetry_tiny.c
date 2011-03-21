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

__xdata uint8_t ao_rdf = 0;
__xdata uint16_t ao_rdf_time;
__xdata uint16_t ao_telemetry_tiny_interval = 0;

#define AO_RDF_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)
#define AO_RDF_LENGTH_MS	500

void
ao_telemetry_tiny(void)
{
	uint16_t	time;
	int16_t		delay;
	static __xdata struct ao_telemetry_tiny telemetry_tiny;

	ao_config_get();
	while (!ao_flight_number)
		ao_sleep(&ao_flight_number);
	memcpy(telemetry_tiny.callsign, ao_config.callsign, AO_MAX_CALLSIGN);
	telemetry_tiny.serial = ao_serial_number;
	telemetry_tiny.flight = ao_log_full() ? 0 : ao_flight_number;
	for (;;) {
		while (ao_telemetry_tiny_interval == 0)
			ao_sleep(&ao_telemetry_tiny_interval);
		time = ao_rdf_time = ao_time();
		while (ao_telemetry_tiny_interval) {
			telemetry_tiny.flight_state = ao_flight_state;
			telemetry_tiny.height = ao_height;
			telemetry_tiny.speed = ao_speed;
			telemetry_tiny.accel = ao_accel;
			telemetry_tiny.ground_pres = ao_ground_pres;
			ao_adc_get(&telemetry_tiny.adc);
			ao_radio_send(&telemetry_tiny, sizeof (telemetry_tiny));
			if (ao_rdf &&
			    (int16_t) (ao_time() - ao_rdf_time) >= 0)
			{
				ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
				ao_radio_rdf(AO_RDF_LENGTH_MS);
			}
			time += ao_telemetry_tiny_interval;
			delay = time - ao_time();
			if (delay > 0)
				ao_delay(delay);
			else
				time = ao_time();
		}
	}
}

void
ao_telemetry_set_interval(uint16_t interval)
{
	ao_telemetry_tiny_interval = interval;
	ao_wakeup(&ao_telemetry_tiny_interval);
}

void
ao_rdf_set(uint8_t rdf)
{
	ao_rdf = rdf;
	if (rdf == 0)
		ao_radio_rdf_abort();
	else
		ao_rdf_time = ao_time();
}

__xdata struct ao_task	ao_telemetry_tiny_task;

void
ao_telemetry_tiny_init()
{
	ao_add_task(&ao_telemetry_tiny_task, ao_telemetry_tiny, "telemetry_tiny");
}
