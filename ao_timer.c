/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

static volatile __data uint16_t ao_tick_count;

uint16_t ao_time(void)
{
	__data uint16_t ret;
	__critical {
		ret = ao_tick_count;
	}
	return ret;
}

void
ao_delay(uint16_t ticks)
{
	uint16_t until = ao_time() + ticks;

	while ((int16_t) (until - ao_time()) > 0)
		ao_sleep(DATA_TO_XDATA(&ao_tick_count));
}

#define T1_CLOCK_DIVISOR	8	/* 24e6/8 = 3e6 */
#define T1_SAMPLE_TIME		30000	/* 3e6/30000 = 100 */

__data uint8_t	ao_adc_interval = 1;
__data uint8_t	ao_adc_count;

void ao_timer_isr(void) interrupt 9
{
	++ao_tick_count;
	if (++ao_adc_count >= ao_adc_interval) {
		ao_adc_count = 0;
		ao_adc_poll();
	}
	ao_wakeup(DATA_TO_XDATA(&ao_tick_count));
}

void
ao_timer_init(void)
{
	/* NOTE:  This uses a timer only present on cc1111 architecture. */

	/* disable timer 1 */
	T1CTL = 0;

	/* set the sample rate */
	T1CC0H = T1_SAMPLE_TIME >> 8;
	T1CC0L = T1_SAMPLE_TIME;

	T1CCTL0 = T1CCTL_MODE_COMPARE;
	T1CCTL1 = 0;
	T1CCTL2 = 0;

	/* clear timer value */
	T1CNTL = 0;

	/* enable overflow interrupt */
	OVFIM = 1;
	/* enable timer 1 interrupt */
	T1IE = 1;

	/* enable timer 1 in module mode, dividing by 8 */
	T1CTL = T1CTL_MODE_MODULO | T1CTL_DIV_8;
}

