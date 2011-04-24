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

#ifndef HAS_ACCEL
#error Please define HAS_ACCEL
#endif

#ifndef HAS_GPS
#error Please define HAS_GPS
#endif

#ifndef HAS_USB
#error Please define HAS_USB
#endif

/*
 * track min/max data over a long interval to detect
 * resting
 */
__pdata uint16_t		ao_interval_end;
__pdata int16_t			ao_interval_min_height;
__pdata int16_t			ao_interval_max_height;

__xdata uint8_t			ao_flight_force_idle;

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

/* Landing is detected by getting constant readings from both pressure and accelerometer
 * for a fairly long time (AO_INTERVAL_TICKS)
 */
#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)

#define abs(a)	((a) < 0 ? -(a) : (a))

void
ao_kalman(void)
{
}

void
ao_flight(void)
{
	ao_sample_init();

#if HAS_USB
	/* Disable the USB controller in flight mode to save power */
	ao_usb_disable();
#endif
	/* Disable packet mode in pad state */
	ao_packet_slave_stop();

	/* signal successful initialization by turning off the LED */
	ao_led_off(AO_LED_RED);

	/* start logging data */
	ao_log_start();

#if 0
	/* turn off the ADC capture */
	ao_timer_set_adc_interval(0);
#endif

	for (;;) {

		/*
		 * Process ADC samples, just looping
		 * until the sensors are calibrated.
		 */
		if (!ao_sample())
			continue;

#if HAS_GPS
		/* Record current GPS position by waking up GPS log tasks */
		ao_wakeup(&ao_gps_data);
		ao_wakeup(&ao_gps_tracking_data);
#endif
	}
}

static __xdata struct ao_task	flight_task;

void
ao_flight_init(void)
{
	ao_add_task(&flight_task, ao_flight, "flight");
}
