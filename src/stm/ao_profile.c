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
#include <ao_profile.h>

static void ao_profile_test(void)
{
	uint8_t	i;
	uint32_t	ticks[20];

	for (i = 0; i < 20; i++) {
		ticks[i] = ao_profile_tick();
		ao_delay(0);
	}
	for (i = 0; i < 19; i++)
		printf ("%d\n", ticks[i+1] - ticks[i]);
}

static const struct ao_cmds ao_profile_cmds[] = {
	{ ao_profile_test,	"P\0Test profile counter" },
	{ 0, NULL }
};

void ao_profile_init(void)
{
	/* Turn on timer 2 and 4 */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM2EN) |
		(1 << STM_RCC_APB1ENR_TIM4EN);

	/* disable timers */
	stm_tim4.cr1 = 0;
	stm_tim2.cr1 = 0;

	/* tim4 is master */


	stm_tim4.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
			(STM_TIM234_CR2_MMS_UPDATE << STM_TIM234_CR2_MMS) |
			(0 << STM_TIM234_CR2_CCDS));

	stm_tim4.smcr = ((0 << STM_TIM234_SMCR_ETP) |
			 (0 << STM_TIM234_SMCR_ECE) |
			 (STM_TIM234_SMCR_ETPS_OFF << STM_TIM234_SMCR_ETPS) |
			 (STM_TIM234_SMCR_ETF_NONE << STM_TIM234_SMCR_ETF) |
			 (0 << STM_TIM234_SMCR_MSM) |
			 (STM_TIM234_SMCR_TS_ITR3 << STM_TIM234_SMCR_TS) |
			 (0 << STM_TIM234_SMCR_OCCS) |
			 (STM_TIM234_SMCR_SMS_DISABLE << STM_TIM234_SMCR_SMS));

	stm_tim4.dier = 0;
	stm_tim4.sr = 0;

	stm_tim4.psc = 31;
	stm_tim4.cnt = 0;
	stm_tim4.arr = 0xffff;

	/* tim2 is slaved to tim4 */

	stm_tim2.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
			(STM_TIM234_CR2_MMS_ENABLE << STM_TIM234_CR2_MMS) |
			(0 << STM_TIM234_CR2_CCDS));
	stm_tim2.smcr = ((0 << STM_TIM234_SMCR_ETP) |
			 (0 << STM_TIM234_SMCR_ECE) |
			 (STM_TIM234_SMCR_ETPS_OFF << STM_TIM234_SMCR_ETPS) |
			 (STM_TIM234_SMCR_ETF_NONE << STM_TIM234_SMCR_ETF) |
			 (0 << STM_TIM234_SMCR_MSM) |
			 (STM_TIM234_SMCR_TS_ITR3 << STM_TIM234_SMCR_TS) |
			 (0 << STM_TIM234_SMCR_OCCS) |
			 (STM_TIM234_SMCR_SMS_EXTERNAL_CLOCK << STM_TIM234_SMCR_SMS));
	stm_tim2.dier = 0;
	stm_tim2.sr = 0;
	stm_tim2.psc = 0;
	stm_tim2.cnt = 0;
	stm_tim2.arr = 0xffff;

	/* Start your timers */

	stm_tim2.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
			(0 << STM_TIM234_CR1_ARPE) |
			(STM_TIM234_CR1_CMS_EDGE | STM_TIM234_CR1_CMS) |
			(STM_TIM234_CR1_DIR_UP << STM_TIM234_CR1_DIR) |
			(0 << STM_TIM234_CR1_OPM) |
			(0 << STM_TIM234_CR1_URS) |
			(0 << STM_TIM234_CR1_UDIS) |
			(1 << STM_TIM234_CR1_CEN));

	stm_tim4.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
			(0 << STM_TIM234_CR1_ARPE) |
			(STM_TIM234_CR1_CMS_EDGE | STM_TIM234_CR1_CMS) |
			(STM_TIM234_CR1_DIR_UP << STM_TIM234_CR1_DIR) |
			(0 << STM_TIM234_CR1_OPM) |
			(1 << STM_TIM234_CR1_URS) |
			(0 << STM_TIM234_CR1_UDIS) |
			(1 << STM_TIM234_CR1_CEN));

	ao_cmd_register(&ao_profile_cmds[0]);
}
