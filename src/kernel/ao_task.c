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
#include <ao_task.h>
#if HAS_SAMPLE_PROFILE
#include <ao_sample_profile.h>
#endif
#if HAS_STACK_GUARD
#include <ao_mpu.h>
#endif

#define DEBUG	0

#define AO_NO_TASK_INDEX	0xff

__xdata struct ao_task * __xdata ao_tasks[AO_NUM_TASKS];
__data uint8_t ao_num_tasks;
__xdata struct ao_task *__data ao_cur_task;

#if !HAS_TASK_QUEUE
static __data uint8_t ao_cur_task_index;
#endif

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

#if HAS_TASK_QUEUE

#define SLEEP_HASH_SIZE	17

static struct ao_list	run_queue;
static struct ao_list	alarm_queue;
static struct ao_list	sleep_queue[SLEEP_HASH_SIZE];

static void
ao_task_to_run_queue(struct ao_task *task)
{
	ao_list_del(&task->queue);
	ao_list_append(&task->queue, &run_queue);
}

static struct ao_list *
ao_task_sleep_queue(void *wchan)
{
	return &sleep_queue[(uintptr_t) wchan % SLEEP_HASH_SIZE];
}

static void
ao_task_to_sleep_queue(struct ao_task *task, void *wchan)
{
	ao_list_del(&task->queue);
	ao_list_append(&task->queue, ao_task_sleep_queue(wchan));
}

#if DEBUG
static void
ao_task_validate_alarm_queue(void)
{
	struct ao_task	*alarm, *prev = NULL;
	int		i;

	if (ao_list_is_empty(&alarm_queue))
		return;
	ao_list_for_each_entry(alarm, &alarm_queue, struct ao_task, alarm_queue) {
		if (prev) {
			if ((int16_t) (alarm->alarm - prev->alarm) < 0) {
				ao_panic(1);
			}
		}
		prev = alarm;
	}
	for (i = 0; i < ao_num_tasks; i++) {
		alarm = ao_tasks[i];
		if (alarm->alarm) {
			if (ao_list_is_empty(&alarm->alarm_queue))
				ao_panic(2);
		} else {
			if (!ao_list_is_empty(&alarm->alarm_queue))
				ao_panic(3);
		}
	}
	if (ao_task_alarm_tick != ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm)
		ao_panic(4);
}
#else
#define ao_task_validate_alarm_queue()
#endif

uint16_t	ao_task_alarm_tick;

static void
ao_task_to_alarm_queue(struct ao_task *task)
{
	struct ao_task	*alarm;
	ao_list_for_each_entry(alarm, &alarm_queue, struct ao_task, alarm_queue) {
		if ((int16_t) (alarm->alarm - task->alarm) >= 0) {
			ao_list_insert(&task->alarm_queue, alarm->alarm_queue.prev);
			ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
			ao_task_validate_alarm_queue();
			return;
		}
	}
	ao_list_append(&task->alarm_queue, &alarm_queue);
	ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
	ao_task_validate_alarm_queue();
}

static void
ao_task_from_alarm_queue(struct ao_task *task)
{
	ao_list_del(&task->alarm_queue);
	if (ao_list_is_empty(&alarm_queue))
		ao_task_alarm_tick = 0;
	else
		ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
	ao_task_validate_alarm_queue();
}

static void
ao_task_init_queue(struct ao_task *task)
{
	ao_list_init(&task->queue);
	ao_list_init(&task->alarm_queue);
}

static void
ao_task_exit_queue(struct ao_task *task)
{
	ao_list_del(&task->queue);
	ao_list_del(&task->alarm_queue);
}

void
ao_task_check_alarm(uint16_t tick)
{
	struct ao_task	*alarm, *next;

	ao_list_for_each_entry_safe(alarm, next, &alarm_queue, struct ao_task, alarm_queue) {
		if ((int16_t) (tick - alarm->alarm) < 0)
			break;
		alarm->alarm = 0;
		ao_task_from_alarm_queue(alarm);
		ao_task_to_run_queue(alarm);
	}
}

void
ao_task_init(void)
{
	uint8_t	i;
	ao_list_init(&run_queue);
	ao_list_init(&alarm_queue);
	ao_task_alarm_tick = 0;
	for (i = 0; i < SLEEP_HASH_SIZE; i++)
		ao_list_init(&sleep_queue[i]);
}

