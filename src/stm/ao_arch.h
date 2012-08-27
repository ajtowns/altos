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

/* Various definitions to make GCC look more like SDCC */

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define
#define __pdata
#define __data
#define __xdata
#define __code const
#define __reentrant
#define __critical
#define __interrupt(n)
#define __at(n)

#define CORTEX_M3_AIRCR		((uint32_t *) 0xe000ed0c)

#define ao_arch_reboot()	(*((uint32_t *) 0xe000ed0c) = 0x05fa0004)

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

#define ARM_PUSH32(stack, val)	(*(--(stack)) = (val))

#define ao_arch_task_members\
	uint32_t *sp;			/* saved stack pointer */

#define cli()	asm("cpsid i")
#define sei()	asm("cpsie i")

#define ao_arch_init_stack(task, start) do {				\
		uint32_t	*sp = (uint32_t *) (task->stack + AO_STACK_SIZE); \
		uint32_t	a = (uint32_t) start; 			\
		int		i;					\
									\
		/* Return address (goes into LR) */			\
		ARM_PUSH32(sp, a);					\
									\
		/* Clear register values r0-r12 */			\
		i = 13;							\
		while (i--)						\
			ARM_PUSH32(sp, 0);				\
									\
		/* APSR */						\
		ARM_PUSH32(sp, 0);					\
									\
		/* PRIMASK with interrupts enabled */			\
		ARM_PUSH32(sp, 0);					\
									\
		task->sp = sp;						\
} while (0);
	
#define ao_arch_save_regs() 	do {					\
		/* Save general registers */				\
		asm("push {r0-r12,lr}\n");				\
									\
		/* Save APSR */						\
		asm("mrs r0,apsr");					\
		asm("push {r0}");					\
									\
		/* Save PRIMASK */ 					\
		asm("mrs r0,primask");					\
		asm("push {r0}");					\
									\
		/* Enable interrupts */					\
		sei();							\
	} while (0)

#define ao_arch_save_stack() do {					\
		uint32_t	*sp;					\
		asm("mov %0,sp" : "=&r" (sp) );				\
		ao_cur_task->sp = (sp);					\
		if ((uint8_t *) sp < &ao_cur_task->stack[0])		\
			ao_panic (AO_PANIC_STACK);			\
	} while (0)

#if 0
#define ao_arch_isr_stack() do {				\
		uint32_t	*sp = (uint32_t *) 0x20004000;	\
		asm("mov %0,sp" : "=&r" (sp) );			\
	} while (0)
#else
#define ao_arch_isr_stack()
#endif


#define ao_arch_cpu_idle() do {			\
		asm("wfi");		\
	} while (0)

#define ao_arch_restore_stack() do { \
		uint32_t	sp;					\
		sp = (uint32_t) ao_cur_task->sp;			\
									\
		/* Switch stacks */					\
		asm("mov sp, %0" : : "r" (sp) );			\
									\
		/* Restore PRIMASK */					\
		asm("pop {r0}");					\
		asm("msr primask,r0");					\
									\
		/* Restore APSR */					\
		asm("pop {r0}");					\
		asm("msr apsr,r0");					\
									\
		/* Restore general registers */				\
		asm("pop {r0-r12,lr}\n");				\
									\
		/* Return to calling function */			\
		asm("bx lr");						\
	} while(0)

#define ao_arch_critical(b) do { cli(); do { b } while (0); sei(); } while (0)

/*
 * For now, we're running at a weird frequency
 */

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

char
ao_serial1_getchar(void);

void
ao_serial1_putchar(char c);

char
ao_serial1_pollchar(void);

void
ao_serial1_set_speed(uint8_t speed);

char
ao_serial2_getchar(void);

void
ao_serial2_putchar(char c);

char
ao_serial2_pollchar(void);

void
ao_serial2_set_speed(uint8_t speed);

char
ao_serial3_getchar(void);

void
ao_serial3_putchar(char c);

char
ao_serial3_pollchar(void);

void
ao_serial3_set_speed(uint8_t speed);

extern const uint32_t	ao_radio_cal;

void
ao_adc_init();

#endif /* _AO_ARCH_H_ */

