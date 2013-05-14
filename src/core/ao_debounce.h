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

#ifndef _AO_DEBOUNCE_H_
#define _AO_DEBOUNCE_H_

struct ao_debounce {
	struct ao_debounce	*next;

	/* time that pin value must be stable before accepting */
	int8_t			hold;

	/* last value reported to app; don't report it twice */
	uint8_t			value;

	/* current count of intervals pin value has been stable */
	int8_t			count;

	/* This pin is running */
	uint8_t			running;

	/* Get the current pin value */
	uint8_t			(*_get)(struct ao_debounce *debounce);

	/* The stable value has changed */
	void 			(*_set)(struct ao_debounce *debounce, uint8_t value);
};

void
_ao_debounce_start(struct ao_debounce *debounce);

void
_ao_debounce_stop(struct ao_debounce *debounce);

void
ao_debounce_init(void);

#endif /* _AO_DEBOUNCE_H_ */
