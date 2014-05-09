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
#include <ao_fast_timer.h>

static void (*ao_fast_timer_callback[AO_FAST_TIMER_MAX])(void);
static uint8_t ao_fast_timer_count;
static uint8_t ao_fast_timer_users;

static void
ao_fast_timer_enable(void)
{
	stm_tim6.cr1 = ((0 << STM_TIM67_CR1_ARPE) |
			(0 << STM_TIM67_CR1_OPM) |
			(1 << STM_TIM67_CR1_URS) |
			(0 << STM_TIM67_CR1_UDIS) |
			(1 << STM_TIM67_CR1_CEN));
}

static void
ao_fast_timer_disable(void)
{
	stm_tim6.cr1 = ((0 << STM_TIM67_CR1_ARPE) |
			(0 << STM_TIM67_CR1_OPM) |
			(1 << STM_TIM67_CR1_URS) |
			(0 << STM_TIM67_CR1_UDIS) |
			(0 << STM_TIM67_CR1_CEN));
}

void
ao_fast_timer_on(void (*callback)(void))
{
	ao_fast_timer_callback[ao_fast_timer_count] = callback;
	if (!ao_fast_timer_count++)
		ao_fast_timer_enable();
}

void
ao_fast_timer_off(void (*callback)(void))
{
	uint8_t	n;

	for (n = 0; n < ao_fast_timer_count; n++)
		if (ao_fast_timer_callback[n] == callback) {
			for (; n < ao_fast_timer_count-1; n++) {
				ao_fast_timer_callback[n] = ao_fast_timer_callback[n+1];
			}
			if (!--ao_fast_timer_count)
				ao_fast_timer_disable();
			break;
		}
}

void stm_tim6_isr(void)
{
	uint8_t	i;
	if (stm_tim6.sr & (1 << STM_TIM67_SR_UIF)) {
		stm_tim6.sr = 0;

		for (i = 0; i < ao_fast_timer_count; i++)
			(*ao_fast_timer_callback[i])();
	}
}

/*
 * According to the STM clock-configuration, timers run
 * twice as fast as the APB1 clock *if* the APB1 prescaler
 * is greater than 1.
 */

#if AO_APB1_PRESCALER > 1
#define TIMER_23467_SCALER 2
#else
#define TIMER_23467_SCALER 1
#endif

#ifndef FAST_TIMER_FREQ
#define FAST_TIMER_FREQ	10000
#endif

#define TIMER_FAST	((AO_PCLK1 * TIMER_23467_SCALER) / FAST_TIMER_FREQ)

void
ao_fast_timer_init(void)
{
	if (!ao_fast_timer_users) {
		stm_nvic_set_enable(STM_ISR_TIM6_POS);
		stm_nvic_set_priority(STM_ISR_TIM6_POS, AO_STM_NVIC_CLOCK_PRIORITY);

		/* Turn on timer 6 */
		stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM6EN);

		stm_tim6.psc = TIMER_FAST;
		stm_tim6.arr = 9;
		stm_tim6.cnt = 0;

		/* Enable update interrupt */
		stm_tim6.dier = (1 << STM_TIM67_DIER_UIE);

		/* Poke timer to reload values */
		stm_tim6.egr |= (1 << STM_TIM67_EGR_UG);

		stm_tim6.cr2 = (STM_TIM67_CR2_MMS_RESET << STM_TIM67_CR2_MMS);
		ao_fast_timer_disable();
	}
	if (ao_fast_timer_users == AO_FAST_TIMER_MAX)
		ao_panic(AO_PANIC_FAST_TIMER);
	ao_fast_timer_users++;
}

