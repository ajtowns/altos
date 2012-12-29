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
#include <ao_data.h>
#endif

/*
 * Current sensor values
 */

#ifndef PRES_TYPE
#define PRES_TYPE int32_t
#define ALT_TYPE int32_t
#define ACCEL_TYPE int16_t
#endif

__pdata uint16_t	ao_sample_tick;		/* time of last data */
__pdata pres_t		ao_sample_pres;
__pdata alt_t		ao_sample_alt;
__pdata alt_t		ao_sample_height;
#if HAS_ACCEL
__pdata accel_t		ao_sample_accel;
#endif
#if HAS_GYRO
__pdata accel_t		ao_sample_accel_along;
__pdata accel_t		ao_sample_accel_across;
__pdata accel_t		ao_sample_accel_through;
__pdata gyro_t		ao_sample_roll;
__pdata gyro_t		ao_sample_pitch;
__pdata gyro_t		ao_sample_yaw;
__pdata angle_t		ao_sample_angle;
__pdata angle_t		ao_sample_roll_angle;
#endif

__data uint8_t		ao_sample_data;

/*
 * Sensor calibration values
 */

__pdata pres_t		ao_ground_pres;		/* startup pressure */
__pdata alt_t		ao_ground_height;	/* MSL of ao_ground_pres */

#if HAS_ACCEL
__pdata accel_t		ao_ground_accel;	/* startup acceleration */
__pdata accel_t		ao_accel_2g;		/* factory accel calibration */
__pdata int32_t		ao_accel_scale;		/* sensor to m/s² conversion */
#endif

#if HAS_GYRO
__pdata accel_t		ao_ground_accel_along;
__pdata accel_t		ao_ground_accel_across;
__pdata accel_t		ao_ground_accel_through;
__pdata gyro_t		ao_ground_pitch;
__pdata gyro_t		ao_ground_yaw;
__pdata gyro_t		ao_ground_roll;
#endif

static __pdata uint8_t	ao_preflight;		/* in preflight mode */

static __pdata uint16_t	nsamples;
__pdata int32_t ao_sample_pres_sum;
#if HAS_ACCEL
__pdata int32_t ao_sample_accel_sum;
#endif
#if HAS_GYRO
__pdata int32_t ao_sample_accel_along_sum;
__pdata int32_t ao_sample_accel_across_sum;
__pdata int32_t	ao_sample_accel_through_sum;
__pdata int32_t ao_sample_pitch_sum;
__pdata int32_t ao_sample_yaw_sum;
__pdata int32_t	ao_sample_roll_sum;
#endif

static void
ao_sample_preflight_add(void)
{
#if HAS_ACCEL
	ao_sample_accel_sum += ao_sample_accel;
#endif
	ao_sample_pres_sum += ao_sample_pres;
#if HAS_GYRO
	ao_sample_accel_along_sum += ao_sample_accel_along;
	ao_sample_accel_across_sum += ao_sample_accel_across;
	ao_sample_accel_through_sum += ao_sample_accel_through;
	ao_sample_pitch_sum += ao_sample_pitch;
	ao_sample_yaw_sum += ao_sample_yaw;
	ao_sample_roll_sum += ao_sample_roll;
#endif
	++nsamples;
}

static void
ao_sample_preflight_set(void)
{
#if HAS_ACCEL
	ao_ground_accel = ao_sample_accel_sum >> 9;
	ao_sample_accel_sum = 0;
#endif
	ao_ground_pres = ao_sample_pres_sum >> 9;
	ao_ground_height = pres_to_altitude(ao_ground_pres);
	ao_sample_pres_sum = 0;
#if HAS_GYRO
	ao_ground_accel_along = ao_sample_accel_along_sum >> 9;
	ao_ground_accel_across = ao_sample_accel_across_sum >> 9;
	ao_ground_accel_through = ao_sample_accel_through_sum >> 9;
	ao_ground_pitch = ao_sample_pitch_sum >> 9;
	ao_ground_yaw = ao_sample_yaw_sum >> 9;
	ao_ground_roll = ao_sample_roll_sum >> 9;
	ao_sample_accel_along_sum = 0;
	ao_sample_accel_across_sum = 0;
	ao_sample_accel_through_sum = 0;
	ao_sample_pitch_sum = 0;
	ao_sample_yaw_sum = 0;
	ao_sample_roll_sum = 0;
	ao_sample_angle = 0;
#endif	
	nsamples = 0;
}

