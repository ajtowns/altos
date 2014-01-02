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

#define GRAVITY 9.80665

#if HAS_ADC
#define AO_DATA_ADC	(1 << 0)
#else
#define AO_DATA_ADC	0
#endif

#if HAS_MS5607
#include <ao_ms5607.h>
#define AO_DATA_MS5607	(1 << 1)
#else
#define AO_DATA_MS5607	0
#endif

#if HAS_MPU6000
#include <ao_mpu6000.h>
#define AO_DATA_MPU6000	(1 << 2)
#else
#define AO_DATA_MPU6000	0
#endif

#if HAS_HMC5883
#include <ao_hmc5883.h>
#define AO_DATA_HMC5883	(1 << 3)
#else
#define AO_DATA_HMC5883	0
#endif

#if HAS_MMA655X
#include <ao_mma655x.h>
#define AO_DATA_MMA655X (1 << 4)
#else
#define AO_DATA_MMA655X 0
#endif

#ifdef AO_DATA_RING

#define AO_DATA_ALL	(AO_DATA_ADC|AO_DATA_MS5607|AO_DATA_MPU6000|AO_DATA_HMC5883|AO_DATA_MMA655X)

struct ao_data {
	uint16_t			tick;
#if HAS_ADC
	struct ao_adc			adc;
#endif
#if HAS_MS5607
	struct ao_ms5607_sample		ms5607_raw;
	struct ao_ms5607_value		ms5607_cooked;
#endif
#if HAS_MPU6000
	struct ao_mpu6000_sample	mpu6000;
#if !HAS_MMA655X
	int16_t				z_accel;
#endif
#endif
#if HAS_HMC5883
	struct ao_hmc5883_sample	hmc5883;
#endif
#if HAS_MMA655X
	uint16_t			mma655x;
#endif
};

#define ao_data_ring_next(n)	(((n) + 1) & (AO_DATA_RING - 1))
#define ao_data_ring_prev(n)	(((n) - 1) & (AO_DATA_RING - 1))

/* Get a copy of the last complete sample set */
void
ao_data_get(__xdata struct ao_data *packet);

extern volatile __xdata struct ao_data	ao_data_ring[AO_DATA_RING];
extern volatile __data uint8_t		ao_data_head;
extern volatile __data uint8_t		ao_data_present;
extern volatile __data uint8_t		ao_data_count;

/*
 * Mark a section of data as ready, check for data complete
 */
#define AO_DATA_PRESENT(bit)	(ao_data_present |= (bit))

/*
 * Wait until it is time to write a sensor sample; this is
 * signaled by the timer tick
 */
#define AO_DATA_WAIT() do {				\
		ao_sleep(DATA_TO_XDATA ((void *) &ao_data_count));	\
	} while (0)

#endif /* AO_DATA_RING */

#if !HAS_BARO && HAS_MS5607

/* Either an MS5607 or an MS5611 hooked to a SPI port
 */

#define HAS_BARO	1

typedef int32_t	pres_t;

#ifndef AO_ALT_TYPE
#define AO_ALT_TYPE	int32_t
#endif

typedef AO_ALT_TYPE	alt_t;

#define ao_data_pres_cook(packet)	ao_ms5607_convert(&packet->ms5607_raw, &packet->ms5607_cooked)

#define ao_data_pres(packet)	((packet)->ms5607_cooked.pres)
#define ao_data_temp(packet)	((packet)->ms5607_cooked.temp)

#define pres_to_altitude(p)	ao_pa_to_altitude(p)

#endif

#if !HAS_BARO && HAS_ADC

#define HAS_BARO	1

typedef int16_t pres_t;
typedef int16_t alt_t;

#define ao_data_pres(packet)	((packet)->adc.pres)
#define ao_data_temp(packet)	((packet)->adc.temp)
#define pres_to_altitude(p)	ao_pres_to_altitude(p)
#define ao_data_pres_cook(p)

#endif

#if !HAS_BARO
typedef int16_t alt_t;
#endif

/*
 * Need a few macros to pull data from the sensors:
 *
 * ao_data_accel_sample	- pull raw sensor and convert to normalized values
 * ao_data_accel	- pull normalized value (lives in the same memory)
 * ao_data_set_accel	- store normalized value back in the sensor location
 * ao_data_accel_invert	- flip rocket ends for positive acceleration
 */

#if HAS_ACCEL

/* This section is for an analog accelerometer hooked to one of the ADC pins. As
 * those are 5V parts, this also requires that the 5V supply be hooked to to anothe ADC
 * pin so that the both can be measured to correct for changes between the 3.3V and 5V rails
 */

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

#define ao_data_accel_cook(packet) \
	((uint16_t) ((((uint32_t) (packet)->adc.accel << 16) / ((packet)->adc.accel_ref << 1))) >> 1)

#else

#define ao_data_accel_cook(packet) ((packet)->adc.accel)

#endif /* HAS_ACCEL_REF */

#endif	/* HAS_ACCEL */

#if !HAS_ACCEL && HAS_MMA655X

#define HAS_ACCEL	1

typedef int16_t accel_t;

/* MMA655X is hooked up so that positive values represent negative acceleration */

#define AO_ACCEL_INVERT		4095

#define ao_data_accel(packet)			((packet)->mma655x)
#if AO_MMA655X_INVERT
#define ao_data_accel_cook(packet)		(AO_ACCEL_INVERT - (packet)->mma655x)
#else
#define ao_data_accel_cook(packet)		((packet)->mma655x)
#endif
#define ao_data_set_accel(packet, accel)	((packet)->mma655x = (accel))
#define ao_data_accel_invert(accel)		(AO_ACCEL_INVERT - (accel))

#endif

#if !HAS_ACCEL && HAS_MPU6000

#define HAS_ACCEL	1

#define AO_ACCEL_INVERT		0

typedef int16_t accel_t;

/* MPU6000 is hooked up so that positive y is positive acceleration */
#define ao_data_accel(packet)			((packet)->z_accel)
#define ao_data_accel_cook(packet)		(-(packet)->mpu6000.accel_y)
#define ao_data_set_accel(packet, accel)	((packet)->z_accel = (accel))
#define ao_data_accel_invert(a)			(-(a))

#endif

#if !HAS_GYRO && HAS_MPU6000

#define HAS_GYRO	1

typedef int16_t	gyro_t;		/* in raw sample units */
typedef int16_t angle_t;	/* in degrees */

/* Y axis is aligned with the direction of motion (along) */
/* X axis is aligned in the other board axis (across) */
/* Z axis is aligned perpendicular to the board (through) */

#define ao_data_along(packet)	((packet)->mpu6000.accel_y)
#define ao_data_across(packet)	((packet)->mpu6000.accel_x)
#define ao_data_through(packet)	((packet)->mpu6000.accel_z)

#define ao_data_roll(packet)	((packet)->mpu6000.gyro_y)
#define ao_data_pitch(packet)	((packet)->mpu6000.gyro_x)
#define ao_data_yaw(packet)	((packet)->mpu6000.gyro_z)

#endif

#if !HAS_MAG && HAS_HMC5883

#define HAS_MAG		1

typedef int16_t ao_mag_t;		/* in raw sample units */

#define ao_data_mag_along(packet)	((packet)->hmc5883.x)
#define ao_data_mag_across(packet)	((packet)->hmc5883.y)
#define ao_data_mag_through(packet)	((packet)->hmc5883.z)

#endif

#endif /* _AO_DATA_H_ */
