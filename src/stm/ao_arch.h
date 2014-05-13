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

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include <stdio.h>
#include <stm32l.h>

/*
 * STM32L definitions and code fragments for AltOS
 */

#define AO_STACK_SIZE	512

#define AO_LED_TYPE	uint16_t

#ifndef AO_TICK_TYPE
#define AO_TICK_TYPE	uint16_t
#define AO_TICK_SIGNED	int16_t
#endif

#define AO_PORT_TYPE	uint16_t

/* Various definitions to make GCC look more like SDCC */

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define
#define __pdata
#define __data
#define __xdata
#define __code const
#define __reentrant
#define __interrupt(n)
#define __at(n)

#define ao_arch_reboot() \
	(stm_scb.aircr = ((STM_SCB_AIRCR_VECTKEY_KEY << STM_SCB_AIRCR_VECTKEY) | \
			  (1 << STM_SCB_AIRCR_SYSRESETREQ)))

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

#undef putchar
#undef getchar
#define putchar(c)	ao_putchar(c)
#define getchar		ao_getchar

extern void putchar(char c);
extern char getchar(void);
extern void ao_avr_stdio_init(void);


/*
 * ao_romconfig.c
 */

#define AO_ROMCONFIG_VERSION	2

#define AO_ROMCONFIG_SYMBOL(a) __attribute__((section(".romconfig"))) const

extern const uint16_t ao_romconfig_version;
extern const uint16_t ao_romconfig_check;
extern const uint16_t ao_serial_number;
extern const uint32_t ao_radio_cal;

#define ao_arch_task_members\
	uint32_t *sp;			/* saved stack pointer */

#define ao_arch_block_interrupts()	asm("cpsid i")
#define ao_arch_release_interrupts()	asm("cpsie i")


/*
 * For now, we're running at a weird frequency
 */

#ifndef AO_HSE
#error High speed frequency undefined
#endif

#if AO_HSE
#define AO_PLLSRC	AO_HSE
#else
#define AO_PLLSRC	STM_HSI_FREQ
#endif

#define AO_PLLVCO	(AO_PLLSRC * AO_PLLMUL)
#define AO_SYSCLK	(AO_PLLVCO / AO_PLLDIV)
#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)
#define AO_PCLK1	(AO_HCLK / AO_APB1_PRESCALER)
#define AO_PCLK2	(AO_HCLK / AO_APB2_PRESCALER)
#define AO_SYSTICK	(AO_HCLK / 8)

#if AO_APB1_PRESCALER == 1
#define AO_TIM23467_CLK		AO_PCLK1
#else
#define AO_TIM23467_CLK		(2 * AO_PCLK1)
#endif

#if AO_APB2_PRESCALER == 1
#define AO_TIM91011_CLK		AO_PCLK2
#else
#define AO_TIM91011_CLK		(2 * AO_PCLK2)
#endif

#define AO_STM_NVIC_HIGH_PRIORITY	4
#define AO_STM_NVIC_CLOCK_PRIORITY	6
#define AO_STM_NVIC_MED_PRIORITY	8
#define AO_STM_NVIC_LOW_PRIORITY	10

void ao_lcd_stm_init(void);

void ao_lcd_font_init(void);

void ao_lcd_font_string(char *s);

extern const uint32_t	ao_radio_cal;

void
ao_adc_init();

/* ADC maximum reported value */
#define AO_ADC_MAX			4095

#define AO_BOOT_APPLICATION_BASE	((uint32_t *) 0x08001000)
#define AO_BOOT_APPLICATION_BOUND	((uint32_t *) (0x08000000 + stm_flash_size()))
#define AO_BOOT_LOADER_BASE		((uint32_t *) 0x08000000)
#define HAS_BOOT_LOADER			1

#endif /* _AO_ARCH_H_ */


