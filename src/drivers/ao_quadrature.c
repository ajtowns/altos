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
#include <ao_fast_timer.h>
#include <ao_event.h>

__xdata int32_t ao_quadrature_count[AO_QUADRATURE_COUNT];
static uint8_t	ao_quadrature_state[AO_QUADRATURE_COUNT];
static int8_t	ao_quadrature_raw[AO_QUADRATURE_COUNT];

#define BIT(a,b)	((a) | ((b) << 1))
#define STATE(old_a, old_b, new_a, new_b)	(((BIT(old_a, old_b) << 2) | BIT(new_a, new_b)))

#define port(q)	AO_QUADRATURE_ ## q ## _PORT
#define bita(q) AO_QUADRATURE_ ## q ## _A
#define bitb(q) AO_QUADRATURE_ ## q ## _B
#define pina(q) AO_QUADRATURE_ ## q ## _A ## _PIN
#define pinb(q) AO_QUADRATURE_ ## q ## _B ## _PIN
#define isr(q)  ao_quadrature_isr_ ## q

static inline uint16_t
ao_quadrature_read(struct stm_gpio *gpio, uint8_t pin_a, uint8_t pin_b) {
	uint16_t	v = stm_gpio_get_all(gpio);

	return ~((((v >> pin_a) & 1) | (((v >> pin_b) & 1) << 1))) & 3;
}

#define _ao_quadrature_get(q)	ao_quadrature_read(port(q), bita(q), bitb(q))

static void
_ao_quadrature_queue(uint8_t q, int8_t step)
{
	ao_quadrature_count[q] += step;
#if AO_EVENT
	ao_event_put_isr(AO_EVENT_QUADRATURE, q, step);
#endif
	ao_wakeup(&ao_quadrature_count[q]);
}

static const int8_t	step[16] = {
	[STATE(0,0,0,0)] = 0,
	[STATE(0,0,0,1)] = -1,
	[STATE(0,0,1,0)] = 1,
	[STATE(0,0,1,1)] = 0,
	[STATE(0,1,0,0)] = 1,
	[STATE(0,1,1,0)] = 0,
	[STATE(0,1,1,1)] = -1,
	[STATE(1,0,0,0)] = -1,
	[STATE(1,0,0,1)] = 0,
	[STATE(1,0,1,0)] = 0,
	[STATE(1,0,1,1)] = 1,
	[STATE(1,1,0,0)] = 0,
	[STATE(1,1,0,1)] = 1,
	[STATE(1,1,1,0)] = -1,
	[STATE(1,1,1,1)] = 0
};

static void
_ao_quadrature_set(uint8_t q, uint8_t value) {
	uint8_t v;
	
	v = ao_quadrature_state[q] & 3;
	value = value & 3;

	if (v == value)
		return;
	
	ao_quadrature_state[q] = (v << 2) | value;

	ao_quadrature_raw[q] += step[ao_quadrature_state[q]];
	if (value == 0) {
		if (ao_quadrature_raw[q] == 4)
			_ao_quadrature_queue(q, 1);
		else if (ao_quadrature_raw[q] == -4)
			_ao_quadrature_queue(q, -1);
		ao_quadrature_raw[q] = 0;
	}
}

static void
ao_quadrature_isr(void)
{
	_ao_quadrature_set(0, _ao_quadrature_get(0));
	_ao_quadrature_set(1, _ao_quadrature_get(1));
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
	if (q >= AO_QUADRATURE_COUNT) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	printf ("count %d state %x raw %d\n",
		ao_quadrature_count[q],
		ao_quadrature_state[q],
		ao_quadrature_raw[q]);
#if 0
	for (;;) {
		int32_t	c;
		flush();
		c = ao_quadrature_wait(q);
		printf ("new count %6d\n", c);
		if (c == 100)
			break;
	}
#endif
}

static const struct ao_cmds ao_quadrature_cmds[] = {
	{ ao_quadrature_test,	"q <unit>\0Test quadrature" },
	{ 0, NULL }
};

#define init(q) do {					\
		ao_enable_input(port(q), bita(q), 0);	\
		ao_enable_input(port(q), bitb(q), 0);	\
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
	ao_fast_timer_init();
	ao_fast_timer_on(ao_quadrature_isr);
	ao_cmd_register(&ao_quadrature_cmds[0]);
}
