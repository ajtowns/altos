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

/* Main flight thread. */

__pdata enum ao_flight_state	ao_flight_state;	/* current flight state */
__pdata uint16_t		ao_flight_tick;		/* time of last data */
__pdata int16_t			ao_flight_accel;	/* filtered acceleration */
__pdata int16_t			ao_flight_pres;		/* filtered pressure */
__pdata int16_t			ao_ground_pres;		/* startup pressure */
__pdata int16_t			ao_ground_accel;	/* startup acceleration */
__pdata int16_t			ao_min_pres;		/* minimum recorded pressure */
__pdata uint16_t		ao_launch_time;		/* time of launch detect */
__pdata int16_t			ao_main_pres;		/* pressure to eject main */

/*
 * track min/max data over a long interval to detect
 * resting
 */
__pdata uint16_t		ao_interval_end;
__pdata int16_t			ao_interval_cur_min_accel;
__pdata int16_t			ao_interval_cur_max_accel;
__pdata int16_t			ao_interval_cur_min_pres;
__pdata int16_t			ao_interval_cur_max_pres;
__pdata int16_t			ao_interval_min_accel;
__pdata int16_t			ao_interval_max_accel;
__pdata int16_t			ao_interval_min_pres;
__pdata int16_t			ao_interval_max_pres;

__data uint8_t ao_flight_adc;
__xdata int16_t ao_accel, ao_prev_accel, ao_pres;

#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)

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
#define ACCEL_LAND	(ACCEL_G / 10)

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
#define BARO_MAIN	(BARO_kPa)	/* 1kPa */
#define BARO_LAND	(BARO_kPa / 20)	/* .05kPa */

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

/* This value is scaled in a weird way. It's a running total of accelerometer
 * readings minus the ground accelerometer reading. That means it measures
 * velocity, and quite accurately too. As it gets updated 100 times a second,
 * it's scaled by 100
 */
__data int32_t	ao_flight_vel;

/* convert m/s to velocity count */
#define VEL_MPS_TO_COUNT(mps) ((int32_t) ((int32_t) (mps) * (int32_t) 100 / (int32_t) ACCEL_G))

