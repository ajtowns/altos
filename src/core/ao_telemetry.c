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
static __pdata uint8_t ao_rdf = 0;
static __pdata uint16_t ao_rdf_time;

#if HAS_APRS
static __pdata uint16_t ao_aprs_time;

#include <ao_aprs.h>
#endif

#if defined(MEGAMETRUM)
#define AO_SEND_MEGA	1
#endif

#if defined(TELEMETRUM_V_0_1) || defined(TELEMETRUM_V_0_2) || defined(TELEMETRUM_V_1_0) || defined(TELEMETRUM_V_1_1) || defined(TELEBALLOON_V_1_1) || defined(TELEMETRUM_V_1_2)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMETRUM
#endif

#if defined(TELEMINI_V_1_0)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMINI
#endif

#if defined(TELENANO_V_0_1)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELENANO
#endif

static __xdata union ao_telemetry_all	telemetry;

#if defined AO_TELEMETRY_SENSOR
/* Send sensor packet */
static void
ao_send_sensor(void)
{
	__xdata	struct ao_data *packet = (__xdata struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];
			
	telemetry.generic.tick = packet->tick;
	telemetry.generic.type = AO_TELEMETRY_SENSOR;

	telemetry.sensor.state = ao_flight_state;
#if HAS_ACCEL
	telemetry.sensor.accel = packet->adc.accel;
#else
	telemetry.sensor.accel = 0;
#endif
	telemetry.sensor.pres = ao_data_pres(packet);
	telemetry.sensor.temp = packet->adc.temp;
	telemetry.sensor.v_batt = packet->adc.v_batt;
#if HAS_IGNITE
	telemetry.sensor.sense_d = packet->adc.sense_d;
	telemetry.sensor.sense_m = packet->adc.sense_m;
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
#endif


#ifdef AO_SEND_MEGA
/* Send mega sensor packet */
static void
ao_send_mega_sensor(void)
{
	__xdata	struct ao_data *packet = (__xdata struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];
			
	telemetry.generic.tick = packet->tick;
	telemetry.generic.type = AO_TELEMETRY_MEGA_SENSOR;

	telemetry.mega_sensor.accel = ao_data_accel(packet);
	telemetry.mega_sensor.pres = ao_data_pres(packet);
	telemetry.mega_sensor.temp = ao_data_temp(packet);

#if HAS_MPU6000
	telemetry.mega_sensor.accel_x = packet->mpu6000.accel_x;
	telemetry.mega_sensor.accel_y = packet->mpu6000.accel_y;
	telemetry.mega_sensor.accel_z = packet->mpu6000.accel_z;

	telemetry.mega_sensor.gyro_x = packet->mpu6000.gyro_x;
	telemetry.mega_sensor.gyro_y = packet->mpu6000.gyro_y;
	telemetry.mega_sensor.gyro_z = packet->mpu6000.gyro_z;
#endif

#if HAS_HMC5883
	telemetry.mega_sensor.mag_x = packet->hmc5883.x;
	telemetry.mega_sensor.mag_y = packet->hmc5883.y;
	telemetry.mega_sensor.mag_z = packet->hmc5883.z;
#endif

	ao_radio_send(&telemetry, sizeof (telemetry));
}

static __pdata int8_t ao_telemetry_mega_data_max;
static __pdata int8_t ao_telemetry_mega_data_cur;

/* Send mega data packet */
static void
ao_send_mega_data(void)
{
	if (--ao_telemetry_mega_data_cur <= 0) {
		__xdata	struct ao_data *packet = (__xdata struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];
		uint8_t	i;

		telemetry.generic.tick = packet->tick;
		telemetry.generic.type = AO_TELEMETRY_MEGA_DATA;

		telemetry.mega_data.state = ao_flight_state;
		telemetry.mega_data.v_batt = packet->adc.v_batt;
		telemetry.mega_data.v_pyro = packet->adc.v_pbatt;

		/* ADC range is 0-4095, so shift by four to save the high 8 bits */
		for (i = 0; i < AO_ADC_NUM_SENSE; i++)
			telemetry.mega_data.sense[i] = packet->adc.sense[i] >> 4;

		telemetry.mega_data.ground_pres = ao_ground_pres;
		telemetry.mega_data.ground_accel = ao_ground_accel;
		telemetry.mega_data.accel_plus_g = ao_config.accel_plus_g;
		telemetry.mega_data.accel_minus_g = ao_config.accel_minus_g;

		telemetry.mega_data.acceleration = ao_accel;
		telemetry.mega_data.speed = ao_speed;
		telemetry.mega_data.height = ao_height;

		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_mega_data_cur = ao_telemetry_mega_data_max;
	}
}
#endif /* AO_SEND_MEGA */

#ifdef AO_SEND_ALL_BARO
static uint8_t		ao_baro_sample;

static void
ao_send_baro(void)
{
	uint8_t		sample = ao_sample_data;
	uint8_t		samples = (sample - ao_baro_sample) & (AO_DATA_RING - 1);

	if (samples > 12) {
		ao_baro_sample = (ao_baro_sample + (samples - 12)) & (AO_DATA_RING - 1);
		samples = 12;
	}
	telemetry.generic.tick = ao_data_ring[sample].tick;
	telemetry.generic.type = AO_TELEMETRY_BARO;
	telemetry.baro.samples = samples;
	for (sample = 0; sample < samples; sample++) {
		telemetry.baro.baro[sample] = ao_data_ring[ao_baro_sample].adc.pres;
		ao_baro_sample = ao_data_ring_next(ao_baro_sample);
	}
	ao_radio_send(&telemetry, sizeof (telemetry));
}
#endif

static __pdata int8_t ao_telemetry_config_max;
static __pdata int8_t ao_telemetry_config_cur;

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
		ao_xmemcpy (telemetry.configuration.callsign,
			    ao_config.callsign,
			    AO_MAX_CALLSIGN);
		ao_xmemcpy (telemetry.configuration.version,
			    CODE_TO_XDATA(ao_version),
			    AO_MAX_VERSION);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_config_cur = ao_telemetry_config_max;
	}
}

