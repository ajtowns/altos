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
	uint8_t			hold;

	/* last value reported to app; don't report it twice */
	uint8_t			value;

	/* current value received from pins */
	uint8_t			current;

	/* current count of intervals pin value has been stable */
	uint8_t			count;

	/* This pin is running */
	uint8_t			running;

	/* Get the current pin value */
	uint8_t			(*_get)(struct ao_debounce *debounce);

	/* The stable value has changed */
	void 			(*_set)(struct ao_debounce *debounce, uint8_t value);
};

static inline void
ao_debounce_config(struct ao_debounce *debounce,
		   uint8_t (*_get)(struct ao_debounce *debounce),
		   void (*_set)(struct ao_debounce *debounce, uint8_t value),
		   uint8_t hold)
{
	debounce->next = 0;
	debounce->hold = hold;
	debounce->value = 0xff;
	debounce->current = 0xff;
	debounce->count = 0;
	debounce->running = 0;
	debounce->_get = _get;
	debounce->_set = _set;
}

void
_ao_debounce_start(struct ao_debounce *debounce);

void
_ao_debounce_stop(struct ao_debounce *debounce);

void
ao_debounce_init(void);

void
ao_debounce_dump(void);

#endif /* _AO_DEBOUNCE_H_ */
