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

static inline int min(int a, int b) { return a < b ? a : b; }

void
ao_demo(void)
{
	char	message[] = "Hello, Mike & Bdale --- ";
	char	part[7];
	int	i = 0;
	int	len = sizeof(message) - 1;
	int	first, second;

	part[6] = '\0';
	for (;;) {
		ao_delay(AO_MS_TO_TICKS(150));
		first = min(6, len - i);
		second = 6 - first;
		memcpy(part, message + i, first);
		memcpy(part + first, message, second);
		ao_lcd_font_string(part);
		if (++i >= len)
			i = 0;
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
	ao_lcd_stm_init();
	ao_lcd_font_init();
	ao_add_task(&demo_task, ao_demo, "demo");
	
	ao_start_scheduler();
	return 0;
}
