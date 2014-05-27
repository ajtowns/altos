/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_FAKE_FLIGHT_H_
#define _AO_FAKE_FLIGHT_H_
#if HAS_MS5607 || HAS_MS5611
#include <ao_ms5607.h>
#endif

extern uint8_t	ao_fake_flight_active;

#define AO_FAKE_CALIB_MAJOR	1
#define AO_FAKE_CALIB_MINOR	0

struct ao_fake_calib {
	uint16_t		major;
	uint16_t		minor;
#if HAS_ACCEL
	int16_t			accel_plus_g;
	int16_t			accel_minus_g;
#endif
#if HAS_GYRO
	int16_t			accel_zero_along;
	int16_t			accel_zero_across;
	int16_t			accel_zero_through;
	uint16_t		pad30;
#endif
#if HAS_MS5607 || HAS_MS5611
	struct ao_ms5607_prom	ms5607_prom;
#endif
};

void
ao_fake_flight_poll(void);

void
ao_fake_flight_init(void);

#endif /* _AO_FAKE_FLIGHT_H_ */
