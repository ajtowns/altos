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

uint8_t	ao_btm_enable;
extern volatile __xdata struct ao_fifo	ao_usart1_rx_fifo;

void
ao_btm(void)
{
	char	c;
	while (ao_btm_enable) {
		c = ao_serial_pollchar();
		if (c != AO_READ_AGAIN)
			ao_usb_putchar(c);
		else {
			ao_usb_flush();
			ao_sleep(&ao_usart1_rx_fifo);
		}
	}
	ao_exit();
}

__xdata struct ao_task	ao_btm_task;

static void
ao_btm_forward(void)
{
	char c;
	ao_btm_enable = 1;
	flush();
	ao_add_task(&ao_btm_task, ao_btm, "btm");

	while ((c = ao_usb_getchar()) != '~') {
		if (c == '\n') c = '\r';
		ao_serial_putchar(c);
	}
	ao_btm_enable = 0;
	while (ao_btm_task.wchan) {
		ao_wakeup(&ao_usart1_rx_fifo);
		ao_delay(AO_MS_TO_TICKS(10));
	}
}

__code struct ao_cmds ao_btm_cmds[] = {
	{ ao_btm_forward,	"B <data>\0BTM serial link." },
	{ 0, NULL },
};


void
ao_btm_init (void)
{
	ao_serial_init();
	ao_serial_set_speed(AO_SERIAL_SPEED_19200);
	ao_cmd_register(&ao_btm_cmds[0]);
}
