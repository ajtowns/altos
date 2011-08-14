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

#ifndef AO_FLIGHT_TEST
#include "ao.h"
#endif

#include "ao_kalman.h"

static __pdata int32_t		ao_k_height;
static __pdata int32_t		ao_k_speed;
static __pdata int32_t		ao_k_accel;

#define AO_K_STEP_100		to_fix16(0.01)
#define AO_K_STEP_2_2_100	to_fix16(0.00005)

#define AO_K_STEP_10		to_fix16(0.1)
#define AO_K_STEP_2_2_10	to_fix16(0.005)

#define AO_K_STEP_1		to_fix16(1)
#define AO_K_STEP_2_2_1		to_fix16(0.5)

__pdata int16_t			ao_height;
__pdata int16_t			ao_speed;
__pdata int16_t			ao_accel;
__pdata int16_t			ao_max_height;
static __pdata int32_t		ao_avg_height_scaled;
__pdata int16_t			ao_avg_height;

__pdata int16_t			ao_error_h;
__pdata int16_t			ao_error_h_sq_avg;

#if HAS_ACCEL
__pdata int16_t			ao_error_a;
#endif

static void
ao_kalman_predict(void)
{
#ifdef AO_FLIGHT_TEST
	if (ao_sample_tick - ao_sample_prev_tick > 50) {
		ao_k_height += ((int32_t) ao_speed * AO_K_STEP_1 +
				(int32_t) ao_accel * AO_K_STEP_2_2_1) >> 4;
		ao_k_speed += (int32_t) ao_accel * AO_K_STEP_1;

		return;
	}
	if (ao_sample_tick - ao_sample_prev_tick > 5) {
		ao_k_height += ((int32_t) ao_speed * AO_K_STEP_10 +
				(int32_t) ao_accel * AO_K_STEP_2_2_10) >> 4;
		ao_k_speed += (int32_t) ao_accel * AO_K_STEP_10;

		return;
	}
	if (ao_flight_debug) {
		printf ("predict speed %g + (%g * %g) = %g\n",
			ao_k_speed / (65536.0 * 16.0), ao_accel / 16.0, AO_K_STEP_100 / 65536.0,
			(ao_k_speed + (int32_t) ao_accel * AO_K_STEP_100) / (65536.0 * 16.0));
	}
#endif
	ao_k_height += ((int32_t) ao_speed * AO_K_STEP_100 +
			(int32_t) ao_accel * AO_K_STEP_2_2_100) >> 4;
	ao_k_speed += (int32_t) ao_accel * AO_K_STEP_100;
}

static void
ao_kalman_err_height(void)
{
	int16_t	e;
	int16_t height_distrust;
#if HAS_ACCEL
	int16_t	speed_distrust;
#endif

	ao_error_h = ao_sample_height - (int16_t) (ao_k_height >> 16);

	e = ao_error_h;
	if (e < 0)
		e = -e;
	if (e > 127)
		e = 127;
#if HAS_ACCEL
	ao_error_h_sq_avg -= ao_error_h_sq_avg >> 2;
	ao_error_h_sq_avg += (e * e) >> 2;
#else
	ao_error_h_sq_avg -= ao_error_h_sq_avg >> 4;
	ao_error_h_sq_avg += (e * e) >> 4;
#endif

	if (ao_flight_state >= ao_flight_drogue)
		return;
	height_distrust = ao_sample_alt - AO_MAX_BARO_HEIGHT;
#if HAS_ACCEL
	/* speed is stored * 16, but we need to ramp between 200 and 328, so
	 * we want to multiply by 2. The result is a shift by 3.
	 */
	speed_distrust = (ao_speed - AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) >> (4 - 1);
	if (speed_distrust <= 0)
		speed_distrust = 0;
	else if (speed_distrust > height_distrust)
		height_distrust = speed_distrust;
#endif
	if (height_distrust > 0) {
#ifdef AO_FLIGHT_TEST
		int	old_ao_error_h = ao_error_h;
#endif
		if (height_distrust > 0x100)
			height_distrust = 0x100;
		ao_error_h = (int16_t) (((int32_t) ao_error_h * (0x100 - height_distrust)) >> 8);
#ifdef AO_FLIGHT_TEST
		if (ao_flight_debug) {
			printf("over height %g over speed %g distrust: %g height: error %d -> %d\n",
			       (double) (ao_sample_alt - AO_MAX_BARO_HEIGHT),
			       (ao_speed - AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) / 16.0,
			       height_distrust / 256.0,
			       old_ao_error_h, ao_error_h);
		}
#endif
	}
}

