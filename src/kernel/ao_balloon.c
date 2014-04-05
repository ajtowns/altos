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

#ifndef AO_FLIGHT_TEST
#include "ao.h"
#endif

#ifndef HAS_ACCEL
#error Please define HAS_ACCEL
#endif

#ifndef HAS_GPS
#error Please define HAS_GPS
#endif

#ifndef HAS_USB
#error Please define HAS_USB
#endif

#if HAS_SENSOR_ERRORS
/* Any sensor can set this to mark the flight computer as 'broken' */
__xdata uint8_t			ao_sensor_errors;
#endif

__pdata uint16_t		ao_motor_number;	/* number of motors burned so far */

/* Main flight thread. */

__pdata enum ao_flight_state	ao_flight_state;	/* current flight state */

__pdata uint8_t			ao_flight_force_idle;

void
ao_flight(void)
{
	ao_sample_init();
	ao_flight_state = ao_flight_startup;
	for (;;) {

		/*
		 * Process ADC samples, just looping
		 * until the sensors are calibrated.
		 */
		if (!ao_sample())
			continue;

		switch (ao_flight_state) {
		case ao_flight_startup:

			/* Check to see what mode we should go to.
			 *  - Invalid mode if accel cal appears to be out
			 *  - pad mode if we're upright,
			 *  - idle mode otherwise
			 */
			if (!ao_flight_force_idle)
 			{
				/* Set pad mode - we can fly! */
				ao_flight_state = ao_flight_pad;
#if HAS_USB
				/* Disable the USB controller in flight mode
				 * to save power
				 */
				if (!ao_usb_running)
					ao_usb_disable();
#endif

				/* Disable packet mode in pad state */
				ao_packet_slave_stop();

				/* Turn on telemetry system */
				ao_rdf_set(1);
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_BALLOON);

				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
			} else {
				/* Set idle mode */
 				ao_flight_state = ao_flight_idle;
 
				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
			}
			/* wakeup threads due to state change */
			ao_wakeup(DATA_TO_XDATA(&ao_flight_state));

			break;
		case ao_flight_pad:

			/* pad to coast:
			 *
			 * barometer: > 20m vertical motion
			 */
			if (ao_height > AO_M_TO_HEIGHT(20))
			{
				ao_flight_state = ao_flight_drogue;

				/* start logging data */
				ao_log_start();

#if HAS_GPS
				/* Record current GPS position by waking up GPS log tasks */
				ao_gps_new = AO_GPS_NEW_DATA | AO_GPS_NEW_TRACKING;
				ao_wakeup(&ao_gps_new);
#endif

				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
		default:
			break;
		}
	}
}

static __xdata struct ao_task	flight_task;

void
ao_flight_init(void)
{
	ao_flight_state = ao_flight_startup;
	ao_add_task(&flight_task, ao_flight, "flight");
}
