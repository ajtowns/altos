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
#if HAS_FAKE_FLIGHT
#include <ao_fake_flight.h>
#endif

#ifndef HAS_TICK
#define HAS_TICK 1
#endif

#if HAS_TICK
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

void stm_systick_isr(void)
{
	if (stm_systick.csr & (1 << STM_SYSTICK_CSR_COUNTFLAG)) {
		++ao_tick_count;
#if HAS_TASK_QUEUE
		if (ao_task_alarm_tick && (int16_t) (ao_tick_count - ao_task_alarm_tick) >= 0)
			ao_task_check_alarm((uint16_t) ao_tick_count);
#endif
#if AO_DATA_ALL
		if (++ao_data_count == ao_data_interval) {
			ao_data_count = 0;
#if HAS_FAKE_FLIGHT
			if (ao_fake_flight_active)
				ao_fake_flight_poll();
			else
#endif
				ao_adc_poll();
#if (AO_DATA_ALL & ~(AO_DATA_ADC))
			ao_wakeup((void *) &ao_data_count);
#endif
		}
#endif
#ifdef AO_TIMER_HOOK
		AO_TIMER_HOOK;
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

#define SYSTICK_RELOAD (AO_SYSTICK / 100 - 1)

void
ao_timer_init(void)
{
	stm_systick.rvr = SYSTICK_RELOAD;
	stm_systick.cvr = 0;
	stm_systick.csr = ((1 << STM_SYSTICK_CSR_ENABLE) |
			   (1 << STM_SYSTICK_CSR_TICKINT) |
			   (STM_SYSTICK_CSR_CLKSOURCE_HCLK_8 << STM_SYSTICK_CSR_CLKSOURCE));
}

#endif

void
ao_clock_init(void)
{
	uint32_t	cfgr;
	uint32_t	cr;
	
	/* Switch to MSI while messing about */
	stm_rcc.cr |= (1 << STM_RCC_CR_MSION);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_MSIRDY)))
		ao_arch_nop();

	stm_rcc.cfgr = (stm_rcc.cfgr & ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW)) |
		(STM_RCC_CFGR_SW_MSI << STM_RCC_CFGR_SW);

	/* wait for system to switch to MSI */
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	       (STM_RCC_CFGR_SWS_MSI << STM_RCC_CFGR_SWS))
		ao_arch_nop();

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
