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

#ifndef _AO_COMPANION_H_
#define _AO_COMPANION_H_

/* ao_companion.c */

#define AO_COMPANION_SETUP		1
#define AO_COMPANION_FETCH		2
#define AO_COMPANION_NOTIFY		3

struct ao_companion_command {
	uint8_t		command;
	uint8_t		flight_state;
	uint16_t	tick;
	uint16_t	serial;
	uint16_t	flight;
	int16_t		accel;
	int16_t		speed;
	int16_t		height;
	int16_t		motor_number;
};

struct ao_companion_setup {
	uint16_t	board_id;
	uint16_t	board_id_inverse;
	uint8_t		update_period;
	uint8_t		channels;
};

extern __pdata uint8_t				ao_companion_running;
extern __xdata uint8_t				ao_companion_mutex;
extern __xdata struct ao_companion_command	ao_companion_command;
extern __xdata struct ao_companion_setup	ao_companion_setup;
extern __xdata uint16_t				ao_companion_data[AO_COMPANION_MAX_CHANNELS];

void
ao_companion_init(void);

#endif /* _AO_COMPANION_H_ */
