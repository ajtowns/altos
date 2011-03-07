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
__pdata uint16_t		ao_flight_tick;		/* time of last data */
__pdata uint16_t		ao_flight_prev_tick;	/* time of previous data */
__pdata int16_t			ao_flight_pres;		/* filtered pressure */
__pdata int16_t			ao_ground_pres;		/* startup pressure */
__pdata int16_t			ao_min_pres;		/* minimum recorded pressure */
__pdata uint16_t		ao_launch_tick;		/* time of launch detect */
__pdata int16_t			ao_main_pres;		/* pressure to eject main */
#if HAS_ACCEL
__pdata int16_t			ao_flight_accel;	/* filtered acceleration */
__pdata int16_t			ao_ground_accel;	/* startup acceleration */
#endif

/*
 * track min/max data over a long interval to detect
 * resting
 */
__pdata uint16_t		ao_interval_end;
__pdata int16_t			ao_interval_cur_min_pres;
__pdata int16_t			ao_interval_cur_max_pres;
__pdata int16_t			ao_interval_min_pres;
__pdata int16_t			ao_interval_max_pres;
#if HAS_ACCEL
__pdata int16_t			ao_interval_cur_min_accel;
__pdata int16_t			ao_interval_cur_max_accel;
__pdata int16_t			ao_interval_min_accel;
__pdata int16_t			ao_interval_max_accel;
#endif

__data uint8_t ao_flight_adc;
__pdata int16_t ao_raw_pres;
__xdata uint8_t ao_flight_force_idle;

#if HAS_ACCEL
__pdata int16_t ao_raw_accel, ao_raw_accel_prev;
__pdata int16_t ao_accel_2g;

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
 * Zero g was measured at 16000 (we would expect 16384).
 * Note that this value is only require to tell if the
 * rocket is standing upright. Once that is determined,
 * the value of the accelerometer is averaged for 100 samples
 * to find the resting accelerometer value, which is used
 * for all further flight computations
 */

#define GRAVITY 9.80665
/* convert m/s to velocity count */
#define VEL_MPS_TO_COUNT(mps) (((int32_t) (((mps) / GRAVITY) * (AO_HERTZ/2))) * (int32_t) ao_accel_2g)

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)
#define ACCEL_BOOST	ao_accel_2g
#define ACCEL_COAST	(ao_accel_2g >> 3)
#define ACCEL_INT_LAND	(ao_accel_2g >> 3)
#define ACCEL_VEL_MACH	VEL_MPS_TO_COUNT(200)
#define ACCEL_VEL_BOOST	VEL_MPS_TO_COUNT(5)

#endif

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
#define BARO_LAUNCH	(BARO_kPa / 5)	/* .2kPa, or about 20m */
#define BARO_APOGEE	(BARO_kPa / 10)	/* .1kPa, or about 10m */
#define BARO_COAST	(BARO_kPa * 5)  /* 5kpa, or about 500m */
#define BARO_MAIN	(BARO_kPa)	/* 1kPa, or about 100m */
#define BARO_INT_LAND	(BARO_kPa / 20)	/* .05kPa, or about 5m */
#define BARO_LAND	(BARO_kPa * 10)	/* 10kPa or about 1000m */

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

#if HAS_ACCEL
/* This value is scaled in a weird way. It's a running total of accelerometer
 * readings minus the ground accelerometer reading. That means it measures
 * velocity, and quite accurately too. As it gets updated 100 times a second,
 * it's scaled by 100
 */
__pdata int32_t	ao_flight_vel;
__pdata int32_t ao_min_vel;
__pdata int32_t	ao_old_vel;
__pdata int16_t ao_old_vel_tick;
__xdata int32_t ao_raw_accel_sum;
#endif

__xdata int32_t ao_raw_pres_sum;

/* Landing is detected by getting constant readings from both pressure and accelerometer
 * for a fairly long time (AO_INTERVAL_TICKS)
 */
#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(5)

#define abs(a)	((a) < 0 ? -(a) : (a))

