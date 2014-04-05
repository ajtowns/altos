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

#include <ao.h>
#include <ao_sample_profile.h>
#include <ao_task.h>

#ifndef AO_SAMPLE_PROFILE_LOW_PC
#define AO_SAMPLE_PROFILE_LOW_PC	0x08002000
#endif

#ifndef AO_SAMPLE_PROFILE_HIGH_PC
#define AO_SAMPLE_PROFILE_HIGH_PC	0x0800f000
#endif

#ifndef AO_SAMPLE_PROFILE_SHIFT
#define AO_SAMPLE_PROFILE_SHIFT		6
#endif

#define AO_SAMPLE_PROFILE_RANGE		(AO_SAMPLE_PROFILE_HIGH_PC - AO_SAMPLE_PROFILE_LOW_PC)
#define AO_SAMPLE_PROFILE_NUM		(AO_SAMPLE_PROFILE_RANGE >> AO_SAMPLE_PROFILE_SHIFT)

static uint16_t	prev_tick;
static uint16_t	samples[AO_SAMPLE_PROFILE_NUM];
static uint8_t missed[AO_SAMPLE_PROFILE_NUM/8];
static uint16_t max_miss;
static uint32_t task, isr, os, idle;

extern uint8_t ao_idle_loc;

void
ao_sample_profile_point(uint32_t pc, uint16_t tick, uint8_t in_isr)
{
	uint16_t	delta = tick - prev_tick;

	if (pc < AO_SAMPLE_PROFILE_LOW_PC)
		return;
	if (pc >= AO_SAMPLE_PROFILE_HIGH_PC)
		return;
	if (ao_cur_task) {
		uint8_t		*sp;
		int32_t		sp_delta;
		
		asm("mov %0,sp" : "=&r" (sp));
		sp_delta = sp - (uint8_t *) ao_cur_task->stack;
		if (-96 < sp_delta && sp_delta < 16)
			ao_panic(AO_PANIC_STACK);
	}

	if (in_isr)
		isr += delta;
	else if (ao_cur_task) {
		ao_cur_task->ticks += delta;
		task += delta;
	} else if (pc == (uint32_t) &ao_idle_loc)
		idle += delta;
	else
		os += delta;

	pc -= AO_SAMPLE_PROFILE_LOW_PC;
	pc >>= AO_SAMPLE_PROFILE_SHIFT;
	samples[pc] += delta;

	if (delta > 1)
		missed[pc >> 3] |= (1 << (pc & 7));
	if (delta > max_miss)
		max_miss = delta;
	prev_tick = tick;
}

static void
ao_sample_profile_start(void)
{
	prev_tick = ao_sample_profile_timer_start();
}

static void
ao_sample_profile_stop(void)
{
	ao_sample_profile_timer_stop();
}

static void
ao_sample_profile_dump(void)
{
	uint16_t	a;
	uint8_t		t;

	printf ("task %6d\n", task);
	printf ("isr  %6d\n", isr);
	printf ("os   %6d\n", os);
	printf ("idle %6d\n", idle);
	printf ("irq blocked %d\n", max_miss);
	for (t = 0; t < ao_num_tasks; t++)
		printf ("task %6d %6d %6d %s\n",
			ao_tasks[t]->ticks,
			ao_tasks[t]->yields,
			ao_tasks[t]->max_run,
			ao_tasks[t]->name);
	for (a = 0; a < AO_SAMPLE_PROFILE_NUM; a++) {
		if (samples[a])
			printf ("%04x %c %u\n",
				(a << AO_SAMPLE_PROFILE_SHIFT) + AO_SAMPLE_PROFILE_LOW_PC,
				missed[a >> 3] & (1 << (a & 7)) ? '*' : ' ',
				samples[a]);
	}
}

static void
ao_sample_profile_clear(void)
{
	int t;

	task = isr = os = idle = 0;
	max_miss = 0;
	memset(samples, '\0', sizeof (samples));
	memset(missed, '\0', sizeof (missed));
	for (t = 0; t < ao_num_tasks; t++) {
		ao_tasks[t]->ticks = 0;
		ao_tasks[t]->yields = 0;
		ao_tasks[t]->max_run = 0;
	}
}

static void
ao_sample_profile_cmd(void)
{
	ao_cmd_white();
	switch (ao_cmd_lex_c) {
	case '1':
		ao_sample_profile_start();
		break;
	case '0':
		ao_sample_profile_stop();
		break;
	case 'd':
		ao_sample_profile_dump();
		break;
	case 'c':
		ao_sample_profile_clear();
		break;
	default:
		ao_cmd_status = ao_cmd_syntax_error;
		break;
	}
}

static __code struct ao_cmds ao_sample_profile_cmds[] = {
	{ ao_sample_profile_cmd,	"S <1 start,0 stop, d dump,c clear>\0Sample profile" },
	{ 0, NULL }
};

void
ao_sample_profile_init(void)
{
	ao_sample_profile_timer_init();
	ao_cmd_register(&ao_sample_profile_cmds[0]);
	ao_sample_profile_clear();
}
