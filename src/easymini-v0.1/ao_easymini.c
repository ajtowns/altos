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

#include <ao.h>
#include <ao_exti.h>

void
main(void)
{
	ao_clock_init();
	ao_task_init();
	ao_timer_init();
	ao_exti_init();

	ao_spi_init();
	ao_storage_init();

	ao_usb_init();

	ao_cmd_init();
#if 0
	ao_flight_init();
	ao_ms5607_init();
	ao_log_init();
	ao_report_init();
	ao_igniter_init();
#endif
	ao_config_init();
	ao_start_scheduler();
}
