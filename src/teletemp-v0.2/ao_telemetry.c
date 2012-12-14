/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 * Copyright © 2012 Anthony Towns <aj@erisian.com.au>
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

static __xdata union ao_telemetry_all	telemetry;

/* Send sensor packet */
static void
ao_send_sensor(void)
{
	__xdata	struct ao_data *packet = (__xdata struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_data_head)];
			
	telemetry.generic.tick = packet->tick;
	telemetry.generic.type = 0xf0;

	telemetry.sensor.state = 0;
	telemetry.sensor.accel = 0;
	telemetry.sensor.pres = 0;
	telemetry.sensor.temp = packet->adc.temp;
	telemetry.sensor.v_batt = 0;
	telemetry.sensor.sense_d = 0;
	telemetry.sensor.sense_m = 0;

	telemetry.sensor.acceleration = 0;
	telemetry.sensor.speed = 0;
	telemetry.sensor.height = packet->adc.etemp;

	telemetry.sensor.ground_pres = 0;
	telemetry.sensor.ground_accel = 0;
	telemetry.sensor.accel_plus_g = 0;
	telemetry.sensor.accel_minus_g = 0;

	ao_radio_send(&telemetry, sizeof (telemetry));
}

static __pdata int8_t ao_telemetry_config_max;
static __pdata int8_t ao_telemetry_config_cur;

static void
ao_send_configuration(void)
{
	if (--ao_telemetry_config_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_CONFIGURATION;
		telemetry.configuration.device = AO_idProduct_NUMBER;
		telemetry.configuration.flight = 0;
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

void
ao_telemetry(void)
{
	uint16_t	time;
	int16_t		delay;

	ao_config_get();
	if (!ao_config.radio_enable)
		ao_exit();

	telemetry.generic.serial = ao_serial_number;
	for (;;) {
		while (ao_telemetry_interval == 0)
			ao_sleep(&telemetry);
		time = ao_time();
		while (ao_telemetry_interval) {
			ao_send_sensor();
			ao_send_configuration();

			time += ao_telemetry_interval;
			delay = time - ao_time();
			if (delay > 0) {
				ao_alarm(delay);
				ao_sleep(&telemetry);
				ao_clear_alarm();
			}
			else
				time = ao_time();
		}
	}
}

void
ao_telemetry_set_interval(uint16_t interval)
{
	int8_t	cur = 0;
	ao_telemetry_interval = interval;
	
	ao_telemetry_config_max = AO_SEC_TO_TICKS(1) / interval;

	ao_wakeup(&telemetry);
}

__xdata struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
