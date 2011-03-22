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
__xdata int16_t			ao_ground_pres;		/* startup pressure */
__pdata uint16_t		ao_launch_tick;		/* time of launch detect */
#if HAS_ACCEL
__pdata int16_t			ao_ground_accel;	/* startup acceleration */
#endif

/*
 * track min/max data over a long interval to detect
 * resting
 */
__pdata uint16_t		ao_interval_end;
__pdata int16_t			ao_interval_min_height;
__pdata int16_t			ao_interval_max_height;

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

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

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

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

#include "ao_kalman.h"

__pdata int16_t			ao_ground_height;
__pdata int16_t			ao_height;
__pdata int16_t			ao_speed;
__pdata int16_t			ao_accel;
__pdata int16_t			ao_max_height;

static __pdata int32_t		ao_k_height;
static __pdata int32_t		ao_k_speed;
static __pdata int32_t		ao_k_accel;

#define AO_K_STEP_100		to_fix16(0.01)
#define AO_K_STEP_2_2_100	to_fix16(0.00005)

#define AO_K_STEP_10		to_fix16(0.1)
#define AO_K_STEP_2_2_10	to_fix16(0.005)

/*
 * Above this height, the baro sensor doesn't work
 */
#define AO_MAX_BARO_HEIGHT	12000

/*
 * Above this speed, baro measurements are unreliable
 */
#define AO_MAX_BARO_SPEED	300

static void
ao_kalman_predict(void)
{
#ifdef AO_FLIGHT_TEST
	if (ao_flight_tick - ao_flight_prev_tick > 5) {
		ao_k_height += ((int32_t) ao_speed * AO_K_STEP_10 +
				(int32_t) ao_accel * AO_K_STEP_2_2_10) >> 4;
		ao_k_speed += (int32_t) ao_accel * AO_K_STEP_10;

		return;
	}
#endif
	ao_k_height += ((int32_t) ao_speed * AO_K_STEP_100 +
			(int32_t) ao_accel * AO_K_STEP_2_2_100) >> 4;
	ao_k_speed += (int32_t) ao_accel * AO_K_STEP_100;
}

static __pdata int16_t ao_error_h;
static __pdata int16_t ao_raw_alt;
static __pdata int16_t ao_raw_height;
static __pdata int16_t ao_error_h_sq_avg;

static void
ao_kalman_err_height(void)
{
	int16_t	e;
	int16_t height_distrust;
#if HAS_ACCEL
	int16_t	speed_distrust;
#endif

	ao_error_h = ao_raw_height - (int16_t) (ao_k_height >> 16);

	e = ao_error_h;
	if (e < 0)
		e = -e;
	if (e > 127)
		e = 127;
	ao_error_h_sq_avg -= ao_error_h_sq_avg >> 4;
	ao_error_h_sq_avg += (e * e) >> 4;

	height_distrust = ao_raw_height - AO_MAX_BARO_HEIGHT;
#if HAS_ACCEL
	speed_distrust = (ao_speed - AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) >> 4;
	if (speed_distrust <= 0)
		speed_distrust = 0;
	else if (speed_distrust > height_distrust)
		height_distrust = speed_distrust;
#endif
	if (height_distrust <= 0)
		height_distrust = 0;

	if (height_distrust) {
		if (height_distrust > 0x100)
			height_distrust = 0x100;
		ao_error_h = (int16_t) ((int32_t) ao_error_h * (0x100 - height_distrust)) >> 8;
	}
}

static void
ao_kalman_correct_baro(void)
{
	ao_kalman_err_height();
#ifdef AO_FLIGHT_TEST
	if (ao_flight_tick - ao_flight_prev_tick > 5) {
		ao_k_height += (int32_t) AO_BARO_K0_10 * ao_error_h;
		ao_k_speed  += (int32_t) AO_BARO_K1_10 * ao_error_h;
		ao_k_accel  += (int32_t) AO_BARO_K2_10 * ao_error_h;
		return;
	}
#endif
	ao_k_height += (int32_t) AO_BARO_K0_100 * ao_error_h;
	ao_k_speed  += (int32_t) AO_BARO_K1_100 * ao_error_h;
	ao_k_accel  += (int32_t) AO_BARO_K2_100 * ao_error_h;
}

#if HAS_ACCEL
static __pdata int16_t ao_error_a;
static __pdata int32_t ao_accel_scale;

static void
ao_kalman_err_accel(void)
{
	int32_t	accel;

	accel = (ao_ground_accel - ao_raw_accel) * ao_accel_scale;

	/* Can't use ao_accel here as it is the pre-prediction value still */
	ao_error_a = (accel - ao_k_accel) >> 16;
}

static void
ao_kalman_correct_both(void)
{
	ao_kalman_err_height();
	ao_kalman_err_accel();

#ifdef AO_FLIGHT_TEST
	if (ao_flight_tick - ao_flight_prev_tick > 5) {
		ao_k_height +=
			(int32_t) AO_BOTH_K00_10 * ao_error_h +
			(int32_t) (AO_BOTH_K01_10 >> 4) * ao_error_a;
		ao_k_speed +=
			((int32_t) AO_BOTH_K10_10 << 4) * ao_error_h +
			(int32_t) AO_BOTH_K11_10 * ao_error_a;
		ao_k_accel +=
			((int32_t) AO_BOTH_K20_10 << 4) * ao_error_h +
			(int32_t) AO_BOTH_K21_10 * ao_error_a;
		return;
	}
#endif
	ao_k_height +=
		(int32_t) AO_BOTH_K00_100 * ao_error_h +
		(int32_t) AO_BOTH_K01_100 * ao_error_a;
	ao_k_speed +=
		(int32_t) AO_BOTH_K10_100 * ao_error_h +
		(int32_t) AO_BOTH_K11_100 * ao_error_a;
	ao_k_accel +=
		(int32_t) AO_BOTH_K20_100 * ao_error_h +
		(int32_t) AO_BOTH_K21_100 * ao_error_a;
}

