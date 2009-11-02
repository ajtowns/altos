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
ao_packet_slave(void)
{
	ao_radio_set_packet();
	ao_tx_packet.addr = ao_serial_number;
	ao_tx_packet.len = AO_PACKET_SYN;
	while (ao_packet_enable) {
		ao_packet_recv();
		ao_packet_send();
	}
	ao_exit();
}

void
ao_packet_slave_start(void)
{
	ao_packet_enable = 1;
	ao_add_task(&ao_packet_task, ao_packet_slave, "slave");
}

void
ao_packet_slave_stop(void)
{
	ao_packet_enable = 0;
	ao_radio_abort();
	while (ao_packet_task.wchan) {
		ao_wake_task(&ao_packet_task);
		ao_yield();
	}
	ao_radio_set_telemetry();
}

void
ao_packet_slave_init(void)
{
	ao_add_stdio(ao_packet_pollchar,
		     ao_packet_putchar,
		     ao_packet_flush);
}
