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

#ifndef _AO_EVENT_H_
#define _AO_EVENT_H_

#define AO_EVENT_NONE		0
#define AO_EVENT_QUADRATURE	1
#define AO_EVENT_BUTTON		2

struct ao_event {
	uint8_t		type;
	uint8_t		unit;
	uint16_t	tick;
	int32_t		value;
};

void
ao_event_get(struct ao_event *ev);

void
ao_event_put_isr(uint8_t type, uint8_t unit, int32_t value);

void
ao_event_put(uint8_t type, uint8_t unit, int32_t value);

#endif /* _AO_EVENT_H_ */