#if DEBUG
static uint8_t
ao_task_validate_queue(struct ao_task *task)
{
	uint32_t flags;
	struct ao_task *m;
	uint8_t ret = 0;
	struct ao_list *queue;

	flags = ao_arch_irqsave();
	if (task->wchan) {
		queue = ao_task_sleep_queue(task->wchan);
		ret |= 2;
	} else {
		queue = &run_queue;
		ret |= 4;
	}
	ao_list_for_each_entry(m, queue, struct ao_task, queue) {
		if (m == task) {
			ret |= 1;
			break;
		}
	}
	ao_arch_irqrestore(flags);
	return ret;
}

static uint8_t
ao_task_validate_alarm(struct ao_task *task)
{
	uint32_t	flags;
	struct ao_task	*m;
	uint8_t		ret = 0;

	flags = ao_arch_irqsave();
	if (task->alarm == 0)
		return 0xff;
	ao_list_for_each_entry(m, &alarm_queue, struct ao_task, alarm_queue) {
		if (m == task)
			ret |= 1;
		else {
			if (!(ret&1)) {
				if ((int16_t) (m->alarm - task->alarm) > 0)
					ret |= 2;
			} else {
				if ((int16_t) (task->alarm - m->alarm) > 0)
					ret |= 4;
			}
		}
	}
	ao_arch_irqrestore(flags);
	return ret;
}


static void
ao_task_validate(void)
{
	uint8_t		i;
	struct ao_task	*task;
	uint8_t		ret;

	for (i = 0; i < ao_num_tasks; i++) {
		task = ao_tasks[i];
		ret = ao_task_validate_queue(task);
		if (!(ret & 1)) {
			if (ret & 2)
				printf ("sleeping task not on sleep queue %s %08x\n",
					task->name, task->wchan);
			else
				printf ("running task not on run queue %s\n",
					task->name);
		}
		ret = ao_task_validate_alarm(task);
		if (ret != 0xff) {
			if (!(ret & 1))
				printf ("alarm task not on alarm queue %s %d\n",
					task->name, task->alarm);
			if (ret & 2)
				printf ("alarm queue has sooner entries after %s %d\n",
					task->name, task->alarm);
			if (ret & 4)
				printf ("alarm queue has later entries before %s %d\n",
					task->name, task->alarm);
		}
	}
}
#endif /* DEBUG */

#endif /* HAS_TASK_QUEUE */

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
	task->task_id = task_id;
	task->name = name;
	task->wchan = NULL;
	/*
	 * Construct a stack frame so that it will 'return'
	 * to the start of the task
	 */
	ao_arch_init_stack(task, start);
	ao_arch_critical(
#if HAS_TASK_QUEUE
		ao_task_init_queue(task);
		ao_task_to_run_queue(task);
#endif
		ao_tasks[ao_num_tasks] = task;
		ao_num_tasks++;
		);
}

__data uint8_t	ao_task_minimize_latency;

/* Task switching function. This must not use any stack variables */
void
ao_yield(void) ao_arch_naked_define
{
	ao_arch_save_regs();

#if HAS_TASK_QUEUE
	if (ao_cur_task == NULL)
		ao_cur_task = ao_tasks[ao_num_tasks-1];
#else
	if (ao_cur_task_index == AO_NO_TASK_INDEX)
		ao_cur_task_index = ao_num_tasks-1;
#endif
	else
	{
#if HAS_SAMPLE_PROFILE
		uint16_t	tick = ao_sample_profile_timer_value();
		uint16_t	run = tick - ao_cur_task->start;
		if (run > ao_cur_task->max_run)
			ao_cur_task->max_run = run;
		++ao_cur_task->yields;
#endif
		ao_arch_save_stack();
	}

	ao_arch_isr_stack();
#if !HAS_TASK_QUEUE
	if (ao_task_minimize_latency)
		ao_arch_release_interrupts();
	else
#endif
		ao_arch_block_interrupts();

#if AO_CHECK_STACK
	in_yield = 1;
#endif
	/* Find a task to run. If there isn't any runnable task,
	 * this loop will run forever, which is just fine
	 */
#if HAS_TASK_QUEUE
	/* If the current task is running, move it to the
	 * end of the queue to allow other tasks a chance
	 */
	if (ao_cur_task->wchan == NULL)
		ao_task_to_run_queue(ao_cur_task);
	ao_cur_task = NULL;
	for (;;) {
		ao_arch_memory_barrier();
		if (!ao_list_is_empty(&run_queue))
			break;
		/* Wait for interrupts when there's nothing ready */
		ao_arch_wait_interrupt();
	}
	ao_cur_task = ao_list_first_entry(&run_queue, struct ao_task, queue);
#else
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

			/* Wait for interrupts when there's nothing ready */
			if (ao_cur_task_index == ao_last_task_index && !ao_task_minimize_latency)
				ao_arch_wait_interrupt();
		}
	}
