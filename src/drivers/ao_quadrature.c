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

__xdata int32_t ao_quadrature_count;

static uint8_t	wait_clear;

static void
ao_quadrature_isr(void)
{
	if (wait_clear) {
		wait_clear = 0;
		ao_exti_set_mode(AO_QUADRATURE_PORT, AO_QUADRATURE_A, AO_EXTI_MODE_RISING);
	} else {
		wait_clear = 1;
		ao_exti_set_mode(AO_QUADRATURE_PORT, AO_QUADRATURE_A, AO_EXTI_MODE_FALLING);
		if (ao_gpio_get(AO_QUADRATURE_PORT, AO_QUADRATURE_B, AO_QUADRATURE_B_PIN))
			ao_quadrature_count++;
		else
			ao_quadrature_count--;
		ao_wakeup(&ao_quadrature_count);
	}
}

int32_t
ao_quadrature_poll(void)
{
	int32_t	ret;
	ao_arch_critical(ret = ao_quadrature_count;);
	return ret;
}

int32_t
ao_quadrature_wait(void)
{
	ao_sleep(&ao_quadrature_count);
	return ao_quadrature_poll();
}

static void
ao_quadrature_test(void)
{
#if 0
	for (;;) {
		int32_t	c;
		printf ("waiting...\n");
		flush();
		c = ao_quadrature_wait();
		printf ("new count %d\n", c);
		if (ao_stdin_ready)
			break;
	}
#endif
	uint8_t	a, old_a, b, old_b;

	old_a = 2; old_b = 2;
	for (;;) {
		a = ao_gpio_get(AO_QUADRATURE_PORT, AO_QUADRATURE_A, AO_QUADRATURE_A_PIN);
		b = ao_gpio_get(AO_QUADRATURE_PORT, AO_QUADRATURE_B, AO_QUADRATURE_B_PIN);
		if (a != old_a || b != old_b) {
			printf ("A %d B %d\n", a, b);
			flush();
			ao_yield();
			old_a = a;
			old_b = b;
		}
		if (ao_stdin_ready)
			break;
	}
		
}

static const struct ao_cmds ao_quadrature_cmds[] = {
	{ ao_quadrature_test,	"q\0Test quadrature" },
	{ 0, NULL }
};

void
ao_quadrature_init(void)
{
	ao_quadrature_count = 0;

	ao_enable_port(AO_QUADRATURE_PORT);
	ao_exti_setup(AO_QUADRATURE_PORT, AO_QUADRATURE_A,
		      AO_EXTI_MODE_PULL_UP|AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED,
		      ao_quadrature_isr);
	ao_exti_enable(AO_QUADRATURE_PORT, AO_QUADRATURE_A);
	ao_enable_input(AO_QUADRATURE_PORT, AO_QUADRATURE_B, AO_EXTI_MODE_PULL_UP);
	ao_cmd_register(&ao_quadrature_cmds[0]);
}
