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

static volatile __data uint16_t ao_tick_count;

uint16_t ao_time(void)
{
	uint16_t	v;
	ao_arch_critical(
		v = ao_tick_count;
		);
	return v;
}

static __xdata uint8_t ao_forever;

void
ao_delay(uint16_t ticks)
{
	ao_alarm(ticks);
	ao_sleep(&ao_forever);
}

#if HAS_ADC
volatile __data uint8_t	ao_adc_interval = 1;
volatile __data uint8_t	ao_adc_count;
#endif

void
ao_debug_out(char c);


void stm_tim6_isr(void)
{
	if (stm_tim6.sr & (1 << STM_TIM67_SR_UIF)) {
		stm_tim6.sr = 0;
		++ao_tick_count;
#if HAS_ADC
		if (++ao_adc_count == ao_adc_interval) {
			ao_adc_count = 0;
			ao_adc_poll();
		}
#endif
	}
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval) __critical
{
	ao_adc_interval = interval;
	ao_adc_count = 0;
}
#endif

#define TIMER_10kHz	(STM_APB1 / 10000)

void
ao_timer_init(void)
{
	stm_nvic_set_enable(STM_ISR_TIM6_POS);
	stm_nvic_set_priority(STM_ISR_TIM6_POS, 1);

	/* Turn on timer 6 */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM6EN);

	stm_tim6.psc = TIMER_10kHz;
	stm_tim6.arr = 100;
	stm_tim6.cnt = 0;

	/* Enable update interrupt */
	stm_tim6.dier = (1 << STM_TIM67_DIER_UIE);

	/* Poke timer to reload values */
	stm_tim6.egr |= (1 << STM_TIM67_EGR_UG);

	stm_tim6.cr2 = (STM_TIM67_CR2_MMS_RESET << STM_TIM67_CR2_MMS);

	/* And turn it on */
	stm_tim6.cr1 = ((0 << STM_TIM67_CR1_ARPE) |
			(0 << STM_TIM67_CR1_OPM) |
			(1 << STM_TIM67_CR1_URS) |
			(0 << STM_TIM67_CR1_UDIS) |
			(1 << STM_TIM67_CR1_CEN));
}

void
ao_clock_init(void)
{
	uint32_t	cfgr;
	uint32_t	cr;
	
	/* Set flash latency to tolerate 32MHz SYSCLK  -> 1 wait state */
	uint32_t	acr = stm_flash.acr;

	/* Enable 64-bit access and prefetch */
	acr |= (1 << STM_FLASH_ACR_ACC64) | (1 << STM_FLASH_ACR_PRFEN);
	stm_flash.acr = acr;

	/* Enable 1 wait state so the CPU can run at 32MHz */
	/* (haven't managed to run the CPU at 32MHz yet, it's at 16MHz) */
	acr |= (1 << STM_FLASH_ACR_LATENCY);
	stm_flash.acr = acr;

	/* HCLK to 16MHz -> AHB prescaler = /1 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE);
	cfgr |= (STM_RCC_CFGR_HPRE_DIV_1 << STM_RCC_CFGR_HPRE);
	stm_rcc.cfgr = cfgr;
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE)) !=
	       (STM_RCC_CFGR_HPRE_DIV_1 << STM_RCC_CFGR_HPRE))
		asm ("nop");
#define STM_AHB_PRESCALER	1

	/* PCLK1 to 16MHz -> APB1 Prescaler = 1 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE1_MASK << STM_RCC_CFGR_PPRE1);
	cfgr |= (STM_RCC_CFGR_PPRE1_DIV_1 << STM_RCC_CFGR_PPRE1);
	stm_rcc.cfgr = cfgr;
#define STM_APB1_PRESCALER	1

	/* PCLK2 to 16MHz -> APB2 Prescaler = 1 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE2_MASK << STM_RCC_CFGR_PPRE2);
	cfgr |= (STM_RCC_CFGR_PPRE2_DIV_1 << STM_RCC_CFGR_PPRE2);
	stm_rcc.cfgr = cfgr;
#define STM_APB2_PRESCALER	1

	/* Enable power interface clock */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_PWREN);

	/* Set voltage range to 1.8V */

	/* poll VOSF bit in PWR_CSR. Wait until it is reset to 0 */
	while ((stm_pwr.csr & (1 << STM_PWR_CSR_VOSF)) != 0)
		asm("nop");

	/* Configure voltage scaling range */
	cr = stm_pwr.cr;
	cr &= ~(STM_PWR_CR_VOS_MASK << STM_PWR_CR_VOS);
	cr |= (STM_PWR_CR_VOS_1_8 << STM_PWR_CR_VOS);
	stm_pwr.cr = cr;

	/* poll VOSF bit in PWR_CSR. Wait until it is reset to 0 */
	while ((stm_pwr.csr & (1 << STM_PWR_CSR_VOSF)) != 0)
		asm("nop");

	/* Enable HSI RC clock 16MHz */
	if (!(stm_rcc.cr & (1 << STM_RCC_CR_HSIRDY))) {
		stm_rcc.cr |= (1 << STM_RCC_CR_HSION);
		while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSIRDY)))
			asm("nop");
	}
#define STM_HSI 16000000

	/* Switch to direct HSI for SYSCLK */
	if ((stm_rcc.cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	    (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS)) {
		cfgr = stm_rcc.cfgr;
		cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
		cfgr |= (STM_RCC_CFGR_SW_HSI << STM_RCC_CFGR_SW);
		stm_rcc.cfgr = cfgr;
		while ((stm_rcc.cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
		       (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS))
			asm("nop");
	}

	/* Disable the PLL */
	stm_rcc.cr &= ~(1 << STM_RCC_CR_PLLON);
	while (stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY))
		asm("nop");
	
	/* PLLVCO to 96MHz (for USB) -> PLLMUL = 6, PLLDIV = 4 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PLLMUL_MASK << STM_RCC_CFGR_PLLMUL);
	cfgr &= ~(STM_RCC_CFGR_PLLDIV_MASK << STM_RCC_CFGR_PLLDIV);

//	cfgr |= (STM_RCC_CFGR_PLLMUL_6 << STM_RCC_CFGR_PLLMUL);
//	cfgr |= (STM_RCC_CFGR_PLLDIV_3 << STM_RCC_CFGR_PLLDIV);

	cfgr |= (STM_RCC_CFGR_PLLMUL_6 << STM_RCC_CFGR_PLLMUL);
	cfgr |= (STM_RCC_CFGR_PLLDIV_4 << STM_RCC_CFGR_PLLDIV);

#define STM_PLLMUL	6
#define STM_PLLDIV	4

	/* PLL source to HSI */
	cfgr &= ~(1 << STM_RCC_CFGR_PLLSRC);

#define STM_PLLSRC	STM_HSI

	stm_rcc.cfgr = cfgr;

	/* Enable the PLL and wait for it */
	stm_rcc.cr |= (1 << STM_RCC_CR_PLLON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY)))
		asm("nop");

	/* Switch to the PLL for the system clock */

	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
	cfgr |= (STM_RCC_CFGR_SW_PLL << STM_RCC_CFGR_SW);
	stm_rcc.cfgr = cfgr;
	for (;;) {
		uint32_t	c, part, mask, val;

		c = stm_rcc.cfgr;
		mask = (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS);
		val = (STM_RCC_CFGR_SWS_PLL << STM_RCC_CFGR_SWS);
		part = c & mask;
		if (part == val)
			break;
	}
}
