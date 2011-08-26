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

/* Landing is detected by getting constant readings from both pressure and accelerometer
 * for a fairly long time (AO_INTERVAL_TICKS)
 */
#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)

static void
ao_flight_nano(void)
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
			if (ao_flight_force_idle) {
				/* Set idle mode */
				ao_flight_state = ao_flight_idle;
			} else {
				ao_flight_state = ao_flight_pad;
				/* Disable packet mode in pad state */
				ao_packet_slave_stop();

				/* Turn on telemetry system */
				ao_rdf_set(1);
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_PAD);
			}
			/* signal successful initialization by turning off the LED */
			ao_led_off(AO_LED_RED);

			/* wakeup threads due to state change */
			ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			break;
		case ao_flight_pad:
			if (ao_height> AO_M_TO_HEIGHT(20)) {
				ao_flight_state = ao_flight_drogue;
				ao_launch_tick = ao_sample_tick;

				/* start logging data */
				ao_log_start();

				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
		case ao_flight_drogue:
			/* drogue/main to land:
			 *
			 * barometer: altitude stable
			 */

			if (ao_height < ao_interval_min_height)
				ao_interval_min_height = ao_height;
			if (ao_height > ao_interval_max_height)
				ao_interval_max_height = ao_height;

			if ((int16_t) (ao_sample_tick - ao_interval_end) >= 0) {
				if (ao_interval_max_height - ao_interval_min_height < AO_M_TO_HEIGHT(5))
				{
					ao_flight_state = ao_flight_landed;

					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);
					ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				}
				ao_interval_min_height = ao_interval_max_height = ao_height;
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;
			}
			break;
		}
	}
}

static __xdata struct ao_task	flight_task;

void
ao_flight_nano_init(void)
{
	ao_flight_state = ao_flight_startup;
	ao_add_task(&flight_task, ao_flight_nano, "flight");
}
