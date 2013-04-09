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
#include <ao_task.h>

volatile AO_TICK_TYPE ao_tick_count;

AO_TICK_TYPE
ao_time(void)
{
	return ao_tick_count;
}

#if AO_DATA_ALL
volatile __data uint8_t	ao_data_interval = 1;
volatile __data uint8_t	ao_data_count;
#endif

void
ao_debug_out(char c);


void stm_tim6_isr(void)
{
	if (stm_tim6.sr & (1 << STM_TIM67_SR_UIF)) {
		stm_tim6.sr = 0;
		++ao_tick_count;
#if HAS_TASK_QUEUE
		if (ao_task_alarm_tick && (int16_t) (ao_tick_count - ao_task_alarm_tick) >= 0)
			ao_task_check_alarm((uint16_t) ao_tick_count);
#endif
#if AO_DATA_ALL
		if (++ao_data_count == ao_data_interval) {
			ao_data_count = 0;
			ao_adc_poll();
#if (AO_DATA_ALL & ~(AO_DATA_ADC))
			ao_wakeup((void *) &ao_data_count);
#endif
		}
#endif
	}
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval)
{
	ao_arch_critical(
		ao_data_interval = interval;
		ao_data_count = 0;
		);
}
#endif

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

#define TIMER_10kHz	((AO_PCLK1 * TIMER_23467_SCALER) / 10000)

void
ao_timer_init(void)
{
	stm_nvic_set_enable(STM_ISR_TIM6_POS);
	stm_nvic_set_priority(STM_ISR_TIM6_POS, AO_STM_NVIC_CLOCK_PRIORITY);

	/* Turn on timer 6 */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM6EN);

	stm_tim6.psc = TIMER_10kHz;
	stm_tim6.arr = 99;
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
	
	/* Switch to MSI while messing about */
	stm_rcc.cr |= (1 << STM_RCC_CR_MSION);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_MSIRDY)))
		asm("nop");

	/* reset SW, HPRE, PPRE1, PPRE2, MCOSEL and MCOPRE */
	stm_rcc.cfgr &= (uint32_t)0x88FFC00C;

	/* reset HSION, HSEON, CSSON and PLLON bits */
	stm_rcc.cr &= 0xeefefffe;
	
	/* reset PLLSRC, PLLMUL and PLLDIV bits */
	stm_rcc.cfgr &= 0xff02ffff;
	
	/* Disable all interrupts */
	stm_rcc.cir = 0;

#if AO_HSE
#if AO_HSE_BYPASS
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEBYP);
#else
	stm_rcc.cr &= ~(1 << STM_RCC_CR_HSEBYP);
#endif
	/* Enable HSE clock */
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSERDY)))
		asm("nop");

#define STM_RCC_CFGR_SWS_TARGET_CLOCK		(STM_RCC_CFGR_SWS_HSE << STM_RCC_CFGR_SWS)
#define STM_RCC_CFGR_SW_TARGET_CLOCK		(STM_RCC_CFGR_SW_HSE)
#define STM_PLLSRC				AO_HSE
#define STM_RCC_CFGR_PLLSRC_TARGET_CLOCK	(1 << STM_RCC_CFGR_PLLSRC)
#else
#define STM_HSI 				16000000
#define STM_RCC_CFGR_SWS_TARGET_CLOCK		(STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS)
#define STM_RCC_CFGR_SW_TARGET_CLOCK		(STM_RCC_CFGR_SW_HSI)
#define STM_PLLSRC				STM_HSI
#define STM_RCC_CFGR_PLLSRC_TARGET_CLOCK	(0 << STM_RCC_CFGR_PLLSRC)
#endif

#if !AO_HSE || HAS_ADC
	/* Enable HSI RC clock 16MHz */
	stm_rcc.cr |= (1 << STM_RCC_CR_HSION);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSIRDY)))
		asm("nop");
#endif

	/* Set flash latency to tolerate 32MHz SYSCLK  -> 1 wait state */

	/* Enable 64-bit access and prefetch */
	stm_flash.acr |= (1 << STM_FLASH_ACR_ACC64);
	stm_flash.acr |= (1 << STM_FLASH_ACR_PRFEN);

	/* Enable 1 wait state so the CPU can run at 32MHz */
	/* (haven't managed to run the CPU at 32MHz yet, it's at 16MHz) */
	stm_flash.acr |= (1 << STM_FLASH_ACR_LATENCY);

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

	/* HCLK to 16MHz -> AHB prescaler = /1 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE);
	cfgr |= (AO_RCC_CFGR_HPRE_DIV << STM_RCC_CFGR_HPRE);
	stm_rcc.cfgr = cfgr;
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE)) !=
	       (AO_RCC_CFGR_HPRE_DIV << STM_RCC_CFGR_HPRE))
		asm ("nop");

	/* APB1 Prescaler = AO_APB1_PRESCALER */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE1_MASK << STM_RCC_CFGR_PPRE1);
	cfgr |= (AO_RCC_CFGR_PPRE1_DIV << STM_RCC_CFGR_PPRE1);
	stm_rcc.cfgr = cfgr;

	/* APB2 Prescaler = AO_APB2_PRESCALER */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE2_MASK << STM_RCC_CFGR_PPRE2);
	cfgr |= (AO_RCC_CFGR_PPRE2_DIV << STM_RCC_CFGR_PPRE2);
	stm_rcc.cfgr = cfgr;

	/* Disable the PLL */
	stm_rcc.cr &= ~(1 << STM_RCC_CR_PLLON);
	while (stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY))
		asm("nop");
	
	/* PLLVCO to 96MHz (for USB) -> PLLMUL = 6, PLLDIV = 4 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PLLMUL_MASK << STM_RCC_CFGR_PLLMUL);
	cfgr &= ~(STM_RCC_CFGR_PLLDIV_MASK << STM_RCC_CFGR_PLLDIV);

	cfgr |= (AO_RCC_CFGR_PLLMUL << STM_RCC_CFGR_PLLMUL);
	cfgr |= (AO_RCC_CFGR_PLLDIV << STM_RCC_CFGR_PLLDIV);

	/* PLL source */
	cfgr &= ~(1 << STM_RCC_CFGR_PLLSRC);
	cfgr |= STM_RCC_CFGR_PLLSRC_TARGET_CLOCK;

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

#if 0
	stm_rcc.apb2rstr = 0xffff;
	stm_rcc.apb1rstr = 0xffff;
	stm_rcc.ahbrstr = 0x3f;
	stm_rcc.ahbenr = (1 << STM_RCC_AHBENR_FLITFEN);
	stm_rcc.apb2enr = 0;
	stm_rcc.apb1enr = 0;
	stm_rcc.ahbrstr = 0;
	stm_rcc.apb1rstr = 0;
	stm_rcc.apb2rstr = 0;
#endif

	/* Clear reset flags */
	stm_rcc.csr |= (1 << STM_RCC_CSR_RMVF);


#if DEBUG_THE_CLOCK
	/* Output SYSCLK on PA8 for measurments */

	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);
	
	stm_afr_set(&stm_gpioa, 8, STM_AFR_AF0);
	stm_moder_set(&stm_gpioa, 8, STM_MODER_ALTERNATE);
	stm_ospeedr_set(&stm_gpioa, 8, STM_OSPEEDR_40MHz);

	stm_rcc.cfgr |= (STM_RCC_CFGR_MCOPRE_DIV_1 << STM_RCC_CFGR_MCOPRE);
	stm_rcc.cfgr |= (STM_RCC_CFGR_MCOSEL_HSE << STM_RCC_CFGR_MCOSEL);
#endif
}
