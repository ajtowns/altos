/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

static int	ao_watchdog_enabled = TRUE;

static void
ao_watchdog(void)
{
	for (;;) {
		while (!ao_watchdog_enabled)
			ao_sleep(&ao_watchdog_enabled);
		while (ao_watchdog_enabled) {
			ao_delay(AO_WATCHDOG_INTERVAL);
			ao_gpio_set(AO_WATCHDOG_PORT, AO_WATCHDOG_BIT, AO_WATCHDOG_PIN, 1);
			ao_delay(1);
			ao_gpio_set(AO_WATCHDOG_PORT, AO_WATCHDOG_BIT, AO_WATCHDOG_PIN, 0);
		}
	}
}

static void
ao_watchdog_set(void)
{
	ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success) {
		ao_watchdog_enabled = ao_cmd_lex_i != 0;
		ao_wakeup(&ao_watchdog_enabled);
	}
}
	

static __code struct ao_cmds ao_watchdog_cmds[] = {
	{ ao_watchdog_set,	"Q <0 off, 1 on>\0Enable or disable watchdog timer" },
	{ 0,			NULL },
};

static struct ao_task watchdog_task;

void
ao_watchdog_init(void)
{
	ao_enable_output(AO_WATCHDOG_PORT, AO_WATCHDOG_BIT, AO_WATCHDOG, 0);
	ao_cmd_register(&ao_watchdog_cmds[0]);
	ao_add_task(&watchdog_task, ao_watchdog, "watchdog");
}

