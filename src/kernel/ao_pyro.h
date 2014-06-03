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

#ifndef _AO_PYRO_H_
#define _AO_PYRO_H_

enum ao_pyro_flag {
	ao_pyro_none			= 0x00000000,

	ao_pyro_accel_less		= 0x00000001,
	ao_pyro_accel_greater		= 0x00000002,

	ao_pyro_speed_less		= 0x00000004,
	ao_pyro_speed_greater		= 0x00000008,

	ao_pyro_height_less		= 0x00000010,
	ao_pyro_height_greater		= 0x00000020,

	ao_pyro_orient_less		= 0x00000040,
	ao_pyro_orient_greater		= 0x00000080,

	ao_pyro_time_less		= 0x00000100,
	ao_pyro_time_greater		= 0x00000200,

	ao_pyro_ascending		= 0x00000400,
	ao_pyro_descending		= 0x00000800,

	ao_pyro_after_motor		= 0x00001000,

	ao_pyro_delay			= 0x00002000,

	ao_pyro_state_less		= 0x00004000,
	ao_pyro_state_greater_or_equal  = 0x00008000,
}
#ifdef __GNUC__
	__attribute__ ((packed))
#endif
	;

struct ao_pyro {
	enum ao_pyro_flag	flags;
	int16_t			accel_less, accel_greater;
	int16_t			speed_less, speed_greater;
	int16_t			height_less, height_greater;
	int16_t			orient_less, orient_greater;
	int16_t			time_less, time_greater;
	int16_t			delay;
	uint8_t			state_less, state_greater_or_equal;
	int16_t			motor;
	uint16_t		delay_done;
	uint8_t			fired;
};

#define AO_PYRO_8_BIT_VALUE	(ao_pyro_state_less|ao_pyro_state_greater_or_equal)

extern uint8_t	ao_pyro_wakeup;

extern uint16_t	ao_pyro_fired;

void
ao_pyro_set(void);

void
ao_pyro_show(void);

void
ao_pyro_init(void);

void
ao_pyro_manual(uint8_t p);

void
ao_pyro_print_status(void);

#endif