void
ao_flight(void)
{
	__pdata static uint16_t	nsamples = 0;

	ao_flight_adc = ao_adc_head;
	ao_raw_pres = 0;
#if HAS_ACCEL
	ao_raw_accel_prev = 0;
	ao_raw_accel = 0;
#endif
	ao_flight_tick = 0;
	for (;;) {
		ao_wakeup(DATA_TO_XDATA(&ao_flight_adc));
		ao_sleep(DATA_TO_XDATA(&ao_adc_head));
		while (ao_flight_adc != ao_adc_head) {
#if HAS_ACCEL
			__pdata uint8_t ticks;
			__pdata int16_t ao_vel_change;
#endif
			__xdata struct ao_adc *ao_adc;
			ao_flight_prev_tick = ao_flight_tick;

			/* Capture a sample */
			ao_adc = &ao_adc_ring[ao_flight_adc];
			ao_flight_tick = ao_adc->tick;
			ao_raw_pres = ao_adc->pres;
			ao_flight_pres -= ao_flight_pres >> 4;
			ao_flight_pres += ao_raw_pres >> 4;

#if HAS_ACCEL
			ao_raw_accel = ao_adc->accel;
#if HAS_ACCEL_REF
			/*
			 * Ok, the math here is a bit tricky.
			 *
			 * ao_raw_accel:  ADC output for acceleration
			 * ao_accel_ref:  ADC output for the 5V reference.
			 * ao_cook_accel: Corrected acceleration value
			 * Vcc:           3.3V supply to the CC1111
			 * Vac:           5V supply to the accelerometer
			 * accel:         input voltage to accelerometer ADC pin
			 * ref:           input voltage to 5V reference ADC pin
			 *
			 *
			 * Measured acceleration is ratiometric to Vcc:
			 *
			 *     ao_raw_accel   accel
			 *     ------------ = -----
			 *        32767        Vcc
			 *
			 * Measured 5v reference is also ratiometric to Vcc:
			 *
			 *     ao_accel_ref    ref
			 *     ------------ = -----
			 *        32767        Vcc
			 *
			 *
			 *	ao_accel_ref = 32767 * (ref / Vcc)
			 *
			 * Acceleration is measured ratiometric to the 5V supply,
			 * so what we want is:
			 *
			 *	ao_cook_accel    accel
			 *      ------------- =  -----
			 *          32767         ref
			 *
			 *
			 *	                accel    Vcc
			 *                    = ----- *  ---
			 *                       Vcc     ref
			 *
			 *                      ao_raw_accel       32767
			 *                    = ------------ *  ------------
			 *                         32737        ao_accel_ref
			 *
			 * Multiply through by 32767:
			 *
			 *                      ao_raw_accel * 32767
			 *	ao_cook_accel = --------------------
			 *                          ao_accel_ref
			 *
			 * Now, the tricky part. Getting this to compile efficiently
			 * and keeping all of the values in-range.
			 *
			 * First off, we need to use a shift of 16 instead of * 32767 as SDCC
			 * does the obvious optimizations for byte-granularity shifts:
			 *
			 *	ao_cook_accel = (ao_raw_accel << 16) / ao_accel_ref
			 *
			 * Next, lets check our input ranges:
			 *
			 * 	0 <= ao_raw_accel <= 0x7fff		(singled ended ADC conversion)
			 *	0x7000 <= ao_accel_ref <= 0x7fff	(the 5V ref value is close to 0x7fff)
			 *
			 * Plugging in our input ranges, we get an output range of 0 - 0x12490,
			 * which is 17 bits. That won't work. If we take the accel ref and shift
			 * by a bit, we'll change its range:
			 *
			 *	0xe000 <= ao_accel_ref<<1 <= 0xfffe
			 *
			 *	ao_cook_accel = (ao_raw_accel << 16) / (ao_accel_ref << 1)
			 *
			 * Now the output range is 0 - 0x9248, which nicely fits in 16 bits. It
			 * is, however, one bit too large for our signed computations. So, we
			 * take the result and shift that by a bit:
			 *
			 *	ao_cook_accel = ((ao_raw_accel << 16) / (ao_accel_ref << 1)) >> 1
			 *
			 * This finally creates an output range of 0 - 0x4924. As the ADC only
			 * provides 11 bits of data, we haven't actually lost any precision,
			 * just dropped a bit of noise off the low end.
			 */
			ao_raw_accel = (uint16_t) ((((uint32_t) ao_raw_accel << 16) / (ao_accel_ref[ao_flight_adc] << 1))) >> 1;
			ao_adc->accel = ao_raw_accel;
#endif

			ao_flight_accel -= ao_flight_accel >> 4;
			ao_flight_accel += ao_raw_accel >> 4;
			/* Update velocity
			 *
			 * The accelerometer is mounted so that
			 * acceleration yields negative values
			 * while deceleration yields positive values,
			 * so subtract instead of add.
			 */
			ticks = ao_flight_tick - ao_flight_prev_tick;
			ao_vel_change = ao_ground_accel - (((ao_raw_accel + 1) >> 1) + ((ao_raw_accel_prev + 1) >> 1));
			ao_raw_accel_prev = ao_raw_accel;

			/* one is a common interval */
			if (ticks == 1)
				ao_flight_vel += (int32_t) ao_vel_change;
			else
				ao_flight_vel += (int32_t) ao_vel_change * (int32_t) ticks;
#endif

			ao_flight_adc = ao_adc_ring_next(ao_flight_adc);
		}

		if (ao_flight_pres < ao_min_pres)
			ao_min_pres = ao_flight_pres;
#if HAS_ACCEL
		if (ao_flight_vel >= 0) {
			if (ao_flight_vel < ao_min_vel)
			    ao_min_vel = ao_flight_vel;
		} else {
			if (-ao_flight_vel < ao_min_vel)
			    ao_min_vel = -ao_flight_vel;
		}
#endif

		switch (ao_flight_state) {
		case ao_flight_startup:

			/* startup state:
			 *
			 * Collect 512 samples of acceleration and pressure
			 * data and average them to find the resting values
			 */
			if (nsamples < 512) {
#if HAS_ACCEL
				ao_raw_accel_sum += ao_raw_accel;
#endif
				ao_raw_pres_sum += ao_raw_pres;
				++nsamples;
				continue;
			}
#if HAS_ACCEL
			ao_ground_accel = ao_raw_accel_sum >> 9;
#endif
			ao_ground_pres = ao_raw_pres_sum >> 9;
			ao_min_pres = ao_ground_pres;
			ao_config_get();
			ao_main_pres = ao_altitude_to_pres(ao_pres_to_altitude(ao_ground_pres) + ao_config.main_deploy);
#if HAS_ACCEL
			ao_accel_2g = ao_config.accel_minus_g - ao_config.accel_plus_g;
			ao_flight_vel = 0;
			ao_min_vel = 0;
			ao_old_vel = ao_flight_vel;
			ao_old_vel_tick = ao_flight_tick;
#endif

			/* Check to see what mode we should go to.
			 *  - Invalid mode if accel cal appears to be out
			 *  - pad mode if we're upright,
			 *  - idle mode otherwise
			 */
			ao_config_get();
#if HAS_ACCEL
			if (ao_config.accel_plus_g == 0 ||
			    ao_config.accel_minus_g == 0 ||
			    ao_flight_accel < ao_config.accel_plus_g - ACCEL_NOSE_UP ||
			    ao_flight_accel > ao_config.accel_minus_g + ACCEL_NOSE_UP)
			{
				/* Detected an accel value outside -1.5g to 1.5g
				 * (or uncalibrated values), so we go into invalid mode
				 */
				ao_flight_state = ao_flight_invalid;

			} else
#endif
				if (!ao_flight_force_idle
#if HAS_ACCEL
				    && ao_flight_accel < ao_config.accel_plus_g + ACCEL_NOSE_UP
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

				/* Disable packet mode in pad state */
				ao_packet_slave_stop();

				/* Turn on telemetry system */
				ao_rdf_set(1);
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_PAD);

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

#if HAS_ACCEL
			/* Trim velocity
			 *
			 * Once a second, remove any velocity from
			 * a second ago
			 */
			if ((int16_t) (ao_flight_tick - ao_old_vel_tick) >= AO_SEC_TO_TICKS(1)) {
				ao_old_vel_tick = ao_flight_tick;
				ao_flight_vel -= ao_old_vel;
				ao_old_vel = ao_flight_vel;
			}
#endif
			/* pad to boost:
			 *
			 * accelerometer: > 2g AND velocity > 5m/s
			 *             OR
			 * barometer: > 20m vertical motion
			 *
			 * The accelerometer should always detect motion before
			 * the barometer, but we use both to make sure this
			 * transition is detected
			 */
			if (
#if HAS_ACCEL
				(ao_flight_accel < ao_ground_accel - ACCEL_BOOST &&
				 ao_flight_vel > ACCEL_VEL_BOOST) ||
#endif
			    ao_flight_pres < ao_ground_pres - BARO_LAUNCH)
			{
#if HAS_ACCEL
				ao_flight_state = ao_flight_boost;
#else
				ao_flight_state = ao_flight_coast;
#endif
				ao_launch_tick = ao_flight_tick;

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
				break;
			}
			break;
#if HAS_ACCEL
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
			if (ao_flight_accel > ao_ground_accel + ACCEL_COAST ||
			    (int16_t) (ao_flight_tick - ao_launch_tick) > BOOST_TICKS_MAX)
			{
				ao_flight_state = ao_flight_fast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				break;
			}
			break;
		case ao_flight_fast:

			/* fast to coast:
			 *
			 * accelerometer: integrated velocity < 200 m/s
			 *               OR
			 * barometer: fall at least 500m from max altitude
			 *
			 * This extra state is required to avoid mis-detecting
			 * apogee due to mach transitions.
			 *
			 * XXX this is essentially a single-detector test
			 * as the 500m altitude change would likely result
			 * in a loss of the rocket. More data on precisely
			 * how big a pressure change the mach transition
			 * generates would be useful here.
			 */
			if (ao_flight_vel < ACCEL_VEL_MACH ||
			    ao_flight_pres > ao_min_pres + BARO_COAST)
			{
				/* set min velocity to current velocity for
				 * apogee detect
				 */
				ao_min_vel = abs(ao_flight_vel);
				ao_flight_state = ao_flight_coast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
			break;
#endif
		case ao_flight_coast:

			/* apogee detect: coast to drogue deploy:
			 *
			 * barometer: fall at least 10m
			 *
			 * It would be nice to use the accelerometer
			 * to detect apogee as well, but tests have
			 * shown that flights far from vertical would
			 * grossly mis-detect apogee. So, for now,
			 * we'll trust to a single sensor for this test
			 */
			if (ao_flight_pres > ao_min_pres + BARO_APOGEE)
			{
				/* ignite the drogue charge */
				ao_ignite(ao_igniter_drogue);

				/* slow down the telemetry system */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_RECOVER);

				/* slow down the ADC sample rate */
				ao_timer_set_adc_interval(10);

				/*
				 * Start recording min/max accel and pres for a while
				 * to figure out when the rocket has landed
				 */
				/* Set the 'last' limits to max range to prevent
				 * early resting detection
				 */
#if HAS_ACCEL
				ao_interval_min_accel = 0;
				ao_interval_max_accel = 0x7fff;
#endif
				ao_interval_min_pres = 0;
				ao_interval_max_pres = 0x7fff;

				/* initialize interval values */
				ao_interval_end = ao_flight_tick + AO_INTERVAL_TICKS;

				ao_interval_cur_min_pres = ao_interval_cur_max_pres = ao_flight_pres;
#if HAS_ACCEL
				ao_interval_cur_min_accel = ao_interval_cur_max_accel = ao_flight_accel;
#endif

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
			if (ao_flight_pres >= ao_main_pres)
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
			 *                           AND
			 * barometer: altitude stable and within 1000m of the launch altitude
			 */

			if (ao_flight_pres < ao_interval_cur_min_pres)
				ao_interval_cur_min_pres = ao_flight_pres;
			if (ao_flight_pres > ao_interval_cur_max_pres)
				ao_interval_cur_max_pres = ao_flight_pres;
#if HAS_ACCEL
			if (ao_flight_accel < ao_interval_cur_min_accel)
				ao_interval_cur_min_accel = ao_flight_accel;
			if (ao_flight_accel > ao_interval_cur_max_accel)
				ao_interval_cur_max_accel = ao_flight_accel;
#endif

			if ((int16_t) (ao_flight_tick - ao_interval_end) >= 0) {
				ao_interval_max_pres = ao_interval_cur_max_pres;
				ao_interval_min_pres = ao_interval_cur_min_pres;
				ao_interval_cur_min_pres = ao_interval_cur_max_pres = ao_flight_pres;
#if HAS_ACCEL
				ao_interval_max_accel = ao_interval_cur_max_accel;
				ao_interval_min_accel = ao_interval_cur_min_accel;
				ao_interval_cur_min_accel = ao_interval_cur_max_accel = ao_flight_accel;
#endif
				ao_interval_end = ao_flight_tick + AO_INTERVAL_TICKS;

				if (
#if HAS_ACCEL
					(uint16_t) (ao_interval_max_accel - ao_interval_min_accel) < (uint16_t) ACCEL_INT_LAND &&
#endif
				    ao_flight_pres > ao_ground_pres - BARO_LAND &&
				    (uint16_t) (ao_interval_max_pres - ao_interval_min_pres) < (uint16_t) BARO_INT_LAND)
				{
					ao_flight_state = ao_flight_landed;

					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);
					/* Enable RDF beacon */
					ao_rdf_set(1);

					ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				}
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
