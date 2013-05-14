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
#include <ao_quadrature.h>
#include <ao_exti.h>
#include <ao_debounce.h>
#include <ao_event.h>

#define AO_QUADRATURE_DEBOUNCE_HOLD	3

static __xdata struct ao_debounce ao_quadrature_debounce[AO_QUADRATURE_COUNT];

#define debounce_id(d)	((d) - ao_quadrature_debounce)

__xdata int32_t ao_quadrature_count[AO_QUADRATURE_COUNT];

static uint8_t	ao_quadrature_state[AO_QUADRATURE_COUNT];

#define BIT(a,b)	((a) | ((b) << 1))
#define STATE(old_a, old_b, new_a, new_b)	(((BIT(old_a, old_b) << 2) | BIT(new_a, new_b)))

#define port(q)	AO_QUADRATURE_ ## q ## _PORT
#define bita(q) AO_QUADRATURE_ ## q ## _A
#define bitb(q) AO_QUADRATURE_ ## q ## _B
#define pina(q) AO_QUADRATURE_ ## q ## _A ## _PIN
#define pinb(q) AO_QUADRATURE_ ## q ## _B ## _PIN

#define q_case(q) case q: v = (!ao_gpio_get(port(q), bita(q), pina(q))) | ((!ao_gpio_get(port(q), bitb(q), pinb(q))) << 1); break

uint8_t quad_raw[64];
uint8_t quad_r;

static uint8_t
_ao_quadrature_get(struct ao_debounce *debounce) {
	uint8_t	q = debounce_id(debounce);
	uint8_t v = 0;

	switch (q) {
#if AO_QUADRATURE_COUNT > 0
		q_case(0);
#endif
#if AO_QUADRATURE_COUNT > 1
		q_case(1);
#endif
	}
	if (q == 0) {
		quad_raw[quad_r] = v;
		quad_r = (quad_r + 1) & 63;
	}
	return v;
}

static void
_ao_quadrature_queue(uint8_t q, int8_t step)
{
	ao_quadrature_count[q] += step;
#if AO_EVENT
	ao_event_put_isr(AO_EVENT_QUADRATURE, q, step);
#endif
	ao_wakeup(&ao_quadrature_count[q]);
}

uint8_t quad_history[64];
uint8_t quad_h;

static void
_ao_quadrature_set(struct ao_debounce *debounce, uint8_t value) {
	uint8_t q = debounce_id(debounce);
	
	ao_quadrature_state[q] = ((ao_quadrature_state[q] & 3) << 2);
	ao_quadrature_state[q] |= value;

	if (q == 0) {
		quad_history[quad_h] = ao_quadrature_state[0];
		quad_h = (quad_h + 1) & 63;
	}

	switch (ao_quadrature_state[q]) {
	case STATE(0, 1, 0, 0):
		_ao_quadrature_queue(q, 1);
		break;
	case STATE(1, 0, 0, 0):
		_ao_quadrature_queue(q, -1);
		break;
	}
}

static void
ao_quadrature_isr(void)
{
	uint8_t	q;
	for (q = 0; q < AO_QUADRATURE_COUNT; q++)
		_ao_debounce_start(&ao_quadrature_debounce[q]);
}

int32_t
ao_quadrature_poll(uint8_t q)
{
	int32_t	ret;
	ao_arch_critical(ret = ao_quadrature_count[q];);
	return ret;
}

int32_t
ao_quadrature_wait(uint8_t q)
{
	ao_sleep(&ao_quadrature_count[q]);
	return ao_quadrature_poll(q);
}

static void
ao_quadrature_test(void)
{
	uint8_t	q;

	ao_cmd_decimal();
	q = ao_cmd_lex_i;
	for (;;) {
		int32_t	c;
		flush();
		c = ao_quadrature_wait(q);
		printf ("new count %6d\n", c);
		if (c == 100)
			break;
	}
}

static const struct ao_cmds ao_quadrature_cmds[] = {
	{ ao_quadrature_test,	"q <unit>\0Test quadrature" },
	{ 0, NULL }
};

static void
ao_quadrature_debounce_init(struct ao_debounce *debounce) {
	debounce->hold = AO_QUADRATURE_DEBOUNCE_HOLD;
	debounce->_get = _ao_quadrature_get;
	debounce->_set = _ao_quadrature_set;
}

#define init(q) do {							\
		ao_enable_port(port(q));				\
		ao_quadrature_debounce_init(&ao_quadrature_debounce[q]); \
		ao_exti_setup(port(q), bita(q),				\
			      AO_QUADRATURE_MODE|AO_EXTI_MODE_FALLING|AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_MED, \
			      ao_quadrature_isr);			\
		ao_exti_enable(port(q), bita(q));			\
									\
		ao_exti_setup(port(q), bitb(q),				\
			      AO_QUADRATURE_MODE|AO_EXTI_MODE_FALLING|AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_MED, \
			      ao_quadrature_isr);			\
		ao_exti_enable(port(q), bitb(q));			\
	} while (0)

void
ao_quadrature_init(void)
{
	ao_debounce_init();
#if AO_QUADRATURE_COUNT > 0
	init(0);
#endif
#if AO_QUADRATURE_COUNT > 1
	init(1);
#endif
	ao_cmd_register(&ao_quadrature_cmds[0]);
}
