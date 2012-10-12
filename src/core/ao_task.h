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

#ifndef _AO_TASK_H_
#define _AO_TASK_H_

/* An AltOS task */
struct ao_task {
	__xdata void *wchan;		/* current wait channel (NULL if running) */
	uint16_t alarm;			/* abort ao_sleep time */
	ao_arch_task_members		/* any architecture-specific fields */
	uint8_t task_id;		/* unique id */
	__code char *name;		/* task name */
	uint8_t	stack[AO_STACK_SIZE];	/* saved stack */
};

extern __xdata struct ao_task *__data ao_cur_task;

#define AO_NUM_TASKS		16	/* maximum number of tasks */
#define AO_NO_TASK		0	/* no task id */

/*
 ao_task.c
 */

/* Suspend the current task until wchan is awoken.
 * returns:
 *  0 on normal wake
 *  1 on alarm
 */
uint8_t
ao_sleep(__xdata void *wchan);

/* Wake all tasks sleeping on wchan */
void
ao_wakeup(__xdata void *wchan);

/* set an alarm to go off in 'delay' ticks */
void
ao_alarm(uint16_t delay);

/* Clear any pending alarm */
void
ao_clear_alarm(void);

/* Yield the processor to another task */
void
ao_yield(void) ao_arch_naked_declare;

/* Add a task to the run queue */
void
ao_add_task(__xdata struct ao_task * task, void (*start)(void), __code char *name) __reentrant;

/* Terminate the current task */
void
ao_exit(void);

/* Dump task info to console */
void
ao_task_info(void);

/* Start the scheduler. This will not return */
void
ao_start_scheduler(void);

#endif
