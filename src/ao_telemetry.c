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
#include "ao_product.h"

static __pdata uint16_t ao_telemetry_interval;
static __pdata int8_t ao_telemetry_config_max;
static __pdata int8_t ao_telemetry_config_cur;
#if HAS_GPS
static __pdata int8_t ao_telemetry_loc_cur;
static __pdata int8_t ao_telemetry_sat_cur;
#endif
#if HAS_COMPANION
static __pdata int8_t ao_telemetry_companion_max;
static __pdata int8_t ao_telemetry_companion_cur;
#endif
static __pdata uint8_t ao_rdf = 0;
static __pdata uint16_t ao_rdf_time;

#define AO_RDF_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)
#define AO_RDF_LENGTH_MS	500

#if defined(TELEMETRUM_V_0_1) || defined(TELEMETRUM_V_0_2) || defined(TELEMETRUM_V_1_0) || defined(TELEMETRUM_V_1_1)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMETRUM
#endif

#if defined(TELEMINI_V_1_0)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMINI
#endif

#if defined(TELENANO_V_0_1)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELENANO
#endif

static __xdata union ao_telemetry_all	telemetry;

/* Send sensor packet */
static void
ao_send_sensor(void)
{
	uint8_t		sample;
	sample = ao_sample_adc;
			
	telemetry.generic.tick = ao_adc_ring[sample].tick;
	telemetry.generic.type = AO_TELEMETRY_SENSOR;

	telemetry.sensor.state = ao_flight_state;
#if HAS_ACCEL
	telemetry.sensor.accel = ao_adc_ring[sample].accel;
#else
	telemetry.sensor.accel = 0;
#endif
	telemetry.sensor.pres = ao_adc_ring[sample].pres;
	telemetry.sensor.temp = ao_adc_ring[sample].temp;
	telemetry.sensor.v_batt = ao_adc_ring[sample].v_batt;
#if HAS_IGNITE
	telemetry.sensor.sense_d = ao_adc_ring[sample].sense_d;
	telemetry.sensor.sense_m = ao_adc_ring[sample].sense_m;
#else
	telemetry.sensor.sense_d = 0;
	telemetry.sensor.sense_m = 0;
#endif

	telemetry.sensor.acceleration = ao_accel;
	telemetry.sensor.speed = ao_speed;
	telemetry.sensor.height = ao_height;

	telemetry.sensor.ground_pres = ao_ground_pres;
#if HAS_ACCEL
	telemetry.sensor.ground_accel = ao_ground_accel;
	telemetry.sensor.accel_plus_g = ao_config.accel_plus_g;
	telemetry.sensor.accel_minus_g = ao_config.accel_minus_g;
#else
	telemetry.sensor.ground_accel = 0;
	telemetry.sensor.accel_plus_g = 0;
	telemetry.sensor.accel_minus_g = 0;
#endif

	ao_radio_send(&telemetry, sizeof (telemetry));
}

static void
ao_send_configuration(void)
{
	if (--ao_telemetry_config_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_CONFIGURATION;
		telemetry.configuration.device = AO_idProduct_NUMBER;
		telemetry.configuration.flight = ao_log_full() ? 0 : ao_flight_number;
		telemetry.configuration.config_major = AO_CONFIG_MAJOR;
		telemetry.configuration.config_minor = AO_CONFIG_MINOR;
		telemetry.configuration.apogee_delay = ao_config.apogee_delay;
		telemetry.configuration.main_deploy = ao_config.main_deploy;
		telemetry.configuration.flight_log_max = ao_config.flight_log_max >> 10;
		memcpy (telemetry.configuration.callsign,
			ao_config.callsign,
			AO_MAX_CALLSIGN);
		memcpy (telemetry.configuration.version,
			ao_version,
			AO_MAX_VERSION);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_config_cur = ao_telemetry_config_max;
	}
}

#if HAS_GPS
static void
ao_send_location(void)
{
	if (--ao_telemetry_loc_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_LOCATION;
		ao_mutex_get(&ao_gps_mutex);
		memcpy(&telemetry.location.flags,
		       &ao_gps_data.flags,
		       26);
		ao_mutex_put(&ao_gps_mutex);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_loc_cur = ao_telemetry_config_max;
	}
}

static void
ao_send_satellite(void)
{
	if (--ao_telemetry_sat_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_SATELLITE;
		ao_mutex_get(&ao_gps_mutex);
		telemetry.satellite.channels = ao_gps_tracking_data.channels;
		memcpy(&telemetry.satellite.sats,
		       &ao_gps_tracking_data.sats,
		       AO_MAX_GPS_TRACKING * sizeof (struct ao_telemetry_satellite_info));
		ao_mutex_put(&ao_gps_mutex);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_sat_cur = ao_telemetry_config_max;
	}
}
#endif

#if HAS_COMPANION
static void
ao_send_companion(void)
{
	if (--ao_telemetry_companion_cur <= 0) {
		telemetry.generic.type = AO_TELEMETRY_COMPANION;
		telemetry.companion.board_id = ao_companion_setup.board_id;
		telemetry.companion.update_period = ao_companion_setup.update_period;
		telemetry.companion.channels = ao_companion_setup.channels;
		ao_mutex_get(&ao_companion_mutex);
		memcpy(&telemetry.companion.companion_data,
		       ao_companion_data,
		       ao_companion_setup.channels * 2);
		ao_mutex_put(&ao_companion_mutex);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_companion_cur = ao_telemetry_companion_max;
	}
}
#endif

void
ao_telemetry(void)
{
	uint16_t	time;
	int16_t		delay;

	ao_config_get();
	if (!ao_config.radio_enable)
		ao_exit();
	while (!ao_flight_number)
		ao_sleep(&ao_flight_number);

	telemetry.generic.serial = ao_serial_number;
	for (;;) {
		while (ao_telemetry_interval == 0)
			ao_sleep(&telemetry);
		time = ao_rdf_time = ao_time();
		while (ao_telemetry_interval) {


			ao_send_sensor();
#if HAS_COMPANION
			if (ao_companion_running)
				ao_send_companion();
#endif
			ao_send_configuration();
#if HAS_GPS
			ao_send_location();
			ao_send_satellite();
#endif
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

#if HAS_COMPANION
	if (!ao_companion_setup.update_period)
		ao_companion_setup.update_period = AO_SEC_TO_TICKS(1);
	ao_telemetry_companion_max = ao_companion_setup.update_period / interval;
	ao_telemetry_companion_cur = 1;
#endif

	ao_telemetry_config_max = AO_SEC_TO_TICKS(1) / interval;
#if HAS_COMPANION
	ao_telemetry_config_cur = ao_telemetry_companion_cur;
	if (ao_telemetry_config_max > ao_telemetry_config_cur)
		ao_telemetry_config_cur++;
#else
	ao_telemetry_config_cur = 1;
#endif

#if HAS_GPS
	ao_telemetry_loc_cur = ao_telemetry_config_cur;
	if (ao_telemetry_config_max > ao_telemetry_loc_cur)
		ao_telemetry_loc_cur++;
	ao_telemetry_sat_cur = ao_telemetry_loc_cur;
	if (ao_telemetry_config_max > ao_telemetry_sat_cur)
		ao_telemetry_sat_cur++;
#endif
	ao_wakeup(&telemetry);
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
