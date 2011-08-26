/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef AVR_DEMO
#define TEENSY 1
#endif

#if TEENSY
#define F_CPU 16000000UL	// 16 MHz
#else
#define F_CPU  8000000UL	// 8 MHz
#endif

/*
 * AVR definitions and code fragments for AltOS
 */

#define AO_STACK_SIZE	128

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

#define ao_arch_reboot()	/* XXX */

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

#undef putchar
#undef getchar
#define putchar(c)	ao_putchar(c)
#define getchar		ao_getchar

extern void putchar(char c);
extern char getchar(void);

extern int ao_serial_number;

#define ao_arch_init_stack(task, start) do {			\
	uint8_t		*sp = task->stack + AO_STACK_SIZE - 1;	\
	uint16_t	a = (uint16_t) start; 			\
	int		i;					\
								\
	/* Return address */					\
	PUSH8(sp, a);						\
	PUSH8(sp, (a >> 8));					\
								\
	/* Clear register values */				\
	i = 32;							\
	while (i--)						\
		PUSH8(sp, 0);					\
								\
	/* SREG with interrupts enabled */			\
	PUSH8(sp, 0x80);					\
	task->sp = sp;						\
} while (0);
	
#define ao_arch_save_context() do {			\
	asm("push r31" "\n\t" "push r30"); \
	asm("push r29" "\n\t" "push r28" "\n\t" "push r27" "\n\t" "push r26" "\n\t" "push r25"); \
	asm("push r24" "\n\t" "push r23" "\n\t" "push r22" "\n\t" "push r21" "\n\t" "push r20"); \
	asm("push r19" "\n\t" "push r18" "\n\t" "push r17" "\n\t" "push r16" "\n\t" "push r15"); \
	asm("push r14" "\n\t" "push r13" "\n\t" "push r12" "\n\t" "push r11" "\n\t" "push r10"); \
	asm("push r9" "\n\t" "push r8" "\n\t" "push r7" "\n\t" "push r6" "\n\t" "push r5"); \
	asm("push r4" "\n\t" "push r3" "\n\t" "push r2" "\n\t" "push r1" "\n\t" "push r0"); \
	asm("in r0, __SREG__" "\n\t" "push r0"); \
	sei(); \
	} while (0)

#endif /* _AO_ARCH_H_ */
