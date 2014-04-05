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

/*
 * CC1111 definitions and code fragments for AltOS
 */

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include "cc1111.h"

/* Convert a __data pointer into an __xdata pointer */
#define DATA_TO_XDATA(a)	((void __xdata *) ((uint8_t) (a) | 0xff00))

/* Code and xdata use the same address space */
#define CODE_TO_XDATA(a)	((__xdata void *) ((uint16_t) (a)))

/* Pdata lives at the start of xdata */
#define PDATA_TO_XDATA(a)	((void __xdata *) ((uint8_t) (a) | 0xf000))

/* Stack runs from above the allocated __data space to 0xfe, which avoids
 * writing to 0xff as that triggers the stack overflow indicator
 */
#define AO_STACK_START	0x90
#define AO_STACK_END	0xfe
#define AO_STACK_SIZE	(AO_STACK_END - AO_STACK_START + 1)

#define ao_arch_reboot() do {					\
	WDCTL = WDCTL_EN | WDCTL_MODE_WATCHDOG | WDCTL_INT_64;	\
	ao_delay(AO_SEC_TO_TICKS(2));				\
	} while (0)
	
#define ao_arch_nop()	__asm nop __endasm
#define ao_arch_interrupt(n)	__interrupt n

#define ao_arch_naked_declare	__naked
#define ao_arch_naked_define	__naked

/* CC1111-specific drivers */

/*
 * ao_romconfig.c
 */

#define AO_ROMCONFIG_VERSION	2

#define AO_ROMCONFIG_SYMBOL(a) __code __at(a)

extern AO_ROMCONFIG_SYMBOL(0x00a0) uint16_t ao_romconfig_version;
extern AO_ROMCONFIG_SYMBOL(0x00a2) uint16_t ao_romconfig_check;
extern AO_ROMCONFIG_SYMBOL(0x00a4) uint16_t ao_serial_number;
extern AO_ROMCONFIG_SYMBOL(0x00a6) uint32_t ao_radio_cal;

#ifndef HAS_USB
#error Please define HAS_USB
#endif

#define ao_arch_task_members\
	uint8_t	stack_count;		/* amount of saved stack */

/* Initialize stack */
#define ao_arch_init_stack(task, start) {			\
	uint8_t __xdata *stack = task->stack;			\
	uint8_t	t;						\
	*stack++ = ((uint16_t) start);		/* 0 */		\
	*stack++ = ((uint16_t) start) >> 8;	/* 1 */		\
								\
	/* and the stuff saved by ao_switch */			\
	*stack++ = 0;				/* 2 acc */   	\
	*stack++ = 0x80;			/* 3 IE */ 	\
								\
	/*  4 DPL 						\
	 *  5 DPH 						\
	 *  6 B 						\
	 *  7 R2 						\
	 *  8 R3 						\
	 *  9 R4 						\
	 * 10 R5 						\
	 * 11 R6 						\
	 * 12 R7 						\
	 * 13 R0 						\
	 * 14 R1 						\
	 * 15 PSW 						\
	 * 16 BP 						\
	 */ 							\
	for (t = 0; t < 13; t++) 				\
		*stack++ = 0; 					\
	task->stack_count = 17;					\
	}


  
/* Save current context */

#define ao_arch_save_regs()						\
	__asm								\
	/* Push ACC first, as when restoring the context it must be restored \
	 * last (it is used to set the IE register). */			\
	push	ACC							\
	push	_IEN0							\
	push	DPL							\
	push	DPH							\
	push	b							\
	push	ar2							\
	push	ar3							\
	push	ar4							\
	push	ar5							\
	push	ar6							\
	push	ar7							\
	push	ar0							\
	push	ar1							\
	push	PSW							\
	__endasm;							\
	PSW = 0;							\
	__asm								\
	push	_bp							\
	__endasm

#define ao_arch_save_stack() { 						\
		uint8_t stack_len;					\
		__data uint8_t *stack_ptr;				\
		__xdata uint8_t *save_ptr;				\
		/* Save the current stack */				\
		stack_len = SP - (AO_STACK_START - 1);			\
		ao_cur_task->stack_count = stack_len;			\
		stack_ptr = (uint8_t __data *) AO_STACK_START;		\
		save_ptr = (uint8_t __xdata *) ao_cur_task->stack;	\
		do							\
			*save_ptr++ = *stack_ptr++;			\
		while (--stack_len);					\
	}

/* Empty the stack; might as well let interrupts have the whole thing */
#define ao_arch_isr_stack()		(SP = AO_STACK_START - 1)

#define ao_arch_block_interrupts()	__asm clr _EA __endasm
#define ao_arch_release_interrupts()	__asm setb _EA __endasm

