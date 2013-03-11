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
#include <ao_exti.h>
#include <ao_boot.h>
#include <ao_flash_stm.h>

void
ao_panic(uint8_t reason)
{
	for (;;);
}

void
ao_application(void)
{
	ao_boot_reboot(AO_BOOT_APPLICATION_BASE);
}

__code struct ao_cmds ao_flash_cmds[] = {
	{ ao_application, "A\0Switch to application" },
	{ 0, NULL },
};


int
main(void)
{
	ao_clock_init();

	ao_task_init();

	ao_timer_init();
//	ao_dma_init();
	ao_cmd_init();
//	ao_exti_init();
	ao_usb_init();

	ao_cmd_register(&ao_flash_cmds[0]);
	ao_start_scheduler();
	return 0;
}
