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
#if HAS_TASK_QUEUE
#include <ao_list.h>
#endif

#ifndef HAS_TASK_INFO
#define HAS_TASK_INFO 1
#endif

/* An AltOS task */
struct ao_task {
	__xdata void *wchan;		/* current wait channel (NULL if running) */
	uint16_t alarm;			/* abort ao_sleep time */
	ao_arch_task_members		/* any architecture-specific fields */
	uint8_t task_id;		/* unique id */
	__code char *name;		/* task name */
#if HAS_TASK_QUEUE
	struct ao_list	queue;
	struct ao_list	alarm_queue;
#endif
	uint8_t	stack[AO_STACK_SIZE];	/* saved stack */
#if HAS_SAMPLE_PROFILE
	uint32_t ticks;
	uint32_t yields;
	uint16_t start;
	uint16_t max_run;
#endif
};

#ifndef AO_NUM_TASKS
#define AO_NUM_TASKS		16	/* maximum number of tasks */
#endif

#define AO_NO_TASK		0	/* no task id */

extern __xdata struct ao_task * __xdata ao_tasks[AO_NUM_TASKS];
extern __data uint8_t ao_num_tasks;
extern __xdata struct ao_task *__data ao_cur_task;
extern __data uint8_t ao_task_minimize_latency;	/* Reduce IRQ latency */

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
ao_wakeup(__xdata void *wchan) __reentrant;

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

#if HAS_TASK_QUEUE
/* Called on timer interrupt to check alarms */
extern uint16_t	ao_task_alarm_tick;
void
ao_task_check_alarm(uint16_t tick);
#endif

/* Terminate the current task */
void
ao_exit(void);

/* Dump task info to console */
void
ao_task_info(void);

/* Start the scheduler. This will not return */
void
ao_start_scheduler(void);

#if HAS_TASK_QUEUE
void
ao_task_init(void);
#else
#define ao_task_init()
#endif

#endif
