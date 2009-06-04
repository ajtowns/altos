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

void
ao_mutex_get(__xdata uint8_t *mutex) __reentrant
{
	if (*mutex == ao_cur_task->task_id)
		ao_panic(AO_PANIC_MUTEX);
	__critical {
		while (*mutex)
			ao_sleep(mutex);
		*mutex = ao_cur_task->task_id;
	}
}

void
ao_mutex_put(__xdata uint8_t *mutex) __reentrant
{
	if (*mutex != ao_cur_task->task_id)
		ao_panic(AO_PANIC_MUTEX);
	__critical {
		*mutex = 0;
		ao_wakeup(mutex);
	}
}