static void
ao_sample_preflight(void)
{
	/* startup state:
	 *
	 * Collect 512 samples of acceleration and pressure
	 * data and average them to find the resting values
	 */
	if (nsamples < 512) {
		ao_sample_preflight_add();
	} else {
#if HAS_ACCEL
		ao_accel_2g = ao_config.accel_minus_g - ao_config.accel_plus_g;
		ao_accel_scale = to_fix32(GRAVITY * 2 * 16) / ao_accel_2g;
#endif
		ao_sample_preflight_set();
		ao_preflight = FALSE;
	}
}

/*
 * While in pad mode, constantly update the ground state by
 * re-averaging the data.  This tracks changes in orientation, which
 * might be caused by adjustments to the rocket on the pad and
 * pressure, which might be caused by changes in the weather.
 */

static void
ao_sample_preflight_update(void)
{
	if (nsamples < 512)
		ao_sample_preflight_add();
	else if (nsamples < 1024)
		++nsamples;
	else
		ao_sample_preflight_set();
}

#if 0
#if HAS_GYRO
static int32_t	p_filt;
static int32_t	y_filt;

static gyro_t inline ao_gyro(void) {
	gyro_t	p = ao_sample_pitch - ao_ground_pitch;
	gyro_t	y = ao_sample_yaw - ao_ground_yaw;

	p_filt = p_filt - (p_filt >> 6) + p;
	y_filt = y_filt - (y_filt >> 6) + y;

	p = p_filt >> 6;
	y = y_filt >> 6;
	return ao_sqrt(p*p + y*y);
}
#endif
#endif

uint8_t
ao_sample(void)
{
	ao_wakeup(DATA_TO_XDATA(&ao_sample_data));
	ao_sleep((void *) DATA_TO_XDATA(&ao_data_head));
	while (ao_sample_data != ao_data_head) {
		__xdata struct ao_data *ao_data;

		/* Capture a sample */
		ao_data = (struct ao_data *) &ao_data_ring[ao_sample_data];
		ao_sample_tick = ao_data->tick;

#if HAS_BARO
		ao_data_pres_cook(ao_data);
		ao_sample_pres = ao_data_pres(ao_data);
		ao_sample_alt = pres_to_altitude(ao_sample_pres);
		ao_sample_height = ao_sample_alt - ao_ground_height;
#endif

#if HAS_ACCEL
		ao_sample_accel = ao_data_accel_cook(ao_data);
		if (ao_config.pad_orientation != AO_PAD_ORIENTATION_ANTENNA_UP)
			ao_sample_accel = ao_data_accel_invert(ao_sample_accel);
		ao_data_set_accel(ao_data, ao_sample_accel);
#endif
#if HAS_GYRO
		ao_sample_accel_along = ao_data_along(ao_data);
		ao_sample_accel_across = ao_data_across(ao_data);
		ao_sample_accel_through = ao_data_through(ao_data);
		ao_sample_pitch = ao_data_pitch(ao_data);
		ao_sample_yaw = ao_data_yaw(ao_data);
		ao_sample_roll = ao_data_roll(ao_data);
#endif

		if (ao_preflight)
			ao_sample_preflight();
		else {
			if (ao_flight_state < ao_flight_boost)
				ao_sample_preflight_update();
			ao_kalman();
#if HAS_GYRO
			/* do quaternion stuff here... */
#endif
		}
		ao_sample_data = ao_data_ring_next(ao_sample_data);
	}
	return !ao_preflight;
}

void
ao_sample_init(void)
{
	ao_config_get();
	nsamples = 0;
	ao_sample_pres_sum = 0;
	ao_sample_pres = 0;
#if HAS_ACCEL
	ao_sample_accel_sum = 0;
	ao_sample_accel = 0;
#endif
#if HAS_GYRO
	ao_sample_accel_along_sum = 0;
	ao_sample_accel_across_sum = 0;
	ao_sample_accel_through_sum = 0;
	ao_sample_accel_along = 0;
	ao_sample_accel_across = 0;
	ao_sample_accel_through = 0;
	ao_sample_pitch_sum = 0;
	ao_sample_yaw_sum = 0;
	ao_sample_roll_sum = 0;
	ao_sample_pitch = 0;
	ao_sample_yaw = 0;
	ao_sample_roll = 0;
	ao_sample_angle = 0;
#endif
	ao_sample_data = ao_data_head;
	ao_preflight = TRUE;
}
