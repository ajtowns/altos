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

/* Main flight thread. */

__pdata enum ao_flight_state	ao_flight_state;	/* current flight state */
__pdata uint16_t		ao_launch_tick;		/* time of launch detect */

/*
 * track min/max data over a long interval to detect
 * resting
 */
__pdata uint16_t		ao_interval_end;
__pdata int16_t			ao_interval_min_height;
__pdata int16_t			ao_interval_max_height;
__pdata uint8_t			ao_flight_force_idle;

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

/* Landing is detected by getting constant readings from both pressure and accelerometer
 * for a fairly long time (AO_INTERVAL_TICKS)
 */
#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(10)

#define abs(a)	((a) < 0 ? -(a) : (a))

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
#if HAS_ACCEL
			if (ao_config.accel_plus_g == 0 ||
			    ao_config.accel_minus_g == 0 ||
			    ao_ground_accel < ao_config.accel_plus_g - ACCEL_NOSE_UP ||
			    ao_ground_accel > ao_config.accel_minus_g + ACCEL_NOSE_UP)
			{
				/* Detected an accel value outside -1.5g to 1.5g
				 * (or uncalibrated values), so we go into invalid mode
				 */
				ao_flight_state = ao_flight_invalid;

				/* Turn on packet system in invalid mode on TeleMetrum */
				ao_packet_slave_start();
			} else
#endif
				if (!ao_flight_force_idle
#if HAS_ACCEL
				    && ao_ground_accel < ao_config.accel_plus_g + ACCEL_NOSE_UP
#endif
					)
 			{
				/* Set pad mode - we can fly! */
				ao_flight_state = ao_flight_pad;
#if HAS_USB
				/* Disable the USB controller in flight mode
				 * to save power
				 */
				ao_usb_disable();
#endif

#if !HAS_ACCEL
				/* Disable packet mode in pad state on TeleMini */
				ao_packet_slave_stop();
#endif

				/* Turn on telemetry system */
				ao_rdf_set(1);
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_PAD);

				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
			} else {
				/* Set idle mode */
 				ao_flight_state = ao_flight_idle;
 
#if HAS_ACCEL
				/* Turn on packet system in idle mode on TeleMetrum */
				ao_packet_slave_start();
#endif

				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
			}
			/* wakeup threads due to state change */
			ao_wakeup(DATA_TO_XDATA(&ao_flight_state));

			break;
		case ao_flight_pad:

			/* pad to boost:
			 *
			 * barometer: > 20m vertical motion
			 *             OR
			 * accelerometer: > 2g AND velocity > 5m/s
			 *
			 * The accelerometer should always detect motion before
			 * the barometer, but we use both to make sure this
			 * transition is detected. If the device
			 * doesn't have an accelerometer, then ignore the
			 * speed and acceleration as they are quite noisy
			 * on the pad.
			 */
			if (ao_height > AO_M_TO_HEIGHT(20)
#if HAS_ACCEL
			    || (ao_accel > AO_MSS_TO_ACCEL(20) &&
				ao_speed > AO_MS_TO_SPEED(5))
#endif
				)
			{
				ao_flight_state = ao_flight_boost;
				ao_launch_tick = ao_sample_tick;

				/* start logging data */
				ao_log_start();

				/* Increase telemetry rate */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_FLIGHT);

				/* disable RDF beacon */
				ao_rdf_set(0);

#if HAS_GPS
				/* Record current GPS position by waking up GPS log tasks */
				ao_wakeup(&ao_gps_data);
				ao_wakeup(&ao_gps_tracking_data);
#endif

				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
		case ao_flight_boost:

			/* boost to fast:
			 *
			 * accelerometer: start to fall at > 1/4 G
			 *              OR
			 * time: boost for more than 15 seconds
			 *
			 * Detects motor burn out by the switch from acceleration to
			 * deceleration, or by waiting until the maximum burn duration
			 * (15 seconds) has past.
			 */
			if ((ao_accel < AO_MSS_TO_ACCEL(-2.5) && ao_height > AO_M_TO_HEIGHT(100)) ||
			    (int16_t) (ao_sample_tick - ao_launch_tick) > BOOST_TICKS_MAX)
			{
#if HAS_ACCEL
				ao_flight_state = ao_flight_fast;
#else
				ao_flight_state = ao_flight_coast;
#endif
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
#if HAS_ACCEL
		case ao_flight_fast:
			/*
			 * This is essentially the same as coast,
			 * but the barometer is being ignored as
			 * it may be unreliable.
			 */
			if (ao_speed < AO_MS_TO_SPEED(AO_MAX_BARO_SPEED))
			{
				ao_flight_state = ao_flight_coast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
#endif
		case ao_flight_coast:

			/* apogee detect: coast to drogue deploy:
			 *
			 * speed: < 0
			 *
			 * Also make sure the model altitude is tracking
			 * the measured altitude reasonably closely; otherwise
			 * we're probably transsonic.
			 */
			if (ao_speed < 0
#if !HAS_ACCEL
			    && (ao_sample_alt >= AO_MAX_BARO_HEIGHT || ao_error_h_sq_avg < 100)
#endif
				)
			{
				/* ignite the drogue charge */
				ao_ignite(ao_igniter_drogue);

				/* slow down the telemetry system */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_RECOVER);

				/* Turn the RDF beacon back on */
				ao_rdf_set(1);

				/* and enter drogue state */
				ao_flight_state = ao_flight_drogue;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}

			break;
		case ao_flight_drogue:

			/* drogue to main deploy:
			 *
			 * barometer: reach main deploy altitude
			 *
			 * Would like to use the accelerometer for this test, but
			 * the orientation of the flight computer is unknown after
			 * drogue deploy, so we ignore it. Could also detect
			 * high descent rate using the pressure sensor to
			 * recognize drogue deploy failure and eject the main
			 * at that point. Perhaps also use the drogue sense lines
			 * to notice continutity?
			 */
			if (ao_height <= ao_config.main_deploy)
			{
				ao_ignite(ao_igniter_main);

				/*
				 * Start recording min/max height
				 * to figure out when the rocket has landed
				 */

				/* initialize interval values */
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;

				ao_interval_min_height = ao_interval_max_height = ao_avg_height;

				ao_flight_state = ao_flight_main;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;

			/* fall through... */
		case ao_flight_main:

			/* main to land:
			 *
			 * barometer: altitude stable
			 */

			if (ao_avg_height < ao_interval_min_height)
				ao_interval_min_height = ao_avg_height;
			if (ao_avg_height > ao_interval_max_height)
				ao_interval_max_height = ao_avg_height;

			if ((int16_t) (ao_sample_tick - ao_interval_end) >= 0) {
				if (ao_interval_max_height - ao_interval_min_height <= AO_M_TO_HEIGHT(4))
				{
					ao_flight_state = ao_flight_landed;

					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);

					ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				}
				ao_interval_min_height = ao_interval_max_height = ao_avg_height;
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;
			}
			break;
		case ao_flight_landed:
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
