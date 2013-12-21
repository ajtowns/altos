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

#include <ao.h>
#include <ao_event.h>

#define AO_EVENT_QUEUE	64

#define ao_event_queue_next(n)	(((n) + 1) & (AO_EVENT_QUEUE - 1))
#define ao_event_queue_prev(n)	(((n) - 1) & (AO_EVENT_QUEUE - 1))
#define ao_event_queue_empty()	(ao_event_queue_insert == ao_event_queue_remove)
#define ao_event_queue_full()	(ao_event_queue_next(ao_event_queue_insert) == ao_event_queue_remove)

struct ao_event ao_event_queue[AO_EVENT_QUEUE];
uint8_t		ao_event_queue_insert;
uint8_t		ao_event_queue_remove;


void
ao_event_get(struct ao_event *ev)
{
	ao_arch_critical(
		while (ao_event_queue_empty())
			ao_sleep(&ao_event_queue);
		*ev = ao_event_queue[ao_event_queue_remove];
		ao_event_queue_remove = ao_event_queue_next(ao_event_queue_remove);
		);
}

/* called with interrupts disabled */
void
ao_event_put_isr(uint8_t type, uint8_t unit, int32_t value)
{
	if (!ao_event_queue_full()) {
		ao_event_queue[ao_event_queue_insert] = (struct ao_event) {
			.type = type,
			.unit = unit,
			.tick = ao_tick_count,
			.value = value
		};
		ao_event_queue_insert = ao_event_queue_next(ao_event_queue_insert);
		ao_wakeup(&ao_event_queue);
	}
}

void
ao_event_put(uint8_t type, uint8_t unit, int32_t value)
{
	ao_arch_critical(ao_event_put_isr(type, unit, value););
}
