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

#define AO_RDF_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)
#define AO_RDF_LENGTH_MS	500

void
ao_telemetry(void)
{
	uint16_t	time;
	int16_t		delay;
	static __xdata struct ao_telemetry telemetry;

	ao_config_get();
	while (!ao_flight_number)
		ao_sleep(&ao_flight_number);
	memcpy(telemetry.callsign, ao_config.callsign, AO_MAX_CALLSIGN);
	telemetry.serial = ao_serial_number;
	telemetry.flight = ao_log_full() ? 0 : ao_flight_number;
	telemetry.accel_plus_g = ao_config.accel_plus_g;
	telemetry.accel_minus_g = ao_config.accel_minus_g;
	for (;;) {
		while (ao_telemetry_interval == 0)
			ao_sleep(&ao_telemetry_interval);
		time = ao_rdf_time = ao_time();
		while (ao_telemetry_interval) {
			telemetry.flight_state = ao_flight_state;
			telemetry.height = ao_height;
			telemetry.u.k.speed = ao_speed;
			telemetry.accel = ao_accel;
			telemetry.u.k.unused = 0x8000;
#if HAS_ACCEL
			telemetry.ground_accel = ao_ground_accel;
#endif
			telemetry.ground_pres = ao_ground_pres;
#if HAS_ADC
			ao_adc_get(&telemetry.adc);
#endif
#if HAS_GPS
			ao_mutex_get(&ao_gps_mutex);
			memcpy(&telemetry.gps, &ao_gps_data, sizeof (struct ao_gps_data));
			memcpy(&telemetry.gps_tracking, &ao_gps_tracking_data, sizeof (struct ao_gps_tracking_data));
			ao_mutex_put(&ao_gps_mutex);
#endif
			ao_radio_send(&telemetry, sizeof (telemetry));
			if (ao_rdf &&
			    (int16_t) (ao_time() - ao_rdf_time) >= 0)
			{
				ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
				ao_radio_rdf(AO_RDF_LENGTH_MS);
			}
			time += ao_telemetry_interval;
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
	ao_telemetry_interval = interval;
	ao_wakeup(&ao_telemetry_interval);
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

__xdata struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