static void
ao_kalman_correct_baro(void)
{
	ao_kalman_err_height();
#ifdef AO_FLIGHT_TEST
	if (ao_sample_tick - ao_sample_prev_tick > 50) {
		ao_k_height += (int32_t) AO_BARO_K0_1 * ao_error_h;
		ao_k_speed  += (int32_t) AO_BARO_K1_1 * ao_error_h;
		ao_k_accel  += (int32_t) AO_BARO_K2_1 * ao_error_h;
		return;
	}
	if (ao_sample_tick - ao_sample_prev_tick > 5) {
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

static void
ao_kalman_err_accel(void)
{
	int32_t	accel;

	accel = (ao_ground_accel - ao_sample_accel) * ao_accel_scale;

	/* Can't use ao_accel here as it is the pre-prediction value still */
	ao_error_a = (accel - ao_k_accel) >> 16;
}

static void
ao_kalman_correct_both(void)
{
	ao_kalman_err_height();
	ao_kalman_err_accel();

#ifdef AO_FLIGHT_TEST
	if (ao_sample_tick - ao_sample_prev_tick > 50) {
		if (ao_flight_debug) {
			printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
				ao_k_speed / (65536.0 * 16.0),
				(double) ao_error_h, AO_BOTH_K10_1 / 65536.0,
				(double) ao_error_a, AO_BOTH_K11_1 / 65536.0,
				(ao_k_speed +
				 (int32_t) AO_BOTH_K10_1 * ao_error_h +
				 (int32_t) AO_BOTH_K11_1 * ao_error_a) / (65536.0 * 16.0));
		}
		ao_k_height +=
			(int32_t) AO_BOTH_K00_1 * ao_error_h +
			(int32_t) AO_BOTH_K01_1 * ao_error_a;
		ao_k_speed +=
			(int32_t) AO_BOTH_K10_1 * ao_error_h +
			(int32_t) AO_BOTH_K11_1 * ao_error_a;
		ao_k_accel +=
			(int32_t) AO_BOTH_K20_1 * ao_error_h +
			(int32_t) AO_BOTH_K21_1 * ao_error_a;
		return;
	}
	if (ao_sample_tick - ao_sample_prev_tick > 5) {
		if (ao_flight_debug) {
			printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
				ao_k_speed / (65536.0 * 16.0),
				(double) ao_error_h, AO_BOTH_K10_10 / 65536.0,
				(double) ao_error_a, AO_BOTH_K11_10 / 65536.0,
				(ao_k_speed +
				 (int32_t) AO_BOTH_K10_10 * ao_error_h +
				 (int32_t) AO_BOTH_K11_10 * ao_error_a) / (65536.0 * 16.0));
		}
		ao_k_height +=
			(int32_t) AO_BOTH_K00_10 * ao_error_h +
			(int32_t) AO_BOTH_K01_10 * ao_error_a;
		ao_k_speed +=
			(int32_t) AO_BOTH_K10_10 * ao_error_h +
			(int32_t) AO_BOTH_K11_10 * ao_error_a;
		ao_k_accel +=
			(int32_t) AO_BOTH_K20_10 * ao_error_h +
			(int32_t) AO_BOTH_K21_10 * ao_error_a;
		return;
	}
	if (ao_flight_debug) {
		printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
			ao_k_speed / (65536.0 * 16.0),
			(double) ao_error_h, AO_BOTH_K10_100 / 65536.0,
			(double) ao_error_a, AO_BOTH_K11_100 / 65536.0,
			(ao_k_speed +
			 (int32_t) AO_BOTH_K10_100 * ao_error_h +
			 (int32_t) AO_BOTH_K11_100 * ao_error_a) / (65536.0 * 16.0));
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

#ifdef FORCE_ACCEL
static void
ao_kalman_correct_accel(void)
{
	ao_kalman_err_accel();

	if (ao_sample_tick - ao_sample_prev_tick > 5) {
		ao_k_height +=(int32_t) AO_ACCEL_K0_10 * ao_error_a;
		ao_k_speed  += (int32_t) AO_ACCEL_K1_10 * ao_error_a;
		ao_k_accel  += (int32_t) AO_ACCEL_K2_10 * ao_error_a;
		return;
	}
	ao_k_height += (int32_t) AO_ACCEL_K0_100 * ao_error_a;
	ao_k_speed  += (int32_t) AO_ACCEL_K1_100 * ao_error_a;
	ao_k_accel  += (int32_t) AO_ACCEL_K2_100 * ao_error_a;
}
#endif
#endif /* HAS_ACCEL */

void
ao_kalman(void)
{
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
	ao_avg_height_scaled = ao_avg_height_scaled - ao_avg_height + ao_sample_height;
#ifdef AO_FLIGHT_TEST
	if (ao_sample_tick - ao_sample_prev_tick > 50)
		ao_avg_height = (ao_avg_height_scaled + 1) >> 1;
	else if (ao_sample_tick - ao_sample_prev_tick > 5)
		ao_avg_height = (ao_avg_height_scaled + 7) >> 4;
	else 
#endif
		ao_avg_height = (ao_avg_height_scaled + 63) >> 7;
#ifdef AO_FLIGHT_TEST
	ao_sample_prev_tick = ao_sample_tick;
#endif
}
