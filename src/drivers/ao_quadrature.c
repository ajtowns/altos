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
#if AO_EVENT
#include <ao_event.h>
#define ao_quadrature_queue(q)	ao_event_put_isr(AO_EVENT_QUADRATURE, q, ao_quadrature_count[q])
#else
#define ao_quadrature_queue(q)
#endif

__xdata int32_t ao_quadrature_count[AO_QUADRATURE_COUNT];

static uint8_t	ao_quadrature_state[AO_QUADRATURE_COUNT];

#define BIT(a,b)	((a) | ((b) << 1))
#define STATE(old_a, old_b, new_a, new_b)	(((BIT(old_a, old_b) << 2) | BIT(new_a, new_b)))

#define port(q)	AO_QUADRATURE_ ## q ## _PORT
#define bita(q) AO_QUADRATURE_ ## q ## _A
#define bitb(q) AO_QUADRATURE_ ## q ## _B

#define ao_quadrature_update(q) do {					\
		ao_quadrature_state[q] = ((ao_quadrature_state[q] & 3) << 2); \
		ao_quadrature_state[q] |= ao_gpio_get(port(q), bita(q), 0); \
		ao_quadrature_state[q] |= ao_gpio_get(port(q), bitb(q), 0) << 1; \
	} while (0)
	

static void
ao_quadrature_isr(void)
{
	uint8_t	q;
#if AO_QUADRATURE_COUNT > 0
	ao_quadrature_update(0);
#endif
#if AO_QUADRATURE_COUNT > 1
	ao_quadrature_update(1);
#endif

	for (q = 0; q < AO_QUADRATURE_COUNT; q++) {
		switch (ao_quadrature_state[q]) {
		case STATE(0, 1, 0, 0):
			ao_quadrature_count[q]++;
			break;
		case STATE(1, 0, 0, 0):
			ao_quadrature_count[q]--;
			break;
		default:
			continue;
		}
		ao_quadrature_queue(q);
		ao_wakeup(&ao_quadrature_count[q]);
	}
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

#define init(q) do {							\
		ao_enable_port(port(q));				\
									\
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
#if AO_QUADRATURE_COUNT > 0
	init(0);
#endif
#if AO_QUADRATURE_COUNT > 1
	init(1);
#endif
	ao_cmd_register(&ao_quadrature_cmds[0]);
}
