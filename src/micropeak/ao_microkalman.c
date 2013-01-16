/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
#include <ao.h>
#endif
#include <ao_micropeak.h>

#define FIX_BITS	16

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix8(x)	((x) >> 8)
#define from_fix(x)	((x) >> 16)
#define fix8_to_fix16(x)	((x) << 8)
#define fix16_to_fix8(x)	((x) >> 8)

#include <ao_kalman.h>

/* Basic time step (96ms) */
#define AO_MK_STEP	to_fix16(0.096)
/* step ** 2 / 2 */
#define AO_MK_STEP_2_2	to_fix16(0.004608)

uint32_t	ao_k_pa;		/* 24.8 fixed point */
int32_t		ao_k_pa_speed;		/* 16.16 fixed point */
int32_t		ao_k_pa_accel;		/* 16.16 fixed point */

uint32_t	ao_pa;			/* integer portion */
int16_t		ao_pa_speed;		/* integer portion */
int16_t		ao_pa_accel;		/* integer portion */

void
ao_microkalman_init(void)
{
	ao_pa = pa;
	ao_k_pa = pa << 8;
}	

void
ao_microkalman_predict(void)
{
	ao_k_pa       += fix16_to_fix8((int32_t) ao_pa_speed * AO_MK_STEP + (int32_t) ao_pa_accel * AO_MK_STEP_2_2);
	ao_k_pa_speed += (int32_t) ao_pa_accel * AO_MK_STEP;
}

void
ao_microkalman_correct(void)
{
	int16_t	e;	/* Height error in Pa */

	e = pa - from_fix8(ao_k_pa);

	ao_k_pa       += fix16_to_fix8((int32_t) e * AO_MK_BARO_K0_10);
	ao_k_pa_speed += (int32_t) e * AO_MK_BARO_K1_10;
	ao_k_pa_accel += (int32_t) e * AO_MK_BARO_K2_10;
	ao_pa = from_fix8(ao_k_pa);
	ao_pa_speed = from_fix(ao_k_pa_speed);
	ao_pa_accel = from_fix(ao_k_pa_accel);
}
