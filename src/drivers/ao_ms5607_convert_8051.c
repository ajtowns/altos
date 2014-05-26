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

#include <ao_ms5607.h>
#include <ao_int64.h>

#if HAS_MS5611
#define SHIFT_OFF	16
#define SHIFT_TCO	7
#define SHIFT_SENS	15
#define SHFIT_TCS	8
#else
#define SHIFT_OFF	17
#define SHIFT_TCO	6
#define SHIFT_SENS	16
#define SHIFT_TCS	7
#endif

void
ao_ms5607_convert(__xdata struct ao_ms5607_sample *sample,
		  __xdata struct ao_ms5607_value *value)
{
	__LOCAL int32_t	dT;
	__LOCAL int32_t TEMP;
	__LOCAL ao_int64_t OFF;
	__LOCAL ao_int64_t SENS;
	__LOCAL ao_int64_t a;

	dT = sample->temp - ((int32_t) ao_ms5607_prom.tref << 8);
	
	/* TEMP = 2000 + (((int64_t) dT * ao_ms5607_prom.tempsens) >> 23); */
	ao_mul64_32_32(&a, dT, ao_ms5607_prom.tempsens);
	ao_rshift64(&a, &a, 23);
	TEMP = 2000 + a.low;
	/* */

	/* OFF = ((int64_t) ao_ms5607_prom.off << SHIFT_OFF) + (((int64_t) ao_ms5607_prom.tco * dT) >> SHIFT_TCO);*/
#if SHIFT_OFF > 16
	OFF.high = ao_ms5607_prom.off >> (32 - SHIFT_OFF);
#else
	OFF.high = 0;
#endif
	OFF.low = (uint32_t) ao_ms5607_prom.off << SHIFT_OFF;
	ao_mul64_32_32(&a, ao_ms5607_prom.tco, dT);
	ao_rshift64(&a, &a, SHIFT_TCO);
	ao_plus64(&OFF, &OFF, &a);
	/**/

	/* SENS = ((int64_t) ao_ms5607_prom.sens << SHIFT_SENS) + (((int64_t) ao_ms5607_prom.tcs * dT) >> SHIFT_TCS); */
	SENS.high = 0;
	SENS.low = (uint32_t) ao_ms5607_prom.sens << SHIFT_SENS;
	ao_mul64_32_32(&a, ao_ms5607_prom.tcs, dT);
	ao_rshift64(&a, &a, SHIFT_TCS);
	ao_plus64(&SENS, &SENS, &a);
	/**/

	if (TEMP < 2000) {
		__LOCAL int32_t	T2;
		__LOCAL int32_t TEMPM;
		__LOCAL ao_int64_t OFF2;
		__LOCAL ao_int64_t SENS2;

		/* T2 = ((int64_t) dT * (int64_t) dT) >> 31; */
		ao_mul64_32_32(&a, dT, dT);
		T2 = (a.low >> 31) | (a.high << 1);
		/**/

		TEMPM = TEMP - 2000;

		/* OFF2 = (61 * (int64_t) TEMPM * (int64_t) TEMPM) >> 4; */
		ao_mul64_32_32(&OFF2, TEMPM, TEMPM);
		ao_mul64_64_16(&OFF2, &OFF2, 61);
		ao_rshift64(&OFF2, &OFF2, 4);
		/**/
		
		/* SENS2 = 2 * (int64_t) TEMPM * (int64_t) TEMPM; */
		ao_mul64_32_32(&SENS2, TEMPM, TEMPM);
		ao_lshift64(&SENS2, &SENS2, 1);
		/**/

		if (TEMP < -1500) {
			int32_t TEMPP;
			int32_t TEMPP2;

			TEMPP = TEMP + 1500;
			TEMPP2 = TEMPP * TEMPP;

			/* OFF2 = OFF2 + 15 * TEMPP2; */
			ao_mul64_32_32(&a, 15, TEMPP2);
			ao_plus64(&OFF2, &OFF2, &a);
			/**/

			/* SENS2 = SENS2 + 8 * TEMPP2; */
			a.high = 0;
			a.low = TEMPP2;
			ao_lshift64(&a, &a, 3);
			ao_plus64(&SENS2, &SENS2, &a);
			/**/
		}
		TEMP -= T2;

		/* OFF -= OFF2; */
		ao_minus64(&OFF, &OFF, &OFF2);
		/**/

		/* SENS -= SENS2; */
		ao_minus64(&SENS, &SENS, &SENS2);
		/**/
	}

	/* value->pres = ((((int64_t) sample->pres * SENS) >> 21) - OFF) >> 15; */
	a.high = 0;
	a.low = sample->pres;
	ao_mul64(&a, &a, &SENS);
	ao_rshift64(&a, &a, 21);
	ao_minus64(&a, &a, &OFF);
	ao_rshift64(&a, &a, 15);
	value->pres = a.low;
	/**/
	
	value->temp = TEMP;
}
