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

#include <ao.h>
#include <ao_debounce.h>
#include <ao_fast_timer.h>

static uint8_t			ao_debounce_initialized;
static uint8_t			ao_debounce_running;
static struct ao_debounce	*ao_debounce;

static uint8_t	values[64];
static uint8_t	n;

#define d_step(n)	(((n) + 1) & 63)

static void
_ao_debounce_set(struct ao_debounce *debounce, uint8_t value)
{
	if (value != debounce->value) {
		values[n] = value;
		n = (n + 1) & 63;
		debounce->value = value;
		debounce->_set(debounce, value);
	}
	_ao_debounce_stop(debounce);
}

void
ao_debounce_dump(void)
{
	uint8_t	s;

	for (s = 0; s < n; s++) {
		printf ("%d: %d\n",
			s, values[s]);
	}
	n = 0;
}

/*
 * Get the current value, set the result when we've
 * reached the debounce count limit
 */
static void
_ao_debounce_check(struct ao_debounce *debounce)
{
	uint8_t	next = debounce->_get(debounce);

	if (next == debounce->current) {
		if (debounce->count < debounce->hold) {
			if (++debounce->count == debounce->hold)
				_ao_debounce_set(debounce, debounce->current);
		}
	} else {
		debounce->count = 0;
		debounce->current = next;
	}
}

static void
_ao_debounce_isr(void)
{
	struct ao_debounce *debounce, *next;

	for (debounce = ao_debounce; debounce; debounce = next) {
		next = debounce->next;
		_ao_debounce_check(debounce);
	}
}

static void
ao_debounce_on(void)
{
	ao_fast_timer_on(_ao_debounce_isr);
}

static void
ao_debounce_off(void)
{
	ao_fast_timer_off(_ao_debounce_isr);
}

/*
 * Start monitoring one pin
 */
void
_ao_debounce_start(struct ao_debounce *debounce)
{
	uint32_t	m;

	m = ao_arch_irqsave();
	if (!debounce->running) {
		debounce->running = 1;

		/* Reset the counter */
		debounce->count = 0;

		/* Link into list */
		debounce->next = ao_debounce;
		ao_debounce = debounce;

		/* Make sure the timer is running */
		if (!ao_debounce_running++)
			ao_debounce_on();

		/* And go check the current value */
		_ao_debounce_check(debounce);
	}
	ao_arch_irqrestore(m);
}

/*
 * Stop monitoring one pin
 */
void
_ao_debounce_stop(struct ao_debounce *debounce)
{
	struct ao_debounce **prev;
	uint32_t m;

	m = ao_arch_irqsave();
	if (debounce->running) {
		debounce->running = 0;

		/* Unlink */
		for (prev = &ao_debounce; (*prev); prev = &((*prev)->next)) {
			if (*prev == debounce) {
				*prev = debounce->next;
				break;
			}
		}
		debounce->next = NULL;

		/* Turn off the timer if possible */
		if (!--ao_debounce_running)
			ao_debounce_off();
	}
	ao_arch_irqrestore(m);
}

void
ao_debounce_init(void)
{
	if (ao_debounce_initialized)
		return;
	ao_debounce_initialized = 1;
	ao_fast_timer_init();
}
