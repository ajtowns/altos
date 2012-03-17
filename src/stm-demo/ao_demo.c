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

struct ao_task demo_task;

void
ao_demo(void)
{
	int	i = 0;
	for (;;) {
		printf ("hello %d\n", i++);
	}
}

void _close() { }
void _sbrk() { }
void _isatty() { }
void _lseek() { }
void _exit () { }
void _read () { }
void _fstat() { }
int
main(void)
{
	ao_clock_init();
	
	ao_serial_init();
	ao_timer_init();
	ao_cmd_init();
	
	ao_demo();
	return 0;
}
