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
	ao_tx_packet.addr = ao_serial_number;
	ao_tx_packet.len = AO_PACKET_SYN;
	while (ao_packet_enable) {
		if (ao_packet_recv()) {
			memcpy(&ao_tx_packet.callsign, &ao_rx_packet.packet.callsign, AO_MAX_CALLSIGN);
#if HAS_FLIGHT
			ao_flight_force_idle = TRUE;
#endif
			ao_packet_send();
		}
	}
	ao_exit();
}

void
ao_packet_slave_start(void)
{
	if (!ao_packet_enable) {
		ao_packet_enable = 1;
		ao_add_task(&ao_packet_task, ao_packet_slave, "slave");
	}
}

void
ao_packet_slave_stop(void)
{
	if (ao_packet_enable) {
		ao_packet_enable = 0;
		while (ao_packet_task.wchan) {
			ao_radio_recv_abort();
			ao_delay(AO_MS_TO_TICKS(10));
		}
	}
}

void
ao_packet_slave_init(uint8_t enable)
{
	ao_add_stdio(ao_packet_pollchar,
		     ao_packet_putchar,
		     NULL);
	if (enable)
		ao_packet_slave_start();
}
