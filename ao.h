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

#ifndef _AO_H_
#define _AO_H_

#include <stdint.h>
#include <stdio.h>
#include "cc1111.h"

#define AO_STACK_START	0x11
#define AO_STACK_END	0xfe
#define AO_STACK_SIZE	(AO_STACK_END - AO_STACK_START + 1)

struct ao_task {
	__xdata void	*wchan;
	uint8_t	stack[AO_STACK_SIZE];
	uint8_t	stack_count;
};

#define AO_NUM_TASKS	10

#define AO_ERROR_NO_TASK	1

int ao_sleep(__xdata void *wchan);
int ao_wakeup(__xdata void *wchan);
void ao_add_task(__xdata struct ao_task * task, void (*start)(void));
void ao_start_scheduler(void);
void ao_yield(void) _naked;
void ao_panic(uint8_t reason);

#endif /* _AO_H_ */
