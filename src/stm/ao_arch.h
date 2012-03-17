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

#define AO_STACK_SIZE	256

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

#define ao_arch_reboot()	/* XXX */

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

#undef putchar
#undef getchar
#define putchar(c)	ao_putchar(c)
#define getchar		ao_getchar

extern void putchar(char c);
extern char getchar(void);
extern void ao_avr_stdio_init(void);

extern const uint16_t ao_serial_number;

#define ARM_PUSH32(stack, val)	(*(--(stack)) = (val))

#define ao_arch_task_members\
	uint32_t *sp;			/* saved stack pointer */

#define cli()	asm("cpsid i")
#define sei()	asm("cpsie i")

#define ao_arch_init_stack(task, start) do {				\
		uint32_t	*sp = (uint32_t *) (task->stack + AO_STACK_SIZE); \
		uint16_t	a = (uint16_t) start; 			\
		int		i;					\
									\
		/* Return address */					\
		ARM_PUSH32(sp, a);					\
									\
		/* Invalid link register */				\
		ARM_PUSH32(sp, 0xffffffff);				\
									\
		/* Clear register values  */				\
		i = 13;							\
		while (i--)						\
			ARM_PUSH32(sp, 0);				\
									\
		/* PSR with interrupts enabled */			\
		ARM_PUSH32(sp, 0x01000000);				\
		task->sp = sp;						\
} while (0);
	
#define ao_arch_save_regs() do {					\
		asm("push {r0-r12,lr}\n");				\
		cli();							\
		asm("mrs r0,psr" "\n\t" "push {r0}");			\
		sei();							\
	} while (0)

#define ao_arch_save_stack() do {					\
		uint32_t	sp;					\
		asm("mov %0,sp" : "=&r" (sp) );				\
		ao_cur_task->sp = (uint32_t *) (sp);			\
	} while (0)

#define ao_arch_isr_stack()	/* nothing */

#define ao_arch_cpu_idle() do {			\
		asm("wfi");			\
	} while (0)

#define ao_arch_restore_stack() do { \
		uint32_t	sp;					\
		sp = (uint32_t) ao_cur_task->sp;			\
		cli();							\
		asm("mov sp, %0" : : "r" (sp) );			\
		asm("pop {r0}" "\n\t" "msr psr,r0");			\
		asm("pop {r0-r12,lr}\n");				\
		asm("bx lr");						\
	} while(0)

#define ao_arch_critical(b) do { cli(); do { b } while (0); sei(); } while (0)

#define AO_ARM_NUM_ADC	12

struct ao_adc {
	uint16_t	tick;			/* tick when the sample was read */
	uint16_t	adc[AO_ARM_NUM_ADC];	/* samples */
};


#endif /* _AO_ARCH_H_ */