#endif
#if HAS_SAMPLE_PROFILE
	ao_cur_task->start = ao_sample_profile_timer_value();
#endif
#if HAS_STACK_GUARD
	ao_mpu_stack_guard(ao_cur_task->stack);
#endif
#if AO_CHECK_STACK
	in_yield = 0;
#endif
	ao_arch_restore_stack();
}

uint8_t
ao_sleep(__xdata void *wchan)
{
#if HAS_TASK_QUEUE
	uint32_t flags;
	flags = ao_arch_irqsave();
#endif
	ao_cur_task->wchan = wchan;
#if HAS_TASK_QUEUE
	ao_task_to_sleep_queue(ao_cur_task, wchan);
	ao_arch_irqrestore(flags);
#endif
	ao_yield();
	if (ao_cur_task->wchan) {
		ao_cur_task->wchan = NULL;
		ao_cur_task->alarm = 0;
		return 1;
	}
	return 0;
}

void
ao_wakeup(__xdata void *wchan) __reentrant
{
#if HAS_TASK_QUEUE
	struct ao_task	*sleep, *next;
	struct ao_list	*sleep_queue;
	uint32_t	flags;

	if (ao_num_tasks == 0)
		return;
	sleep_queue = ao_task_sleep_queue(wchan);
	flags = ao_arch_irqsave();
	ao_list_for_each_entry_safe(sleep, next, sleep_queue, struct ao_task, queue) {
		if (sleep->wchan == wchan) {
			sleep->wchan = NULL;
			ao_task_to_run_queue(sleep);
		}
	}
	ao_arch_irqrestore(flags);
#else
	uint8_t	i;
	for (i = 0; i < ao_num_tasks; i++)
		if (ao_tasks[i]->wchan == wchan)
			ao_tasks[i]->wchan = NULL;
#endif
	ao_check_stack();
}

void
ao_alarm(uint16_t delay)
{
#if HAS_TASK_QUEUE
	uint32_t flags;
	/* Make sure we sleep *at least* delay ticks, which means adding
	 * one to account for the fact that we may be close to the next tick
	 */
	flags = ao_arch_irqsave();
#endif
	if (!(ao_cur_task->alarm = ao_time() + delay + 1))
		ao_cur_task->alarm = 1;
#if HAS_TASK_QUEUE
	ao_task_to_alarm_queue(ao_cur_task);
	ao_arch_irqrestore(flags);
#endif
}

void
ao_clear_alarm(void)
{
#if HAS_TASK_QUEUE
	uint32_t flags;

	flags = ao_arch_irqsave();
#endif
	ao_cur_task->alarm = 0;
#if HAS_TASK_QUEUE
	ao_task_from_alarm_queue(ao_cur_task);
	ao_arch_irqrestore(flags);
#endif
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
	uint8_t	i;
	ao_arch_block_interrupts();
	ao_num_tasks--;
#if HAS_TASK_QUEUE
	for (i = 0; i < ao_num_tasks; i++)
		if (ao_tasks[i] == ao_cur_task)
			break;
	ao_task_exit_queue(ao_cur_task);
#else
	i = ao_cur_task_index;
	ao_cur_task_index = AO_NO_TASK_INDEX;
#endif
	for (; i < ao_num_tasks; i++)
		ao_tasks[i] = ao_tasks[i+1];
	ao_cur_task = NULL;
	ao_yield();
	/* we'll never get back here */
}

#if HAS_TASK_INFO
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
#if HAS_TASK_QUEUE && DEBUG
	ao_task_validate();
#endif
}
#endif

void
ao_start_scheduler(void)
{
#if !HAS_TASK_QUEUE
	ao_cur_task_index = AO_NO_TASK_INDEX;
#endif
	ao_cur_task = NULL;
#if HAS_ARCH_START_SCHEDULER
	ao_arch_start_scheduler();
#endif
	ao_yield();
}
