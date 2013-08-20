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

#include "ao.h"

#ifndef BEEPER_CHANNEL
#define BEEPER_CHANNEL	1
#endif

void
ao_beep(uint8_t beep)
{
	if (beep == 0) {
		stm_tim3.cr1 = 0;
		stm_rcc.apb1enr &= ~(1 << STM_RCC_APB1ENR_TIM3EN);
	} else {
		stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM3EN);

		stm_tim3.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
				(STM_TIM234_CR2_MMS_RESET << STM_TIM234_CR2_MMS) |
				(0 << STM_TIM234_CR2_CCDS));

		/* Set prescaler to match cc1111 clocks
		 */
		stm_tim3.psc = AO_TIM23467_CLK / 750000;

		/* 1. Select the counter clock (internal, external, prescaler).
		 *
		 * Setting SMCR to zero means use the internal clock
		 */

		stm_tim3.smcr = 0;

		/* 2. Write the desired data in the TIMx_ARR and TIMx_CCRx registers. */
		stm_tim3.arr = beep;
		stm_tim3.ccr1 = beep;

		/* 3. Set the CCxIE and/or CCxDE bits if an interrupt and/or a
		 * DMA request is to be generated.
		 */
		/* don't want this */

		/* 4. Select the output mode. For example, you must write
		 *  OCxM=011, OCxPE=0, CCxP=0 and CCxE=1 to toggle OCx output
		 *  pin when CNT matches CCRx, CCRx preload is not used, OCx
		 *  is enabled and active high.
		 */

#if BEEPER_CHANNEL == 1
		stm_tim3.ccmr1 = ((0 << STM_TIM234_CCMR1_OC2CE) |
				  (STM_TIM234_CCMR1_OC2M_FROZEN << STM_TIM234_CCMR1_OC2M) |
				  (0 << STM_TIM234_CCMR1_OC2PE) |
				  (0 << STM_TIM234_CCMR1_OC2FE) |
				  (STM_TIM234_CCMR1_CC2S_OUTPUT << STM_TIM234_CCMR1_CC2S) |

				  (0 << STM_TIM234_CCMR1_OC1CE) |
				  (STM_TIM234_CCMR1_OC1M_TOGGLE << STM_TIM234_CCMR1_OC1M) |
				  (0 << STM_TIM234_CCMR1_OC1PE) |
				  (0 << STM_TIM234_CCMR1_OC1FE) |
				  (STM_TIM234_CCMR1_CC1S_OUTPUT << STM_TIM234_CCMR1_CC1S));

		stm_tim3.ccer = ((0 << STM_TIM234_CCER_CC4NP) |
				 (0 << STM_TIM234_CCER_CC4P) |
				 (0 << STM_TIM234_CCER_CC4E) |
				 (0 << STM_TIM234_CCER_CC3NP) |
				 (0 << STM_TIM234_CCER_CC3P) |
				 (0 << STM_TIM234_CCER_CC3E) |
				 (0 << STM_TIM234_CCER_CC2NP) |
				 (0 << STM_TIM234_CCER_CC2P) |
				 (0 << STM_TIM234_CCER_CC2E) |
				 (0 << STM_TIM234_CCER_CC1NP) |
				 (0 << STM_TIM234_CCER_CC1P) |
				 (1 << STM_TIM234_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 4
		stm_tim3.ccmr2 = ((0 << STM_TIM234_CCMR2_OC4CE) |
				  (STM_TIM234_CCMR2_OC4M_TOGGLE << STM_TIM234_CCMR2_OC4M) |
				  (0 << STM_TIM234_CCMR2_OC4PE) |
				  (0 << STM_TIM234_CCMR2_OC4FE) |
				  (STM_TIM234_CCMR2_CC4S_OUTPUT << STM_TIM234_CCMR2_CC4S) |

				  (0 << STM_TIM234_CCMR2_OC3CE) |
				  (STM_TIM234_CCMR2_OC3M_FROZEN << STM_TIM234_CCMR2_OC3M) |
				  (0 << STM_TIM234_CCMR2_OC3PE) |
				  (0 << STM_TIM234_CCMR2_OC3FE) |
				  (STM_TIM234_CCMR2_CC3S_OUTPUT << STM_TIM234_CCMR2_CC3S));

		stm_tim3.ccer = ((0 << STM_TIM234_CCER_CC4NP) |
				 (0 << STM_TIM234_CCER_CC4P) |
				 (1 << STM_TIM234_CCER_CC4E) |
				 (0 << STM_TIM234_CCER_CC3NP) |
				 (0 << STM_TIM234_CCER_CC3P) |
				 (0 << STM_TIM234_CCER_CC3E) |
				 (0 << STM_TIM234_CCER_CC2NP) |
				 (0 << STM_TIM234_CCER_CC2P) |
				 (0 << STM_TIM234_CCER_CC2E) |
				 (0 << STM_TIM234_CCER_CC1NP) |
				 (0 << STM_TIM234_CCER_CC1P) |
				 (0 << STM_TIM234_CCER_CC1E));
#endif


		/* 5. Enable the counter by setting the CEN bit in the TIMx_CR1 register. */

		stm_tim3.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
				(0 << STM_TIM234_CR1_ARPE) |
				(STM_TIM234_CR1_CMS_EDGE << STM_TIM234_CR1_CMS) |
				(0 << STM_TIM234_CR1_DIR) |
				(0 << STM_TIM234_CR1_OPM) |
				(0 << STM_TIM234_CR1_URS) |
				(0 << STM_TIM234_CR1_UDIS) |
				(1 << STM_TIM234_CR1_CEN));

		/* Update the values */
		stm_tim3.egr = (1 << STM_TIM234_EGR_UG);
	}
}

void
ao_beep_for(uint8_t beep, uint16_t ticks) __reentrant
{
	ao_beep(beep);
	ao_delay(ticks);
	ao_beep(0);
}

void
ao_beep_init(void)
{
#if BEEPER_CHANNEL == 1

	/* Our beeper is on PC6, which is hooked to TIM3_CH1.
	 */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOCEN);

	stm_afr_set(&stm_gpioc, 6, STM_AFR_AF2);
#endif
#if BEEPER_CHANNEL == 4

	/* Our beeper is on PB1, which is hooked to TIM3_CH4.
	 */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);

	stm_afr_set(&stm_gpiob, 1, STM_AFR_AF2);
#endif

	/* Leave the timer off until requested */
	
	stm_rcc.apb1enr &= ~(1 << STM_RCC_APB1ENR_TIM3EN);
}
