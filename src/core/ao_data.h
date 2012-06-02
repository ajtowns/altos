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

#ifndef _AO_DATA_H_
#define _AO_DATA_H_

#if HAS_MS5607
#include <ao_ms5607.h>
#endif

#if HAS_MPU6000
#include <ao_mpu6000.h>
#endif

struct ao_data {
	uint16_t			tick;
#if HAS_ADC
	struct ao_adc			adc;
#endif
#if HAS_MS5607
	struct ao_ms5607_sample		ms5607;
#endif
#if HAS_MPU6000
	struct ao_mpu6000_sample	mpu6000;
#endif
};

#define ao_data_ring_next(n)	(((n) + 1) & (AO_DATA_RING - 1))
#define ao_data_ring_prev(n)	(((n) - 1) & (AO_DATA_RING - 1))

extern volatile __xdata struct ao_data	ao_data_ring[AO_DATA_RING];
extern volatile __data uint8_t		ao_data_head;

#if HAS_MS5607

typedef int32_t	pres_t;
typedef int32_t alt_t;

static inline pres_t ao_data_pres(struct ao_data *packet)
{
	struct ao_ms5607_value	value;

	ao_ms5607_convert(&packet->ms5607, &value);
	return value.pres;
}

#define pres_to_altitude(p)	ao_pa_to_altitude(p)

#else

typedef int16_t pres_t;
typedef int16_t alt_t;

#define ao_data_pres(packet)	((packet)->adc.pres)
#define pres_to_altitude(p)	ao_pres_to_altitude(p)

#endif

/*
 * Need a few macros to pull data from the sensors:
 *
 * ao_data_accel_sample	- pull raw sensor and convert to normalized values
 * ao_data_accel	- pull normalized value (lives in the same memory)
 * ao_data_set_accel	- store normalized value back in the sensor location
 * ao_data_accel_invert	- flip rocket ends for positive acceleration
 */

#if HAS_MPU6000 && !HAS_HIGHG_ACCEL

typedef int16_t accel_t;

/* MPU6000 is hooked up so that positive y is positive acceleration */
#define ao_data_accel(packet)			((packet)->mpu6000.accel_y)
#define ao_data_accel_sample(packet)		(-ao_data_accel(packet))
#define ao_data_set_accel(packet, accel)	((packet)->mpu6000.accel_y = (accel))
#define ao_data_accel_invert(a)			(-(a))

#else

typedef int16_t accel_t;
#define ao_data_accel(packet)			((packet)->adc.accel)
#define ao_data_set_accel(packet, a)		((packet)->adc.accel = (a))
#define ao_data_accel_invert(a)			(0x7fff -(a))

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
#if HAS_ACCEL_REF
#define ao_data_accel_sample(packet) \
	((uint16_t) ((((uint32_t) (packet)->adc.accel << 16) / ((packet)->adc.accel_ref << 1))) >> 1)
#else
#define ao_data_accel_sample(packet) ((packet)->adc.accel)
#endif /* HAS_ACCEL_REF */

#endif	/* else some other pressure sensor */

#endif /* _AO_DATA_H_ */
