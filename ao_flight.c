/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

__xdata struct ao_adc		ao_flight_data;		/* last acquired data */
__data enum flight_state	ao_flight_state;	/* current flight state */
__data uint16_t			ao_flight_state_tick;	/* time of last data */
__data int16_t			ao_flight_accel;	/* filtered acceleration */
__data int16_t			ao_flight_pres;		/* filtered pressure */
__data int16_t			ao_ground_pres;		/* startup pressure */
__data int16_t			ao_ground_accel;	/* startup acceleration */
__data int16_t			ao_min_pres;		/* minimum recorded pressure */
__data uint16_t			ao_launch_time;		/* time of launch detect */

/* Accelerometer calibration
 *
 * We're sampling the accelerometer through a resistor divider which
 * consists of 5k and 10k resistors. This multiplies the values by 2/3.
 * That goes into the cc1111 A/D converter, which is running at 11 bits
 * of precision with the bits in the MSB of the 16 bit value. Only positive
 * values are used, so values should range from 0-32752 for 0-3.3V. The
 * specs say we should see 40mV/g (uncalibrated), multiply by 2/3 for what
 * the A/D converter sees (26.67 mV/g). We should see 32752/3300 counts/mV,
 * for a final computation of:
 *
 * 26.67 mV/g * 32767/3300 counts/mV = 264.8 counts/g
 *
 * Zero g was measured at 16000 (we would expect 16384)
 */

#define ACCEL_G		265
#define ACCEL_ZERO_G	16000
#define ACCEL_NOSE_UP	(ACCEL_ZERO_G - ACCEL_G * 2 /3)
#define ACCEL_BOOST	(ACCEL_NOSE_UP - ACCEL_G * 2)

/*
 * Barometer calibration
 *
 * We directly sample the barometer. The specs say:
 *
 * Pressure range: 15-115 kPa
 * Voltage at 115kPa: 2.82
 * Output scale: 27mV/kPa
 * 
 * If we want to detect launch with the barometer, we need
 * a large enough bump to not be fooled by noise. At typical
 * launch elevations (0-2000m), a 200Pa pressure change cooresponds
 * to about a 20m elevation change. This is 5.4mV, or about 3LSB.
 * As all of our calculations are done in 16 bits, we'll actually see a change
 * of 16 times this though
 *
 * 27 mV/kPa * 32767 / 3300 counts/mV = 268.1 counts/kPa
 */

#define BARO_kPa	268
#define BARO_LAUNCH	(BARO_kPa / 5)	/* .2kPa */
#define BARO_APOGEE	(BARO_kPa / 10)	/* .1kPa */

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(10)

void
ao_flight(void)
{
	__data static uint8_t	nsamples = 0;
	
	for (;;) {
		ao_sleep(&ao_adc_ring);
		ao_adc_get(&ao_flight_data);
		ao_flight_accel -= ao_flight_accel >> 4;
		ao_flight_accel += ao_flight_data.accel >> 4;
		ao_flight_pres -= ao_flight_pres >> 4;
		ao_flight_pres += ao_flight_data.pres >> 4;
		
		switch (ao_flight_state) {
		case ao_flight_startup:
			if (nsamples < 100) {
				++nsamples;
				continue;
			}
			ao_ground_accel = ao_flight_accel;
			ao_ground_pres = ao_flight_pres;
			ao_min_pres = ao_flight_pres;
			if (ao_flight_accel < ACCEL_NOSE_UP) {
				ao_flight_state = ao_flight_launchpad;
				ao_flight_state_tick = ao_time();
				ao_report_notify();
			} else {
				ao_flight_state = ao_flight_idle;
				ao_flight_state_tick = ao_time();
				ao_report_notify();
			}
			/* signal successful initialization by turning off the LED */
			ao_led_off(AO_LED_RED);
			break;
		case ao_flight_launchpad:
			if (ao_flight_accel < ACCEL_BOOST || 
			    ao_flight_pres + BARO_LAUNCH < ao_ground_pres)
			{
				ao_flight_state = ao_flight_boost;
				ao_flight_state_tick = ao_time();
				ao_log_start();
				ao_report_notify();
				break;
			}
			break;
		case ao_flight_boost:
			if (ao_flight_accel > ACCEL_ZERO_G ||
			    (int16_t) (ao_flight_data.tick - ao_launch_time) > BOOST_TICKS_MAX)
			{
				ao_flight_state = ao_flight_coast;
				ao_flight_state_tick = ao_time();
				ao_report_notify();
				break;
			}
			break;
		case ao_flight_coast:
			if (ao_flight_pres < ao_min_pres)
				ao_min_pres = ao_flight_pres;
			if (ao_flight_pres - BARO_APOGEE > ao_min_pres) {
				ao_flight_state = ao_flight_apogee;
				ao_flight_state_tick = ao_time();
				ao_report_notify();
			}
			break;
		case ao_flight_apogee:
			break;
		}
	}
}

static __xdata struct ao_task	flight_task;

void
ao_flight_init(void)
{
	ao_flight_state = ao_flight_startup;

	ao_add_task(&flight_task, ao_flight);
}