/* Idle the CPU, waking when an interrupt occurs */
#define ao_arch_wait_interrupt() do {		\
		ao_arch_release_interrupts();	\
		(PCON = PCON_IDLE);		\
		ao_arch_block_interrupts();	\
	} while (0)

#define ao_arch_restore_stack() {					\
		uint8_t stack_len;					\
		__data uint8_t *stack_ptr;				\
		__xdata uint8_t *save_ptr;				\
									\
		/* Restore the old stack */				\
		stack_len = ao_cur_task->stack_count;			\
		SP = AO_STACK_START - 1 + stack_len;			\
									\
		stack_ptr = (uint8_t __data *) AO_STACK_START;		\
		save_ptr = (uint8_t __xdata *) ao_cur_task->stack;	\
		do							\
			*stack_ptr++ = *save_ptr++;			\
		while (--stack_len);					\
									\
		__asm							\
		pop		_bp					\
		pop		PSW					\
		pop		ar1					\
		pop		ar0					\
		pop		ar7					\
		pop		ar6					\
		pop		ar5					\
		pop		ar4					\
		pop		ar3					\
		pop		ar2					\
		pop		b					\
		pop		DPH					\
		pop		DPL					\
		/* The next byte of the stack is the IE register.  Only the global \
		   enable bit forms part of the task context.  Pop off the IE then set \
		   the global enable bit to match that of the stored IE register. */ \
		pop		ACC					\
		JB		ACC.7,0098$				\
		CLR		_EA					\
		LJMP	0099$						\
		0098$:							\
			SETB		_EA				\
		0099$:							\
		/* Finally restore ACC, which was the first register saved. */ \
		pop		ACC					\
		ret							\
		__endasm;						\
}

#define ao_arch_critical(b) __critical { b }

#define AO_DATA_RING	32

/* ao_button.c */
#ifdef HAS_BUTTON
void
ao_p0_isr(void) ao_arch_interrupt(13);

void
ao_p1_isr(void) ao_arch_interrupt(15);

void
ao_p2_isr(void);

#define HAS_P2_ISR	1

#endif

void
ao_button_init(void);

char
ao_button_get(void) __critical;

void
ao_button_clear(void) __critical;

/* ao_string.c */

void
_ao_xmemcpy(__xdata void *dst, __xdata void *src, uint16_t count);

#define ao_xmemcpy(d,s,c) _ao_xmemcpy(d,s,c)

void
_ao_xmemset(__xdata void *dst, uint8_t value, uint16_t count);

#define ao_xmemset(d,v,c) _ao_xmemset(d,v,c)

int8_t
_ao_xmemcmp(__xdata void *a, __xdata void *b, uint16_t count);

#define ao_xmemcmp(d,s,c) _ao_xmemcmp((d), (s), (c))

struct ao_serial_speed {
	uint8_t	baud;
	uint8_t	gcr;
};

extern const __code struct ao_serial_speed ao_serial_speeds[];

/*
 * ao_dma.c
 */

/* Allocate a DMA channel. the 'done' parameter will be set when the
 * dma is finished and will be used to wakeup any waiters
 */

uint8_t
ao_dma_alloc(__xdata uint8_t * done);

/* Setup a DMA channel */
void
ao_dma_set_transfer(uint8_t id,
		    void __xdata *srcaddr,
		    void __xdata *dstaddr,
		    uint16_t count,
		    uint8_t cfg0,
		    uint8_t cfg1);

/* Start a DMA channel */
void
ao_dma_start(uint8_t id);

/* Manually trigger a DMA channel */
void
ao_dma_trigger(uint8_t id);

/* Abort a running DMA transfer */
void
ao_dma_abort(uint8_t id);

/* DMA interrupt routine */
void
ao_dma_isr(void) ao_arch_interrupt(8);

/* ao_adc.c */

#if HAS_ADC
/* The A/D interrupt handler */
void
ao_adc_isr(void) ao_arch_interrupt(1);
#endif

#if HAS_USB
/* USB interrupt handler */
void
ao_usb_isr(void) ao_arch_interrupt(6);
#endif

#if HAS_SERIAL_0
void
ao_serial0_rx_isr(void) ao_arch_interrupt(2);

void
ao_serial0_tx_isr(void) ao_arch_interrupt(7);
#endif

#if HAS_SERIAL_1
void
ao_serial1_rx_isr(void) ao_arch_interrupt(3);

void
ao_serial1_tx_isr(void) ao_arch_interrupt(14);
#endif

#if HAS_EXTI_0
void
ao_p0_isr(void) __interrupt(13);
#endif

#define AO_ADC_MAX	32767

#endif /* _AO_ARCH_H_ */
