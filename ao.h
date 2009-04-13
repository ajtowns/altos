/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License
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

#ifndef _AO_H_
#define _AO_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cc1111.h"

#define DATA_TO_XDATA(a)	((void __xdata *) ((uint8_t) (a) | 0xff00))

#define AO_STACK_START	0x27
#define AO_STACK_END	0xfe
#define AO_STACK_SIZE	(AO_STACK_END - AO_STACK_START + 1)

struct ao_task {
	__xdata void	*wchan;
	uint8_t	stack[AO_STACK_SIZE];
	uint8_t	stack_count;
};

#define AO_NUM_TASKS	10

#define AO_ERROR_NO_TASK	1

#define ao_interrupt_disable()	(EA = 0)
#define ao_interrupt_enable()	(EA = 1)

/* ao_task.c */
int ao_sleep(__xdata void *wchan);
int ao_wakeup(__xdata void *wchan);
void ao_add_task(__xdata struct ao_task * task, void (*start)(void));
void ao_start_scheduler(void);
void ao_yield(void) _naked;
void ao_panic(uint8_t reason);

/* ao_timer.c */

volatile __data uint16_t ao_time;

void ao_timer_isr(void) interrupt 9;
void ao_timer_init(void);
uint16_t ao_time_atomic(void);
void ao_delay(uint16_t ticks);

/* ao_adc.c */

#define ADC_RING	32

struct ao_adc {
	uint16_t	tick;
	int16_t		accel;
	int16_t		pres;
	int16_t		temp;
	int16_t		v_batt;
	int16_t		sense_d;
	int16_t		sense_m;
};

extern __xdata struct ao_adc	ao_adc_ring[ADC_RING];
extern __data uint8_t		ao_adc_head;

void ao_adc_isr(void) interrupt 1;
void ao_adc_init(void);
void ao_adc_poll(void);
void ao_adc_get(__xdata struct ao_adc *packet);

#endif /* _AO_H_ */