static void
ao_kalman_correct_accel(void)
{
	ao_kalman_err_accel();

#ifdef AO_FLIGHT_TEST
	if (ao_flight_tick - ao_flight_prev_tick > 5) {
		ao_k_height +=(int32_t) AO_ACCEL_K0_10 * ao_error_a;
		ao_k_speed  += (int32_t) AO_ACCEL_K1_10 * ao_error_a;
		ao_k_accel  += (int32_t) AO_ACCEL_K2_10 * ao_error_a;
		return;
	}
#endif
	ao_k_height += (int32_t) AO_ACCEL_K0_100 * ao_error_a;
	ao_k_speed  += (int32_t) AO_ACCEL_K1_100 * ao_error_a;
	ao_k_accel  += (int32_t) AO_ACCEL_K2_100 * ao_error_a;
}
#endif /* HAS_ACCEL */

__xdata int32_t ao_raw_pres_sum;

#ifdef HAS_ACCEL
__xdata int32_t ao_raw_accel_sum;
#endif

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
			__xdata struct ao_adc *ao_adc;
			ao_flight_prev_tick = ao_flight_tick;

			/* Capture a sample */
			ao_adc = &ao_adc_ring[ao_flight_adc];
			ao_flight_tick = ao_adc->tick;
			ao_raw_pres = ao_adc->pres;
			ao_raw_alt = ao_pres_to_altitude(ao_raw_pres);
			ao_raw_height = ao_raw_alt - ao_ground_height;
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
#endif

			if (ao_flight_state > ao_flight_idle) {
				ao_kalman_predict();
#if HAS_ACCEL
				if (ao_flight_state <= ao_flight_coast) {
#ifdef FORCE_ACCEL
					ao_kalman_correct_accel();
#else
					ao_kalman_correct_both();
#endif
				} else
#endif
					ao_kalman_correct_baro();
				ao_height = from_fix(ao_k_height);
				ao_speed = from_fix(ao_k_speed);
				ao_accel = from_fix(ao_k_accel);
				if (ao_height > ao_max_height)
					ao_max_height = ao_height;
			}
			ao_flight_adc = ao_adc_ring_next(ao_flight_adc);
		}

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
			ao_config_get();
#if HAS_ACCEL
			ao_ground_accel = ao_raw_accel_sum >> 9;
			ao_accel_2g = ao_config.accel_minus_g - ao_config.accel_plus_g;
			ao_accel_scale = to_fix32(GRAVITY * 2 * 16) / ao_accel_2g;
#endif
			ao_ground_pres = ao_raw_pres_sum >> 9;
			ao_ground_height = ao_pres_to_altitude(ao_ground_pres);

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
			    (int16_t) (ao_flight_tick - ao_launch_tick) > BOOST_TICKS_MAX)
			{
				ao_flight_state = ao_flight_fast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				break;
			}
			break;
		case ao_flight_fast:
			/*
			 * This is essentially the same as coast,
			 * but the barometer is being ignored as
			 * it may be unreliable.
			 */
			if (ao_speed < AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) {
				ao_flight_state = ao_flight_coast;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				break;
			}
			break;
		case ao_flight_coast:

			/* apogee detect: coast to drogue deploy:
			 *
			 * speed: < 0
			 *
			 * Also make sure the model altitude is tracking
			 * the measured altitude reasonably closely; otherwise
			 * we're probably transsonic.
			 */
			if (ao_speed < 0 && (ao_raw_alt >= AO_MAX_BARO_HEIGHT || ao_error_h_sq_avg < 100))
			{
				/* ignite the drogue charge */
				ao_ignite(ao_igniter_drogue);

				/* slow down the telemetry system */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_RECOVER);

				/*
				 * Start recording min/max height
				 * to figure out when the rocket has landed
				 */

				/* initialize interval values */
				ao_interval_end = ao_flight_tick + AO_INTERVAL_TICKS;

				ao_interval_min_height = ao_interval_max_height = ao_height;

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
				ao_flight_state = ao_flight_main;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}

			/* fall through... */
		case ao_flight_main:

			/* drogue/main to land:
			 *
			 * barometer: altitude stable and within 1000m of the launch altitude
			 */

			if (ao_height < ao_interval_min_height)
				ao_interval_min_height = ao_height;
			if (ao_height > ao_interval_max_height)
				ao_interval_max_height = ao_height;

			if ((int16_t) (ao_flight_tick - ao_interval_end) >= 0) {
				if (ao_height < AO_M_TO_HEIGHT(1000) &&
				    ao_interval_max_height - ao_interval_min_height < AO_M_TO_HEIGHT(5))
				{
					ao_flight_state = ao_flight_landed;

					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);
					/* Enable RDF beacon */
					ao_rdf_set(1);

					ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
				}
				ao_interval_min_height = ao_interval_max_height = ao_height;
				ao_interval_end = ao_flight_tick + AO_INTERVAL_TICKS;
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
