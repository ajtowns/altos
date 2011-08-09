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

/*
 * Current sensor values
 */

__pdata uint16_t	ao_sample_tick;		/* time of last data */
__pdata int16_t		ao_sample_pres;
__pdata int16_t		ao_sample_alt;
__pdata int16_t		ao_sample_height;
#if HAS_ACCEL
__pdata int16_t		ao_sample_accel;
#endif

__data uint8_t		ao_sample_adc;

/*
 * Sensor calibration values
 */

__pdata int16_t		ao_ground_pres;		/* startup pressure */
__pdata int16_t		ao_ground_height;	/* MSL of ao_ground_pres */

#if HAS_ACCEL
__pdata int16_t		ao_ground_accel;	/* startup acceleration */
__pdata int16_t		ao_accel_2g;		/* factory accel calibration */
__pdata int32_t		ao_accel_scale;		/* sensor to m/s² conversion */
#endif

static __pdata uint8_t	ao_preflight;		/* in preflight mode */

static __pdata uint16_t	nsamples;
__pdata int32_t ao_sample_pres_sum;
#if HAS_ACCEL
__pdata int32_t ao_sample_accel_sum;
#endif

static void
ao_sample_preflight(void)
{
	/* startup state:
	 *
	 * Collect 512 samples of acceleration and pressure
	 * data and average them to find the resting values
	 */
	if (nsamples < 512) {
#if HAS_ACCEL
		ao_sample_accel_sum += ao_sample_accel;
#endif
		ao_sample_pres_sum += ao_sample_pres;
		++nsamples;
	} else {
		ao_config_get();
#if HAS_ACCEL
		ao_ground_accel = ao_sample_accel_sum >> 9;
		ao_accel_2g = ao_config.accel_minus_g - ao_config.accel_plus_g;
		ao_accel_scale = to_fix32(GRAVITY * 2 * 16) / ao_accel_2g;
#endif
		ao_ground_pres = ao_sample_pres_sum >> 9;
		ao_ground_height = ao_pres_to_altitude(ao_ground_pres);
		ao_preflight = FALSE;
	}
}

uint8_t
ao_sample(void)
{
	ao_wakeup(DATA_TO_XDATA(&ao_sample_adc));
	ao_sleep(DATA_TO_XDATA(&ao_adc_head));
	while (ao_sample_adc != ao_adc_head) {
		__xdata struct ao_adc *ao_adc;

		/* Capture a sample */
		ao_adc = &ao_adc_ring[ao_sample_adc];
		ao_sample_tick = ao_adc->tick;
		ao_sample_pres = ao_adc->pres;
		ao_sample_alt = ao_pres_to_altitude(ao_sample_pres);
		ao_sample_height = ao_sample_alt - ao_ground_height;
#if HAS_ACCEL
		ao_sample_accel = ao_adc->accel;
#if HAS_ACCEL_REF
		/*
		 * Ok, the math here is a bit tricky.
		 *
		 * ao_sample_accel:  ADC output for acceleration
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
		 *     ao_sample_accel   accel
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
		 *                      ao_sample_accel       32767
		 *                    = ------------ *  ------------
		 *                         32767        ao_accel_ref
		 *
		 * Multiply through by 32767:
		 *
		 *                      ao_sample_accel * 32767
		 *	ao_cook_accel = --------------------
		 *                          ao_accel_ref
		 *
		 * Now, the tricky part. Getting this to compile efficiently
		 * and keeping all of the values in-range.
		 *
		 * First off, we need to use a shift of 16 instead of * 32767 as SDCC
		 * does the obvious optimizations for byte-granularity shifts:
		 *
		 *	ao_cook_accel = (ao_sample_accel << 16) / ao_accel_ref
		 *
		 * Next, lets check our input ranges:
		 *
		 * 	0 <= ao_sample_accel <= 0x7fff		(singled ended ADC conversion)
		 *	0x7000 <= ao_accel_ref <= 0x7fff	(the 5V ref value is close to 0x7fff)
		 *
		 * Plugging in our input ranges, we get an output range of 0 - 0x12490,
		 * which is 17 bits. That won't work. If we take the accel ref and shift
		 * by a bit, we'll change its range:
		 *
		 *	0xe000 <= ao_accel_ref<<1 <= 0xfffe
		 *
		 *	ao_cook_accel = (ao_sample_accel << 16) / (ao_accel_ref << 1)
		 *
		 * Now the output range is 0 - 0x9248, which nicely fits in 16 bits. It
		 * is, however, one bit too large for our signed computations. So, we
		 * take the result and shift that by a bit:
		 *
		 *	ao_cook_accel = ((ao_sample_accel << 16) / (ao_accel_ref << 1)) >> 1
		 *
		 * This finally creates an output range of 0 - 0x4924. As the ADC only
		 * provides 11 bits of data, we haven't actually lost any precision,
		 * just dropped a bit of noise off the low end.
		 */
		ao_sample_accel = (uint16_t) ((((uint32_t) ao_sample_accel << 16) / (ao_accel_ref[ao_sample_adc] << 1))) >> 1;
		if (ao_config.pad_orientation != AO_PAD_ORIENTATION_ANTENNA_UP)
			ao_sample_accel = 0x7fff - ao_sample_accel;
		ao_adc->accel = ao_sample_accel;
#endif
#endif

		if (ao_preflight)
			ao_sample_preflight();
		else
			ao_kalman();
		ao_sample_adc = ao_adc_ring_next(ao_sample_adc);
	}
	return !ao_preflight;
}

void
ao_sample_init(void)
{
	nsamples = 0;
	ao_sample_pres_sum = 0;
	ao_sample_pres = 0;
#if HAS_ACCEL
	ao_sample_accel_sum = 0;
	ao_sample_accel = 0;
#endif
	ao_sample_adc = ao_adc_head;
	ao_preflight = TRUE;
}
