/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_flight.h>
#include <ao_distance.h>
#include <ao_exti.h>

enum ao_flight_state	ao_flight_state;

/* Speeds for the various modes, 2m/s seems reasonable for 'not moving' */
#define AO_TRACKER_NOT_MOVING	200

static uint8_t	ao_tracker_force_telem;

#if HAS_USB_CONNECT
static inline uint8_t
ao_usb_connected(void)
{
	return ao_gpio_get(AO_USB_CONNECT_PORT, AO_USB_CONNECT_PIN, AO_USB_CONNECT) != 0;
}
#else
#define ao_usb_connected()	1
#endif

static void
ao_tracker(void)
{
	uint16_t	telem_rate = AO_SEC_TO_TICKS(1), new_telem_rate;
	uint8_t		gps_rate = 1, new_gps_rate;
	uint8_t		telem_enabled = 1, new_telem_enabled;
	int32_t		start_latitude = 0, start_longitude = 0;
	int16_t		start_altitude = 0;
	uint32_t	ground_distance;
	int16_t		height;
	uint16_t	speed;

	ao_timer_set_adc_interval(100);

	ao_flight_state = ao_flight_startup;
	for (;;) {
		ao_sleep(&ao_gps_new);

		new_gps_rate = gps_rate;
		new_telem_rate = telem_rate;

		new_telem_enabled = ao_tracker_force_telem || !ao_usb_connected();

		ao_mutex_get(&ao_gps_mutex);

		/* Don't change anything if GPS isn't locked */
		if ((ao_gps_data.flags & (AO_GPS_VALID|AO_GPS_COURSE_VALID)) ==
		    (AO_GPS_VALID|AO_GPS_COURSE_VALID))
		{
			switch (ao_flight_state) {
			case ao_flight_startup:
				/* startup to pad when GPS locks */
				ao_flight_state = ao_flight_pad;
				start_latitude = ao_gps_data.longitude;
				start_longitude = ao_gps_data.latitude;
				start_altitude = ao_gps_data.altitude;
				break;
			case ao_flight_pad:
				ground_distance = ao_distance(ao_gps_data.latitude,
							      start_latitude,
							      ao_gps_data.longitude,
							      start_longitude);
				height = ao_gps_data.altitude - start_altitude;
				if (height < 0)
					height = -height;
				if (ground_distance >= ao_config.tracker_start_horiz ||
				    height >= ao_config.tracker_start_vert)
				{
					ao_flight_state = ao_flight_drogue;
					ao_log_start();
				}
				break;
			case ao_flight_drogue:

				/* Modulate data rates based on speed (in cm/s) */
				if (ao_gps_data.climb_rate < 0)
					speed = -ao_gps_data.climb_rate;
				else
					speed = ao_gps_data.climb_rate;
				speed += ao_gps_data.ground_speed;

				if (speed < AO_TRACKER_NOT_MOVING) {
					new_telem_rate = AO_SEC_TO_TICKS(10);
					new_gps_rate = 10;
				} else {
					new_telem_rate = AO_SEC_TO_TICKS(1);
					new_gps_rate = 1;
				}
				break;
			default:
				break;
			}
		}
		ao_mutex_put(&ao_gps_mutex);

		if (new_telem_rate != telem_rate || new_telem_enabled != telem_enabled) {
			if (new_telem_enabled)
				ao_telemetry_set_interval(new_telem_rate);
			else
				ao_telemetry_set_interval(0);
			telem_rate = new_telem_rate;
			telem_enabled = new_telem_enabled;
		}

		if (new_gps_rate != gps_rate) {
			ao_gps_set_rate(new_gps_rate);
			gps_rate = new_gps_rate;
		}
	}
}

static struct ao_task ao_tracker_task;

static void
ao_tracker_set_telem(void)
{
	ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success)
		ao_tracker_force_telem = ao_cmd_lex_i;
}

static const struct ao_cmds ao_tracker_cmds[] = {
	{ ao_tracker_set_telem,	"t <d>\0Set telem on USB" },
	{ 0, NULL },
};

void
ao_tracker_init(void)
{
#if HAS_USB_CONNECT
	ao_enable_input(AO_USB_CONNECT_PORT, AO_USB_CONNECT_PIN, 0);
#endif
	ao_cmd_register(&ao_tracker_cmds[0]);
	ao_add_task(&ao_tracker_task, ao_tracker, "tracker");
}
