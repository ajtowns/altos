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
#include <ao_sample_profile.h>

struct stm_exception {
	uint32_t	r0;
	uint32_t	r1;
	uint32_t	r2;
	uint32_t	r3;
	uint32_t	r12;
	uint32_t	lr;
	uint32_t	pc;
	uint32_t	psr;
};

void
stm_tim10_isr(void)
{
	struct stm_exception	*ex;

	asm("mov %0,sp" : "=&r" (ex));

	stm_tim10.sr = 0;
	ao_sample_profile_point(ex->pc, stm_tim11.cnt, (ex->psr & 0xff) != 0);
}

uint16_t
ao_sample_profile_timer_start(void)
{
	/* Reset counts */
	stm_tim11.cnt = 0;
	stm_tim10.cnt = 0;

	/* Turn on timer 11 */
	stm_tim11.cr1 = ((0 << STM_TIM1011_CR1_CKD) |
			 (0 << STM_TIM1011_CR1_ARPE) |
			 (1 << STM_TIM1011_CR1_URS) |
			 (0 << STM_TIM1011_CR1_UDIS) |
			 (1 << STM_TIM1011_CR1_CEN));

	/* Turn on timer 10 */
	stm_tim10.cr1 = ((0 << STM_TIM1011_CR1_CKD) |
			 (0 << STM_TIM1011_CR1_ARPE) |
			 (1 << STM_TIM1011_CR1_URS) |
			 (0 << STM_TIM1011_CR1_UDIS) |
			 (1 << STM_TIM1011_CR1_CEN));
	return stm_tim11.cnt;
}

void
ao_sample_profile_timer_stop(void)
{
	stm_tim10.cr1 = 0;
	stm_tim11.cr1 = 0;
}

#if AO_APB2_PRESCALER > 1
#define TIMER_91011_SCALER 2
#else
#define TIMER_91011_SCALER 1
#endif

#define TIMER_10kHz	((AO_PCLK2 * TIMER_91011_SCALER) / 10000)
#define TIMER_1kHz	((AO_PCLK2 * TIMER_91011_SCALER) / 1000)

void
ao_sample_profile_timer_init(void)
{
	/* Turn on power for timer 10 and 11 */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_TIM10EN) | (1 << STM_RCC_APB2ENR_TIM11EN);

	/* Timer 10 is the 1kHz interrupt */
	stm_tim10.cr1 = 0;
	stm_tim10.psc = TIMER_10kHz;
	stm_tim10.arr = 9;
	stm_tim10.cnt = 0;

	/* Enable timer 10 update interrupt */
	stm_tim10.dier = (1 << STM_TIM1011_DIER_UIE);

	/* Poke timer to reload values */
	stm_tim10.egr |= (1 << STM_TIM1011_EGR_UG);

	/* Timer 11 is the 1kHz counter */
	stm_tim11.cr1 = 0;
	stm_tim11.psc = TIMER_1kHz;
	stm_tim11.arr = 0xffff;
	stm_tim11.cnt = 0;

	/* Disable interrupts for timer 11 */
	stm_tim11.dier = 0;

	/* Poke timer to reload values */
	stm_tim11.egr |= (1 << STM_TIM1011_EGR_UG);

	stm_tim10.sr = 0;
	stm_nvic_set_enable(STM_ISR_TIM10_POS);
	stm_nvic_set_priority(STM_ISR_TIM10_POS, AO_STM_NVIC_HIGH_PRIORITY);
}
