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

struct ao_task __xdata blink_0_task;
struct ao_task __xdata blink_1_task;
struct ao_task __xdata wakeup_task;

void delay(int n) __reentrant
{
	uint8_t	j = 0;
	while (--n)
		while (--j)
			ao_yield();
}

static __xdata uint8_t blink_chan;

void
blink_0(void)
{
	for (;;) {
		P1 ^= 1;
		ao_sleep(&blink_chan);
	}
}

void
blink_1(void)
{
	for (;;) {
		P1 ^= 2;
		ao_delay(20);
	}
}

void
wakeup(void)
{
	for (;;) {
		ao_delay(10);
		ao_wakeup(&blink_chan);
	}
}

void
main(void)
{
	CLKCON = 0;
	while (!(SLEEP & SLEEP_XOSC_STB))
		;

	/* Set p1_1 and p1_0 to output */
	P1DIR = 0x03;
	
	ao_add_task(&blink_0_task, blink_0);
	ao_add_task(&blink_1_task, blink_1);
	ao_add_task(&wakeup_task, wakeup);
	ao_start_scheduler();
}
