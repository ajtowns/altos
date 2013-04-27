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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define HAS_TASK_QUEUE		0

/* Bridge SB17 on the board and use the MCO from the other chip */
#define AO_HSE		8000000
#define AO_HSE_BYPASS		1

/* PLLVCO = 96MHz (so that USB will work) */
#define AO_PLLMUL		12
#define AO_RCC_CFGR_PLLMUL	(STM_RCC_CFGR_PLLMUL_12)

/* SYSCLK = 32MHz */
#define AO_PLLDIV		3
#define AO_RCC_CFGR_PLLDIV	(STM_RCC_CFGR_PLLDIV_3)

/* HCLK = 32MHZ (CPU clock) */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* Run APB1 at HCLK/1 */
#define AO_APB1_PRESCALER	1
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE2_DIV_1

/* Run APB2 at HCLK/1 */
#define AO_APB2_PRESCALER      	1
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_1

#define HAS_USB			1
#define USE_USB_STDIO		0
#define HAS_BEEP		0
#define HAS_TASK		0
#define HAS_ECHO		0
#define HAS_TICK		0

#define PACKET_HAS_SLAVE	0

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOCEN
#define LED_PORT		(&stm_gpiob)
#define LED_PIN_RED		6
#define LED_PIN_GREEN		7
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define AO_TICK_TYPE		uint32_t
#define AO_TICK_SIGNED		int32_t

#define HAS_TASK_INFO		0
#define HAS_VERSION		0

#define AO_BOOT_CHAIN		1
#define AO_BOOT_PIN		1

#define AO_BOOT_APPLICATION_GPIO	stm_gpiod
#define AO_BOOT_APPLICATION_PIN		2
#define AO_BOOT_APPLICATION_VALUE	1
#define AO_BOOT_APPLICATION_MODE	AO_EXTI_MODE_PULL_UP

#endif /* _AO_PINS_H_ */
