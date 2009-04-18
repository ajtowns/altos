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

#define AO_NO_TASK_INDEX	0xff

__xdata struct ao_task * __xdata ao_tasks[AO_NUM_TASKS];
__data uint8_t ao_num_tasks;
__data uint8_t ao_cur_task_index;
__xdata struct ao_task *__data ao_cur_task;

void
ao_add_task(__xdata struct ao_task * task, void (*start)(void))
{
	uint8_t	__xdata *stack;
	if (ao_num_tasks == AO_NUM_TASKS)
		ao_panic(AO_PANIC_NO_TASK);
	ao_tasks[ao_num_tasks++] = task;
	task->task_id = ao_num_tasks;
	/*
	 * Construct a stack frame so that it will 'return'
	 * to the start of the task
	 */
	stack = task->stack;
	
	*stack++ = ((uint16_t) start);
	*stack++ = ((uint16_t) start) >> 8;
	
	/* and the stuff saved by ao_switch */
	*stack++ = 0;		/* acc */
	*stack++ = 0x80;	/* IE */
	*stack++ = 0;		/* DPL */
	*stack++ = 0;		/* DPH */
	*stack++ = 0;		/* B */
	*stack++ = 0;		/* R2 */
	*stack++ = 0;		/* R3 */
	*stack++ = 0;		/* R4 */
	*stack++ = 0;		/* R5 */
	*stack++ = 0;		/* R6 */
	*stack++ = 0;		/* R7 */
	*stack++ = 0;		/* R0 */
	*stack++ = 0;		/* R1 */
	*stack++ = 0;		/* PSW */
	*stack++ = 0;		/* BP */
	task->stack_count = stack - task->stack;
	task->wchan = NULL;
}

/* Task switching function. This must not use any stack variables */
void
ao_yield(void) _naked
{

	/* Save current context */
	_asm
		/* Push ACC first, as when restoring the context it must be restored
		 * last (it is used to set the IE register). */
		push	ACC
		/* Store the IE register then enable interrupts. */
		push	_IEN0
		setb	_EA
		push	DPL
		push	DPH
		push	b
		push	ar2
		push	ar3
		push	ar4
		push	ar5
		push	ar6
		push	ar7
		push	ar0
		push	ar1
		push	PSW
	_endasm;
	PSW = 0;
	_asm
		push	_bp
	_endasm;
	
	if (ao_cur_task_index != AO_NO_TASK_INDEX)
	{
	uint8_t stack_len;
	__data uint8_t *  stack_ptr;
	__xdata uint8_t * save_ptr;
		/* Save the current stack */
		stack_len = SP - (AO_STACK_START - 1);
		ao_cur_task->stack_count = stack_len;
		stack_ptr = (uint8_t __data *) AO_STACK_START;
		save_ptr = (uint8_t __xdata *) ao_cur_task->stack;
		do
			*save_ptr++ = *stack_ptr++;
		while (--stack_len);
	}
	
	/* Empty the stack; might as well let interrupts have the whole thing */
	SP = AO_STACK_START - 1;

	/* Find a task to run. If there isn't any runnable task,
	 * this loop will run forever, which is just fine
	 */
	for (;;) {
		++ao_cur_task_index;
		if (ao_cur_task_index == ao_num_tasks)
			ao_cur_task_index = 0;
		ao_cur_task = ao_tasks[ao_cur_task_index];
		if (ao_cur_task->wchan == NULL)
			break;
	}

	{
		uint8_t stack_len;
		__data uint8_t *  stack_ptr;
		__xdata uint8_t * save_ptr;

		/* Restore the old stack */
		stack_len = ao_cur_task->stack_count;
		SP = AO_STACK_START - 1 + stack_len;
	
		stack_ptr = (uint8_t __data *) AO_STACK_START;
		save_ptr = (uint8_t __xdata *) ao_cur_task->stack;
		do
			*stack_ptr++ = *save_ptr++;
		while (--stack_len);
	}

	_asm
		pop		_bp
		pop		PSW
		pop		ar1
		pop		ar0
		pop		ar7
		pop		ar6
		pop		ar5
		pop		ar4
		pop		ar3
		pop		ar2
		pop		b
		pop		DPH
		pop		DPL
		/* The next byte of the stack is the IE register.  Only the global
		enable bit forms part of the task context.  Pop off the IE then set
		the global enable bit to match that of the stored IE register. */
		pop		ACC
		JB		ACC.7,0098$
		CLR		_EA
		LJMP	0099$
	0098$:
		SETB		_EA
	0099$:
		/* Finally pop off the ACC, which was the first register saved. */
		pop		ACC
		ret
	_endasm;
}

int
ao_sleep(__xdata void *wchan)
{
	__critical {
	ao_cur_task->wchan = wchan;
	}
	ao_yield();
}

int
ao_wakeup(__xdata void *wchan)
{
	uint8_t	i;

	for (i = 0; i < ao_num_tasks; i++)
		if (ao_tasks[i]->wchan == wchan)
			ao_tasks[i]->wchan = NULL;
}

void
ao_start_scheduler(void)
{

	ao_cur_task_index = AO_NO_TASK_INDEX;
	ao_cur_task = NULL;
	ao_yield();
}