#if HAS_GPS

static __pdata int8_t ao_telemetry_loc_cur;
static __pdata int8_t ao_telemetry_sat_cur;

static void
ao_send_location(void)
{
	if (--ao_telemetry_loc_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_LOCATION;
		ao_mutex_get(&ao_gps_mutex);
		ao_xmemcpy(&telemetry.location.flags,
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
		ao_xmemcpy(&telemetry.satellite.sats,
		       &ao_gps_tracking_data.sats,
		       AO_MAX_GPS_TRACKING * sizeof (struct ao_telemetry_satellite_info));
		ao_mutex_put(&ao_gps_mutex);
		ao_radio_send(&telemetry, sizeof (telemetry));
		ao_telemetry_sat_cur = ao_telemetry_config_max;
	}
}
#endif

#if HAS_COMPANION

static __pdata int8_t ao_telemetry_companion_max;
static __pdata int8_t ao_telemetry_companion_cur;

static void
ao_send_companion(void)
{
	if (--ao_telemetry_companion_cur <= 0) {
		telemetry.generic.type = AO_TELEMETRY_COMPANION;
		telemetry.companion.board_id = ao_companion_setup.board_id;
		telemetry.companion.update_period = ao_companion_setup.update_period;
		telemetry.companion.channels = ao_companion_setup.channels;
		ao_mutex_get(&ao_companion_mutex);
		ao_xmemcpy(&telemetry.companion.companion_data,
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
#if HAS_APRS
		ao_aprs_time = time;
#endif
		while (ao_telemetry_interval) {


#if HAS_APRS
			if (!(ao_config.radio_enable & AO_RADIO_DISABLE_TELEMETRY))
#endif
			{
#ifdef AO_SEND_ALL_BARO
				ao_send_baro();
#endif
#ifdef AO_SEND_MEGA
				ao_send_mega_sensor();
				ao_send_mega_data();
#else
				ao_send_sensor();
#endif

#if HAS_COMPANION
				if (ao_companion_running)
					ao_send_companion();
#endif
				ao_send_configuration();
#if HAS_GPS
				ao_send_location();
				ao_send_satellite();
#endif
			}
#ifndef AO_SEND_ALL_BARO
			if (ao_rdf &&
#if HAS_APRS
			    !(ao_config.radio_enable & AO_RADIO_DISABLE_RDF) &&
#endif
			    (int16_t) (ao_time() - ao_rdf_time) >= 0)
			{
#if HAS_IGNITE_REPORT
				uint8_t	c;
#endif
				ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
#if HAS_IGNITE_REPORT
				if (ao_flight_state == ao_flight_pad && (c = ao_report_igniter()))
					ao_radio_continuity(c);
				else
#endif
					ao_radio_rdf();
			}
#if HAS_APRS
			if (ao_rdf &&
			    (ao_config.radio_enable & AO_RADIO_ENABLE_APRS) &&
			    (int16_t) (ao_time() - ao_aprs_time) >= 0)
			{
				ao_aprs_time = ao_time() + AO_APRS_INTERVAL_TICKS;
				ao_aprs_send();
			}
#endif
#endif
			time += ao_telemetry_interval;
			delay = time - ao_time();
			if (delay > 0) {
				ao_alarm(delay);
				ao_sleep(&telemetry);
				ao_clear_alarm();
			}
			else
				time = ao_time();
		bottom:	;
		}
	}
}

void
ao_telemetry_set_interval(uint16_t interval)
{
	int8_t	cur = 0;
	ao_telemetry_interval = interval;
	
#if AO_SEND_MEGA
	if (interval > 1)
		ao_telemetry_mega_data_max = 1;
	else
		ao_telemetry_mega_data_max = 2;
	if (ao_telemetry_mega_data_max > cur)
		cur++;
	ao_telemetry_mega_data_cur = cur;
#endif

#if HAS_COMPANION
	if (!ao_companion_setup.update_period)
		ao_companion_setup.update_period = AO_SEC_TO_TICKS(1);
	ao_telemetry_companion_max = ao_companion_setup.update_period / interval;
	if (ao_telemetry_companion_max > cur)
		cur++;
	ao_telemetry_companion_cur = cur;
#endif

	ao_telemetry_config_max = AO_SEC_TO_TICKS(1) / interval;
#if HAS_COMPANION
	if (ao_telemetry_config_max > cur)
		cur++;
	ao_telemetry_config_cur = cur;
#endif

#if HAS_GPS
	if (ao_telemetry_config_max > cur)
		cur++;
	ao_telemetry_loc_cur = cur;
	if (ao_telemetry_config_max > cur)
		cur++;
	ao_telemetry_sat_cur = cur;
#endif
	ao_wakeup(&telemetry);
}

void
ao_rdf_set(uint8_t rdf)
{
	ao_rdf = rdf;
	if (rdf == 0)
		ao_radio_rdf_abort();
	else {
		ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
#if HAS_APRS
		ao_aprs_time = ao_time() + AO_APRS_INTERVAL_TICKS;
#endif
	}
}

__xdata struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
