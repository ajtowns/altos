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
#include <ao_log.h>
#include <ao_log_gps.h>
#include <ao_distance.h>
#include <ao_tracker.h>
#include <ao_exti.h>

static uint8_t		ao_tracker_force_telem;

#if HAS_USB_CONNECT
static inline uint8_t
ao_usb_connected(void)
{
	return ao_gpio_get(AO_USB_CONNECT_PORT, AO_USB_CONNECT_PIN, AO_USB_CONNECT) != 0;
}
#else
#define ao_usb_connected()	1
#endif

struct gps_position {
	int32_t	latitude;
	int32_t	longitude;
	int16_t	altitude;
};

#define GPS_RING	16

struct gps_position	gps_position[GPS_RING];

#define ao_gps_ring_next(n)	(((n) + 1) & (GPS_RING - 1))
#define ao_gps_ring_prev(n)	(((n) - 1) & (GPS_RING - 1))

static uint8_t	gps_head;

static uint8_t	tracker_mutex;
static uint8_t	log_started;
static struct ao_telemetry_location gps_data;
static uint8_t	tracker_running;
static uint16_t tracker_interval;

static void
ao_tracker(void)
{
	uint8_t	new;
	int32_t	ground_distance;
	int16_t height;
	uint16_t gps_tick;
	uint8_t new_tracker_running;

#if HAS_ADC
	ao_timer_set_adc_interval(100);
#endif

#if !HAS_USB_CONNECT
	ao_tracker_force_telem = 1;
#endif
	ao_log_scan();

	ao_rdf_set(1);

	tracker_interval = ao_config.tracker_interval;
	ao_gps_set_rate(tracker_interval);

	for (;;) {

		/** Wait for new GPS data
		 */
		while (!(new = ao_gps_new))
			ao_sleep(&ao_gps_new);
		ao_mutex_get(&ao_gps_mutex);
		gps_data = ao_gps_data;
		gps_tick = ao_gps_tick;
		ao_gps_new = 0;
		ao_mutex_put(&ao_gps_mutex);

		new_tracker_running = ao_tracker_force_telem || !ao_usb_connected();

		if (ao_config.tracker_interval != tracker_interval) {
			tracker_interval = ao_config.tracker_interval;
			ao_gps_set_rate(tracker_interval);

			/* force telemetry interval to be reset */
			tracker_running = 0;
		}

		if (new_tracker_running && !tracker_running)
			ao_telemetry_set_interval(AO_SEC_TO_TICKS(tracker_interval));
		else if (!new_tracker_running && tracker_running)
			ao_telemetry_set_interval(0);

		tracker_running = new_tracker_running;

		if (new_tracker_running && !ao_log_running)
			ao_log_start();
		else if (!new_tracker_running && ao_log_running)
			ao_log_stop();

		if (!ao_log_running)
			continue;

		if (new & AO_GPS_NEW_DATA) {
			if ((gps_data.flags & (AO_GPS_VALID|AO_GPS_COURSE_VALID)) ==
			    (AO_GPS_VALID|AO_GPS_COURSE_VALID))
			{
				uint8_t	ring;
				uint8_t	moving = 0;

				for (ring = ao_gps_ring_next(gps_head); ring != gps_head; ring = ao_gps_ring_next(ring)) {
					ground_distance = ao_distance(gps_data.latitude, gps_data.longitude,
								      gps_position[ring].latitude,
								      gps_position[ring].longitude);
					height = gps_position[ring].altitude - gps_data.altitude;
					if (height < 0)
						height = -height;

					if (ao_tracker_force_telem)
						printf("head %d ring %d ground_distance %d height %d\n", gps_head, ring, ground_distance, height);
					if (ground_distance > ao_config.tracker_motion ||
					    height > (ao_config.tracker_motion << 1))
					{
						moving = 1;
						break;
					}
				}
				if (ao_tracker_force_telem) {
					printf ("moving %d started %d\n", moving, log_started);
					flush();
				}
				if (moving) {
					ao_mutex_get(&tracker_mutex);
					if (!log_started) {
						ao_log_gps_flight();
						log_started = 1;
					}
					ao_log_gps_data(gps_tick, &gps_data);
					gps_position[gps_head].latitude = gps_data.latitude;
					gps_position[gps_head].longitude = gps_data.longitude;
					gps_position[gps_head].altitude = gps_data.altitude;
					gps_head = ao_gps_ring_next(gps_head);
					ao_mutex_put(&tracker_mutex);
				}
			}
		}
	}
}

static uint8_t erasing_current;

void
ao_tracker_erase_start(uint16_t flight)
{
	erasing_current = flight == ao_flight_number;
	if (erasing_current) {
		ao_mutex_get(&tracker_mutex);
		ao_log_stop();
		if (++ao_flight_number == 0)
			ao_flight_number = 1;
	}
}

void
ao_tracker_erase_end(void)
{
	if (erasing_current) {
		ao_log_scan();
		log_started = 0;
		ao_mutex_put(&tracker_mutex);
	}
}

static struct ao_task ao_tracker_task;

static void
ao_tracker_set_telem(void)
{
	uint8_t	telem;
	ao_cmd_hex();
	telem = ao_cmd_lex_i;
	if (ao_cmd_status == ao_cmd_success)
		ao_tracker_force_telem = telem;
	ao_cmd_status = ao_cmd_success;
	printf ("flight: %d\n", ao_flight_number);
	printf ("force_telem: %d\n", ao_tracker_force_telem);
	printf ("tracker_running: %d\n", tracker_running);
	printf ("log_started: %d\n", log_started);
	printf ("latitude: %ld\n", (long) gps_data.latitude);
	printf ("longitude: %ld\n", (long) gps_data.longitude);
	printf ("altitude: %d\n", gps_data.altitude);
	printf ("log_running: %d\n", ao_log_running);
	printf ("log_start_pos: %ld\n", (long) ao_log_start_pos);
	printf ("log_cur_pos: %ld\n", (long) ao_log_current_pos);
	printf ("log_end_pos: %ld\n", (long) ao_log_end_pos);
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