void
ao_flight(void)
{
	__pdata static uint8_t	nsamples = 0;
	
	ao_flight_adc = ao_adc_head;
	ao_prev_accel = 0;
	ao_accel = 0;
	ao_pres = 0;
	for (;;) {
		ao_sleep(&ao_adc_ring);
		while (ao_flight_adc != ao_adc_head) {
			ao_accel = ao_adc_ring[ao_flight_adc].accel;
			ao_pres = ao_adc_ring[ao_flight_adc].pres;
			ao_flight_tick = ao_adc_ring[ao_flight_adc].tick;
			ao_flight_vel += (int32_t) (((ao_accel + ao_prev_accel) >> 4) - (ao_ground_accel << 1));
			ao_prev_accel = ao_accel;
			ao_flight_adc = ao_adc_ring_next(ao_flight_adc);
		}
		ao_flight_accel -= ao_flight_accel >> 4;
		ao_flight_accel += ao_accel >> 4;
		ao_flight_pres -= ao_flight_pres >> 4;
		ao_flight_pres += ao_pres >> 4;
		
		if (ao_flight_pres < ao_min_pres)
			ao_min_pres = ao_flight_pres;

		if ((int16_t) (ao_flight_tick - ao_interval_end) >= 0) {
			ao_interval_max_pres = ao_interval_cur_max_pres;
			ao_interval_min_pres = ao_interval_cur_min_pres;
			ao_interval_max_accel = ao_interval_cur_max_accel;
			ao_interval_min_accel = ao_interval_cur_min_accel;
			ao_interval_end = ao_flight_tick + AO_INTERVAL_TICKS;
		}
			       
		switch (ao_flight_state) {
		case ao_flight_startup:
			if (nsamples < 100) {
				++nsamples;
				continue;
			}
			ao_ground_accel = ao_flight_accel;
			ao_ground_pres = ao_flight_pres;
			ao_min_pres = ao_flight_pres;
			ao_main_pres = ao_ground_pres - BARO_MAIN;
			ao_flight_vel = 0;
			
			ao_interval_end = ao_flight_tick;
			
			/* Go to launchpad state if the nose is pointing up */
			if (ao_flight_accel < ACCEL_NOSE_UP) {
				ao_flight_state = ao_flight_launchpad;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			} else {
				ao_flight_state = ao_flight_idle;
				
				/* Turn on the Green LED in idle mode
				 * This also happens to bring the USB up for the TI board
				 */
				ao_led_on(AO_LED_GREEN);
				ao_timer_set_adc_interval(100);
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			/* signal successful initialization by turning off the LED */
			ao_led_off(AO_LED_RED);
			break;
		case ao_flight_launchpad:

			/* pad to boost:
			 *
			 * accelerometer: > 2g
			 * barometer: > 20m vertical motion
			 */
			if (ao_flight_accel < ACCEL_BOOST || 
			    ao_flight_pres + BARO_LAUNCH < ao_ground_pres)
			{
				ao_flight_state = ao_flight_boost;
				ao_log_start();
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				break;
			}
			break;
		case ao_flight_boost:

			/* boost to coast:
			 *
			 * accelerometer: start to fall at > 1/4 G
			 * time: boost for more than 15 seconds
			 */
			if (ao_flight_accel > ao_ground_accel + (ACCEL_G >> 2) ||
			    (int16_t) (ao_flight_tick - ao_launch_time) > BOOST_TICKS_MAX)
			{
				ao_flight_state = ao_flight_coast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				break;
			}
			break;
		case ao_flight_coast:
			
			/* coast to apogee detect:
			 * 
			 * accelerometer: integrated velocity < 200 m/s
			 * barometer: fall at least 500m from max altitude
			 */
			if (ao_flight_vel < VEL_MPS_TO_COUNT(200) ||
			    ao_flight_pres - (5 * BARO_kPa) > ao_min_pres)
			{
				ao_flight_state = ao_flight_apogee;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
		case ao_flight_apogee:

			/* apogee to drogue deploy:
			 *
			 * accelerometer: integrated velocity < 10m/s
			 * barometer: fall at least 10m
			 */
			if (ao_flight_vel < VEL_MPS_TO_COUNT(-10) ||
			    ao_flight_pres - BARO_APOGEE > ao_min_pres)
			{
				ao_ignite(ao_igniter_drogue);
				ao_flight_state = ao_flight_drogue;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break; 
		case ao_flight_drogue:
			
			/* drogue to main deploy:
			 *
			 * accelerometer: abs(velocity) > 50m/s
			 * barometer: reach main deploy altitude
			 */
			if (ao_flight_vel < VEL_MPS_TO_COUNT(-50) ||
			    ao_flight_vel > VEL_MPS_TO_COUNT(50) ||
			    ao_flight_pres >= ao_main_pres)
			{
				ao_ignite(ao_igniter_main);
				ao_flight_state = ao_flight_main;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			/* fall through... */
		case ao_flight_main:

			/* drogue/main to land:
			 *
			 * accelerometer: value stable
			 * barometer: altitude stable
			 */
			if ((ao_interval_max_accel - ao_interval_min_accel) < ACCEL_LAND ||
			     (ao_interval_max_pres - ao_interval_min_pres) < BARO_LAND)
			{
				ao_flight_state = ao_flight_landed;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
		case ao_flight_landed:
			ao_log_stop();
			break;
		}
	}
}

static __xdata struct ao_task	flight_task;

void
ao_flight_init(void)
{
	ao_flight_state = ao_flight_startup;
	ao_interval_min_accel = 0;
	ao_interval_max_accel = 0x7fff;
	ao_interval_min_pres = 0;
	ao_interval_max_pres = 0x7fff;
	ao_interval_end = AO_INTERVAL_TICKS;

	ao_add_task(&flight_task, ao_flight, "flight");
}

