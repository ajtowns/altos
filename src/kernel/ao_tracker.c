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

enum ao_flight_state	ao_flight_state;

static uint8_t	ao_tracker_force_telem;
static uint8_t	ao_tracker_force_launch;

#if HAS_USB_CONNECT
static inline uint8_t
ao_usb_connected(void)
{
	return ao_gpio_get(AO_USB_CONNECT_PORT, AO_USB_CONNECT_PIN, AO_USB_CONNECT) != 0;
}
#else
#define ao_usb_connected()	1
#endif

#define STARTUP_AVERAGE	5

int32_t	ao_tracker_start_latitude;
int32_t	ao_tracker_start_longitude;
int16_t	ao_tracker_start_altitude;

struct ao_tracker_data	ao_tracker_data[AO_TRACKER_RING];
uint8_t			ao_tracker_head;
static uint8_t		ao_tracker_log_pos;

static uint16_t	telem_rate;
static uint8_t	gps_rate;
static uint8_t	telem_enabled;

static int16_t	lat_sum, lon_sum;
static int32_t	alt_sum;
static int	nsamples;

static void
ao_tracker_state_update(struct ao_tracker_data *tracker)
{
	uint16_t	new_telem_rate;
	uint8_t		new_gps_rate;
	uint8_t		new_telem_enabled;
	uint32_t	ground_distance;
	int16_t		height;
	uint16_t	speed;

	new_gps_rate = gps_rate;
	new_telem_rate = telem_rate;

	new_telem_enabled = ao_tracker_force_telem || !ao_usb_connected();

	/* Don't change anything if GPS isn't locked */
	if ((tracker->new & AO_GPS_NEW_DATA) &&
	    (tracker->gps_data.flags & (AO_GPS_VALID|AO_GPS_COURSE_VALID)) ==
	    (AO_GPS_VALID|AO_GPS_COURSE_VALID))
	{
		switch (ao_flight_state) {
		case ao_flight_startup:
			/* startup to pad when GPS locks */

			lat_sum += tracker->gps_data.latitude;
			lon_sum += tracker->gps_data.longitude;
			alt_sum += tracker->gps_data.altitude;

			if (++nsamples >= STARTUP_AVERAGE) {
				ao_flight_state = ao_flight_pad;
				ao_wakeup(&ao_flight_state);
				ao_tracker_start_latitude = lat_sum / nsamples;
				ao_tracker_start_longitude = lon_sum / nsamples;
				ao_tracker_start_altitude = alt_sum / nsamples;
			}
			break;
		case ao_flight_pad:
			ground_distance = ao_distance(tracker->gps_data.latitude,
						      tracker->gps_data.longitude,
						      ao_tracker_start_latitude,
						      ao_tracker_start_longitude);
			height = tracker->gps_data.altitude - ao_tracker_start_altitude;
			if (height < 0)
				height = -height;

			if (ground_distance >= ao_config.tracker_start_horiz ||
			    height >= ao_config.tracker_start_vert ||
			    ao_tracker_force_launch)
			{
				ao_flight_state = ao_flight_drogue;
				ao_wakeup(&ao_flight_state);
				ao_tracker_log_pos = ao_tracker_ring_next(ao_tracker_head);
				ao_log_start();
				ao_log_gps_flight();
			}
			break;
		case ao_flight_drogue:
			/* Modulate data rates based on speed (in cm/s) */
			if (tracker->gps_data.climb_rate < 0)
				speed = -tracker->gps_data.climb_rate;
			else
				speed = tracker->gps_data.climb_rate;
			speed += tracker->gps_data.ground_speed;

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

#if HAS_LOG
static uint8_t	ao_tracker_should_log;

static void
ao_tracker_log(void)
{
	struct ao_tracker_data	*tracker;

	if (ao_log_running) {
		while (ao_tracker_log_pos != ao_tracker_head) {
			tracker = &ao_tracker_data[ao_tracker_log_pos];
			if (tracker->new & AO_GPS_NEW_DATA) {
				ao_tracker_should_log = ao_log_gps_should_log(tracker->gps_data.latitude,
									      tracker->gps_data.longitude,
									      tracker->gps_data.altitude);
				if (ao_tracker_should_log)
					ao_log_gps_data(tracker->tick, tracker->state, &tracker->gps_data);
			}
			if (tracker->new & AO_GPS_NEW_TRACKING) {
				if (ao_tracker_should_log)
					ao_log_gps_tracking(tracker->tick, &tracker->gps_tracking_data);
			}
			ao_tracker_log_pos = ao_tracker_ring_next(ao_tracker_log_pos);
		}
	}
}
#endif

static void
ao_tracker(void)
{
	uint8_t		new;
	struct ao_tracker_data	*tracker;

#if HAS_ADC
	ao_timer_set_adc_interval(100);
#endif

#if !HAS_USB_CONNECT
	ao_tracker_force_telem = 1;
#endif

	ao_log_scan();

	ao_rdf_set(1);
	ao_telemetry_set_interval(0);
	telem_rate = AO_SEC_TO_TICKS(1);
	telem_enabled = 0;
	gps_rate = 1;

	ao_flight_state = ao_flight_startup;
	for (;;) {
		while (!(new = ao_gps_new))
			ao_sleep(&ao_gps_new);

		/* Stick GPS data into the ring */
		ao_mutex_get(&ao_gps_mutex);
		tracker = &ao_tracker_data[ao_tracker_head];
		tracker->tick = ao_gps_tick;
		tracker->new = new;
		tracker->state = ao_flight_state;
		tracker->gps_data = ao_gps_data;
		tracker->gps_tracking_data = ao_gps_tracking_data;
		ao_tracker_head = ao_tracker_ring_next(ao_tracker_head);

		ao_gps_new = 0;
		ao_mutex_put(&ao_gps_mutex);

		/* Update state based on current GPS data */
		ao_tracker_state_update(tracker);

#if HAS_LOG
		/* Log all gps data */
		ao_tracker_log();
#endif
	}
}

static struct ao_task ao_tracker_task;

static void
ao_tracker_set_telem(void)
{
	ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success) {
		ao_tracker_force_telem = (ao_cmd_lex_i & 1) != 0;
		ao_tracker_force_launch = (ao_cmd_lex_i & 2) != 0;
	}
	printf ("flight %d force telem %d force launch %d\n",
		ao_flight_number, ao_tracker_force_telem, ao_tracker_force_launch);
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
