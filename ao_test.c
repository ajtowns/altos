/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

struct ao_task __xdata blink_task;

void delay(int n) __reentrant
{
	while (n--)
		ao_yield();
}

void
blink(void)
{
	for (;;) {
		P1 ^= 2;
		delay(100);
	}
}

void
main(void)
{
	CLKCON = 0;
	/* Set p1_1 to output */
	P1DIR = 0x02;
	
	ao_add_task(&blink_task, blink);
	ao_start_scheduler();
}
