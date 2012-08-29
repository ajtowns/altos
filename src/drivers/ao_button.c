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
#include <ao_button.h>
#include <ao_exti.h>
#if AO_EVENT
#include <ao_event.h>
#define ao_button_queue(b,v)	ao_event_put_isr(AO_EVENT_BUTTON, b, v)
#else
#define ao_button_queue(b,v)
#endif

static uint8_t		ao_button[AO_BUTTON_COUNT];
static AO_TICK_TYPE	ao_button_time[AO_BUTTON_COUNT];

#define AO_DEBOUNCE	AO_MS_TO_TICKS(20)

#define port(q)	AO_BUTTON_ ## q ## _PORT
#define bit(q) AO_BUTTON_ ## q
#define pin(q) AO_BUTTON_ ## q ## _PIN

static void
ao_button_do(uint8_t b, uint8_t v)
{
	/* Debounce */
	if ((AO_TICK_SIGNED) (ao_tick_count - ao_button_time[b]) < AO_DEBOUNCE)
		return;

	/* pins are inverted */
	v = !v;
	if (ao_button[b] != v) {
		ao_button[b] = v;
		ao_button_time[b] = ao_tick_count;
		ao_button_queue(b, v);
		ao_wakeup(&ao_button[b]);
	}
}

#define ao_button_update(b)	ao_button_do(b, ao_gpio_get(port(b), bit(b), pin(b)))

static void
ao_button_isr(void)
{
#if AO_BUTTON_COUNT > 0
	ao_button_update(0);
#endif
#if AO_BUTTON_COUNT > 1
	ao_button_update(1);
#endif
#if AO_BUTTON_COUNT > 2
	ao_button_update(2);
#endif
#if AO_BUTTON_COUNT > 3
	ao_button_update(3);
#endif
#if AO_BUTTON_COUNT > 4
	ao_button_update(4);
#endif
}

#define init(b) do {							\
		ao_enable_port(port(b));				\
									\
		ao_exti_setup(port(b), bit(b),				\
			      AO_BUTTON_MODE|AO_EXTI_MODE_FALLING|AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_MED, \
			      ao_button_isr);			\
		ao_exti_enable(port(b), bit(b));			\
	} while (0)

void
ao_button_init(void)
{
#if AO_BUTTON_COUNT > 0
	init(0);
#endif
#if AO_BUTTON_COUNT > 1
	init(1);
#endif
}
