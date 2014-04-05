/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_SAMPLE_H_
#define _AO_SAMPLE_H_

#include <ao_data.h>

/*
 * ao_sample.c
 */

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

/*
 * Above this height, the baro sensor doesn't work
 */
#if HAS_MS5607
#define AO_MAX_BARO_HEIGHT	30000
#else
#define AO_MAX_BARO_HEIGHT	12000
#endif

/*
 * Above this speed, baro measurements are unreliable
 */
#define AO_MAX_BARO_SPEED	200

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

/*
 * Speed and acceleration are scaled by 16 to provide a bit more
 * resolution while still having reasonable range. Note that this
 * limits speed to 2047m/s (around mach 6) and acceleration to
 * 2047m/s² (over 200g)
 */

#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))

extern __pdata uint16_t	ao_sample_tick;		/* time of last data */
extern __data uint8_t	ao_sample_adc;		/* Ring position of last processed sample */
extern __data uint8_t	ao_sample_data;		/* Ring position of last processed sample */

#if HAS_BARO
extern __pdata pres_t	ao_sample_pres;		/* most recent pressure sensor reading */
extern __pdata alt_t	ao_sample_alt;		/* MSL of ao_sample_pres */
extern __pdata alt_t	ao_sample_height;	/* AGL of ao_sample_pres */
extern __pdata pres_t	ao_ground_pres;		/* startup pressure */
extern __pdata alt_t	ao_ground_height;	/* MSL of ao_ground_pres */
#endif

#if HAS_ACCEL
extern __pdata accel_t	ao_sample_accel;	/* most recent accel sensor reading */
extern __pdata accel_t	ao_ground_accel;	/* startup acceleration */
extern __pdata accel_t 	ao_accel_2g;		/* factory accel calibration */
extern __pdata int32_t	ao_accel_scale;		/* sensor to m/s² conversion */
#endif
#if HAS_GYRO
extern __pdata accel_t	ao_ground_accel_along;
extern __pdata accel_t	ao_ground_accel_across;
extern __pdata accel_t	ao_ground_accel_through;
extern __pdata int32_t	ao_ground_pitch;	/* * 512 */
extern __pdata int32_t	ao_ground_yaw;		/* * 512 */
extern __pdata int32_t	ao_ground_roll;		/* * 512 */
extern __pdata accel_t	ao_sample_accel_along;
extern __pdata accel_t	ao_sample_accel_across;
extern __pdata accel_t	ao_sample_accel_through;
extern __pdata gyro_t	ao_sample_roll;
extern __pdata gyro_t	ao_sample_pitch;
extern __pdata gyro_t	ao_sample_yaw;
extern __pdata angle_t	ao_sample_orient;
#endif

void ao_sample_init(void);

/* returns FALSE in preflight mode, TRUE in flight mode */
uint8_t ao_sample(void);

/*
 * ao_kalman.c
 */

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

extern __pdata int16_t			ao_height;	/* meters */
extern __pdata int16_t			ao_speed;	/* m/s * 16 */
extern __pdata int16_t			ao_accel;	/* m/s² * 16 */
extern __xdata int16_t			ao_max_height;	/* max of ao_height */
extern __xdata int16_t			ao_avg_height;	/* running average of height */

extern __pdata int16_t			ao_error_h;
extern __pdata int16_t			ao_error_h_sq_avg;

#if HAS_ACCEL
extern __pdata int16_t			ao_error_a;
#endif

void ao_kalman(void);

#endif /* _AO_SAMPLE_H_ */
