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
#include <ao_debounce.h>
#if AO_EVENT
#include <ao_event.h>
#define ao_button_queue(b,v)	ao_event_put_isr(AO_EVENT_BUTTON, b, v)
#else
#define ao_button_queue(b,v)
#endif

#define AO_BUTTON_DEBOUNCE_HOLD	10

static struct ao_debounce	ao_button_debounce[AO_BUTTON_COUNT];

#define port(q)	AO_BUTTON_ ## q ## _PORT
#define bit(q) AO_BUTTON_ ## q
#define pin(q) AO_BUTTON_ ## q ## _PIN

/* pins are inverted */
#define ao_button_value(b)	!ao_gpio_get(port(b), bit(b), pin(b))

static uint8_t
_ao_button_get(struct ao_debounce *debounce)
{
	uint8_t	b = debounce - ao_button_debounce;

	switch (b) {
#if AO_BUTTON_COUNT > 0
	case 0: return ao_button_value(0);
#endif
#if AO_BUTTON_COUNT > 1
	case 1: return ao_button_value(1);
#endif
#if AO_BUTTON_COUNT > 2
	case 2: return ao_button_value(2);
#endif
#if AO_BUTTON_COUNT > 3
	case 3: return ao_button_value(3);
#endif
#if AO_BUTTON_COUNT > 4
	case 4: return ao_button_value(4);
#endif
	}
	return 0;
}

static void
_ao_button_set(struct ao_debounce *debounce, uint8_t value)
{
	uint8_t b = debounce - ao_button_debounce;

	ao_button_queue(b, value);
}


#define ao_button_update(b)	ao_button_do(b, ao_gpio_get(port(b), bit(b), pin(b)))

static void
ao_button_debounce_init(struct ao_debounce *debounce) {
	ao_debounce_config(debounce,
			   _ao_button_get,
			   _ao_button_set,
			   AO_BUTTON_DEBOUNCE_HOLD);
}

static void
ao_button_isr(void)
{
	uint8_t	b;

	for (b = 0; b < AO_BUTTON_COUNT; b++)
		_ao_debounce_start(&ao_button_debounce[b]);
}

#define init(b) do {							\
		ao_button_debounce_init(&ao_button_debounce[b]);	\
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
#if AO_BUTTON_COUNT > 2
	init(2);
#endif
#if AO_BUTTON_COUNT > 3
	init(3);
#endif
#if AO_BUTTON_COUNT > 4
	init(4);
#endif
	ao_debounce_init();
}
