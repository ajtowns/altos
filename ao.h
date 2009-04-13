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

#define AO_STACK_START	0x32
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

#define AO_MS_TO_TICKS(ms)	((ms) / 10)
#define AO_SEC_TO_TICKS(s)	((s) * 100)

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

extern volatile __xdata struct ao_adc	ao_adc_ring[ADC_RING];
extern volatile __data uint8_t		ao_adc_head;

void ao_adc_isr(void) interrupt 1;
void ao_adc_init(void);
void ao_adc_poll(void);
void ao_adc_get(__xdata struct ao_adc *packet);

/* ao_beep.c */

#define AO_BEEP_LOW	150
#define AO_BEEP_MID	94
#define AO_BEEP_HIGH	75
#define AO_BEEP_OFF	0

void
ao_beep_init(void);

void
ao_beep(uint8_t beep);

void
ao_beep_for(uint8_t beep, uint16_t ticks);

/* ao_led.c */

#define AO_LED_NONE	0
#define AO_LED_GREEN	1
#define AO_LED_RED	2

void
ao_led_init(void);

void
ao_led_on(uint8_t colors);

void
ao_led_off(uint8_t colors);

void
ao_led_set(uint8_t colors);

void
ao_led_for(uint8_t colors, uint16_t ticks);

/* ao_usb.c */

void
ao_usb_isr(void) interrupt 6;

void
ao_usb_flush(void);

void
ao_usb_putchar(uint8_t c);

uint8_t
ao_usb_getchar(void);

void
ao_usb_init(void);

#endif /* _AO_H_ */
