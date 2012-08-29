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

#include <ao.h>

#define AO_NO_TASK_INDEX	0xff

__xdata struct ao_task * __xdata ao_tasks[AO_NUM_TASKS];
__data uint8_t ao_num_tasks;
__data uint8_t ao_cur_task_index;
__xdata struct ao_task *__data ao_cur_task;

#ifdef ao_arch_task_globals
ao_arch_task_globals
#endif

#define AO_CHECK_STACK	0

#if AO_CHECK_STACK
static uint8_t	in_yield;

static inline void ao_check_stack(void) {
	uint8_t	q;
	if (!in_yield && ao_cur_task && &q < &ao_cur_task->stack[0])
		ao_panic(AO_PANIC_STACK);
}
#else
#define ao_check_stack()
#endif

void
ao_add_task(__xdata struct ao_task * task, void (*start)(void), __code char *name) __reentrant
{
	uint8_t task_id;
	uint8_t t;
	if (ao_num_tasks == AO_NUM_TASKS)
		ao_panic(AO_PANIC_NO_TASK);
	for (task_id = 1; task_id != 0; task_id++) {
		for (t = 0; t < ao_num_tasks; t++)
			if (ao_tasks[t]->task_id == task_id)
				break;
		if (t == ao_num_tasks)
			break;
	}
	ao_tasks[ao_num_tasks++] = task;
	task->task_id = task_id;
	task->name = name;
	task->wchan = NULL;
	/*
	 * Construct a stack frame so that it will 'return'
	 * to the start of the task
	 */
	ao_arch_init_stack(task, start);
}

/* Task switching function. This must not use any stack variables */
void
ao_yield(void) ao_arch_naked_define
{
	ao_arch_save_regs();

	if (ao_cur_task_index == AO_NO_TASK_INDEX)
		ao_cur_task_index = ao_num_tasks-1;
	else
	{
		ao_arch_save_stack();
	}

	ao_arch_isr_stack();

#if AO_CHECK_STACK
	in_yield = 1;
#endif
	/* Find a task to run. If there isn't any runnable task,
	 * this loop will run forever, which is just fine
	 */
	{
		__pdata uint8_t	ao_last_task_index = ao_cur_task_index;
		for (;;) {
			++ao_cur_task_index;
			if (ao_cur_task_index == ao_num_tasks)
				ao_cur_task_index = 0;

			ao_cur_task = ao_tasks[ao_cur_task_index];

			/* Check for ready task */
			if (ao_cur_task->wchan == NULL)
				break;

			/* Check if the alarm is set for a time which has passed */
			if (ao_cur_task->alarm &&
			    (int16_t) (ao_time() - ao_cur_task->alarm) >= 0)
				break;

			/* Enter lower power mode when there isn't anything to do */
			if (ao_cur_task_index == ao_last_task_index)
				ao_arch_cpu_idle();
		}
	}
#if AO_CHECK_STACK
	cli();
	in_yield = 0;
#endif
	ao_arch_restore_stack();
}

uint8_t
ao_sleep(__xdata void *wchan)
{
	ao_cur_task->wchan = wchan;
	ao_yield();
	if (ao_cur_task->wchan) {
		ao_cur_task->wchan = NULL;
		ao_cur_task->alarm = 0;
		return 1;
	}
	return 0;
}

void
ao_wakeup(__xdata void *wchan)
{
	uint8_t	i;

	ao_check_stack();
	for (i = 0; i < ao_num_tasks; i++)
		if (ao_tasks[i]->wchan == wchan)
			ao_tasks[i]->wchan = NULL;
}

void
ao_alarm(uint16_t delay)
{
	/* Make sure we sleep *at least* delay ticks, which means adding
	 * one to account for the fact that we may be close to the next tick
	 */
	if (!(ao_cur_task->alarm = ao_time() + delay + 1))
		ao_cur_task->alarm = 1;
}

void
ao_clear_alarm(void)
{
	ao_cur_task->alarm = 0;
}

static __xdata uint8_t ao_forever;

void
ao_delay(uint16_t ticks)
{
	ao_alarm(ticks);
	ao_sleep(&ao_forever);
	ao_clear_alarm();
}

void
ao_exit(void)
{
	ao_arch_critical(
		uint8_t	i;
		ao_num_tasks--;
		for (i = ao_cur_task_index; i < ao_num_tasks; i++)
			ao_tasks[i] = ao_tasks[i+1];
		ao_cur_task_index = AO_NO_TASK_INDEX;
		ao_yield();
		);
	/* we'll never get back here */
}

void
ao_task_info(void)
{
	uint8_t		i;
	__xdata struct ao_task *task;

	for (i = 0; i < ao_num_tasks; i++) {
		task = ao_tasks[i];
		printf("%12s: wchan %04x\n",
		       task->name,
		       (int) task->wchan);
	}
}

void
ao_start_scheduler(void)
{
	ao_cur_task_index = AO_NO_TASK_INDEX;
	ao_cur_task = NULL;
	ao_yield();
}
