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

#define T2_CLOCK_DIVISOR	8	/* 24e6/8 = 3e6 */
#define T2_SAMPLE_TIME		30000	/* 3e6/30000 = 100 */

#if HAS_ADC
volatile __data uint8_t	ao_adc_interval = 1;
volatile __data uint8_t	ao_adc_count;
#endif

void
ao_debug_out(char c);


void tim2_isr(void)
{
	++ao_tick_count;
#if HAS_ADC
	if (++ao_adc_count == ao_adc_interval) {
		ao_adc_count = 0;
		ao_adc_poll();
	}
#endif
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval) __critical
{
	ao_adc_interval = interval;
	ao_adc_count = 0;
}
#endif

void
ao_timer_init(void)
{
}

void
ao_clock_init(void)
{
	uint32_t	cfgr;
	
	/* Set flash latency to tolerate 32MHz SYSCLK  -> 1 wait state */
	uint32_t	acr = STM_FLASH->acr;

	/* Enable 64-bit access and prefetch */
	acr |= (1 << STM_FLASH_ACR_ACC64) | (1 << STM_FLASH_ACR_PRFEN);
	STM_FLASH->acr = acr;

	/* Enable 1 wait state so the CPU can run at 32MHz */
	acr |= (1 << STM_FLASH_ACR_LATENCY);
	STM_FLASH->acr = acr;

	/* Enable HSI RC clock 16MHz */
	if (!(STM_RCC->cr & (1 << STM_RCC_CR_HSIRDY))) {
		STM_RCC->cr |= (1 << STM_RCC_CR_HSION);
		while (!(STM_RCC->cr & (1 << STM_RCC_CR_HSIRDY)))
			asm("nop");
	}

	/* Switch to direct HSI for SYSCLK */
	if ((STM_RCC->cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	    (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS)) {
		cfgr = STM_RCC->cfgr;
		cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
		cfgr |= (STM_RCC_CFGR_SW_HSI << STM_RCC_CFGR_SW);
		STM_RCC->cfgr = cfgr;
		while ((STM_RCC->cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
		       (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS))
			asm("nop");
	}

	/* Disable the PLL */
	STM_RCC->cr &= ~(1 << STM_RCC_CR_PLLON);
	while (STM_RCC->cr & (1 << STM_RCC_CR_PLLRDY))
		asm("nop");
	
	/* PLLVCO to 96MHz (for USB) -> PLLMUL = 6 */
	cfgr = STM_RCC->cfgr;
	cfgr &= ~(STM_RCC_CFGR_PLLMUL_MASK << STM_RCC_CFGR_PLLMUL);
	cfgr |= (STM_RCC_CFGR_PLLMUL_6 << STM_RCC_CFGR_PLLMUL);
	
	/* SYSCLK to 32MHz from PLL clock -> PLLDIV = /3 */
	cfgr &= ~(STM_RCC_CFGR_PLLDIV_MASK << STM_RCC_CFGR_PLLDIV);
	cfgr |= (STM_RCC_CFGR_PLLDIV_3 << STM_RCC_CFGR_PLLDIV);

	/* PLL source to HSI */
	cfgr &= ~(1 << STM_RCC_CFGR_PLLSRC);

	STM_RCC->cfgr = cfgr;

	/* Enable the PLL and wait for it */
	STM_RCC->cr |= (1 << STM_RCC_CR_PLLON);
	while (!(STM_RCC->cr & (1 << STM_RCC_CR_PLLRDY)))
		asm("nop");

	/* Switch to the PLL for the system clock */

	cfgr = STM_RCC->cfgr;
	cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
	cfgr |= (STM_RCC_CFGR_SW_PLL << STM_RCC_CFGR_SW);
	STM_RCC->cfgr = cfgr;
	while ((STM_RCC->cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	       (STM_RCC_CFGR_SWS_PLL << STM_RCC_CFGR_SWS))
		asm("nop");

	/* HCLK to 32MHz -> AHB prescaler = /1 */
	cfgr = STM_RCC->cfgr;
	cfgr &= ~(STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE);
	cfgr |= (STM_RCC_CFGR_HPRE_DIV_1 << STM_RCC_CFGR_HPRE);
	STM_RCC->cfgr = cfgr;
	while ((STM_RCC->cfgr & (STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE)) !=
	       (STM_RCC_CFGR_HPRE_DIV_1 << STM_RCC_CFGR_HPRE))
		asm ("nop");

	/* PCLK1 to 16MHz -> APB1 Prescaler = 2 */
	cfgr = STM_RCC->cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE1_MASK << STM_RCC_CFGR_PPRE1);
	cfgr |= (STM_RCC_CFGR_PPRE1_DIV_2 << STM_RCC_CFGR_PPRE1);
	STM_RCC->cfgr = cfgr;

	/* PCLK2 to 16MHz -> APB2 Prescaler = 2 */
	cfgr = STM_RCC->cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE2_MASK << STM_RCC_CFGR_PPRE2);
	cfgr |= (STM_RCC_CFGR_PPRE2_DIV_2 << STM_RCC_CFGR_PPRE2);
	STM_RCC->cfgr = cfgr;

}
