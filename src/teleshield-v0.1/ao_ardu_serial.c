/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

static void
ao_ardu_serial_recv(void)
{
	char	c;

	for (;;) {
		if (ao_fifo_empty(ao_serial0_rx_fifo))
			flush();
		c = ao_serial0_getchar();
		putchar (c);
	}
}

static __xdata struct ao_task ao_ardu_serial_recv_task;

void
ao_ardu_serial_init (void)
{
	ao_add_task(&ao_ardu_serial_recv_task, ao_ardu_serial_recv, "recv");
}
