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

__xdata struct ao_packet_recv ao_rx_packet;
__xdata struct ao_packet ao_tx_packet;
__pdata uint8_t ao_packet_rx_len, ao_packet_rx_used, ao_packet_tx_used;

static __xdata uint8_t tx_data[AO_PACKET_MAX];
static __xdata uint8_t rx_data[AO_PACKET_MAX];
static __pdata uint8_t rx_seq;

__xdata struct ao_task	ao_packet_task;
__xdata uint8_t ao_packet_enable;
__xdata uint8_t ao_packet_restart;

#if PACKET_HAS_MASTER
__xdata uint8_t ao_packet_master_sleeping;
#endif

void
ao_packet_send(void)
{
#ifdef AO_LED_RED
	ao_led_on(AO_LED_RED);
#endif
	/* If any tx data is pending then copy it into the tx packet */
	if (ao_packet_tx_used && ao_tx_packet.len == 0) {
		ao_xmemcpy(&ao_tx_packet.d, tx_data, ao_packet_tx_used);
		ao_tx_packet.len = ao_packet_tx_used;
		ao_tx_packet.seq++;
		ao_packet_tx_used = 0;
		ao_wakeup(&tx_data);
	}
	ao_radio_send(&ao_tx_packet, sizeof (ao_tx_packet));
#ifdef AO_LED_RED
	ao_led_off(AO_LED_RED);
#endif
}

uint8_t
ao_packet_recv(void)
{
	uint8_t	dma_done;

#ifdef AO_LED_GREEN
	ao_led_on(AO_LED_GREEN);
#endif
	dma_done = ao_radio_recv(&ao_rx_packet, sizeof (struct ao_packet_recv), 0);
#ifdef AO_LED_GREEN
	ao_led_off(AO_LED_GREEN);
#endif
#if AO_PROFILE
		{
			extern uint32_t	ao_rx_start_tick, ao_rx_packet_tick, ao_rx_done_tick, ao_rx_last_done_tick;
			extern uint32_t ao_fec_decode_start, ao_fec_decode_end;

			printf ("between packet: %d\n", ao_rx_start_tick - ao_rx_last_done_tick);
			printf ("receive start delay: %d\n", ao_rx_packet_tick - ao_rx_start_tick);
			printf ("decode time: %d\n", ao_fec_decode_end - ao_fec_decode_start);
			printf ("rx cleanup: %d\n\n", ao_rx_done_tick - ao_fec_decode_end);
			flush();
		}
#endif

	/* Check to see if we got a valid packet */
	if (!dma_done)
		return 0;
	if (!(ao_rx_packet.status & AO_RADIO_STATUS_CRC_OK))
		return 0;

	/* Accept packets with matching call signs, or any packet if
	 * our callsign hasn't been configured
	 */
	if (ao_xmemcmp(ao_rx_packet.packet.callsign,
		       ao_config.callsign,
		       AO_MAX_CALLSIGN) != 0 &&
	    ao_xmemcmp(ao_config.callsign, CODE_TO_XDATA("N0CALL"), 7) != 0)
		return 0;

	/* SYN packets carry no data */
	if (ao_rx_packet.packet.len == AO_PACKET_SYN) {
		rx_seq = ao_rx_packet.packet.seq;
		ao_tx_packet.seq = ao_rx_packet.packet.ack;
		ao_tx_packet.ack = rx_seq;
	} else if (ao_rx_packet.packet.len) {

		/* Check for incoming data at the next sequence and
		 * for an empty data buffer
		 */
		if ((ao_rx_packet.packet.seq == (uint8_t) (rx_seq + (uint8_t) 1) ||
		     ao_packet_restart) &&
		    ao_packet_rx_used == ao_packet_rx_len) {

			/* Copy data to the receive data buffer and set up the
			 * offsets
			 */
			ao_xmemcpy(rx_data, ao_rx_packet.packet.d, ao_rx_packet.packet.len);
			ao_packet_rx_used = 0;
			ao_packet_rx_len = ao_rx_packet.packet.len;

			/* Mark the sequence that we've received to
			 * let the sender know when we return a packet
			 */
			rx_seq = ao_rx_packet.packet.seq;
			ao_tx_packet.ack = rx_seq;

			/* Poke anyone looking for received data */
			ao_wakeup(&ao_stdin_ready);
		}
	}
	ao_packet_restart = 0;

	/* If the other side has seen the latest data we queued,
	 * wake up any task waiting to send data and let them go again
	 */
	if (ao_rx_packet.packet.ack == ao_tx_packet.seq) {
		ao_tx_packet.len = 0;
		ao_wakeup(&ao_tx_packet);
	}
	return 1;
}

#if PACKET_HAS_MASTER
void
ao_packet_flush(void)
{
	/* If there is data to send, and this is the master,
	 * then poke the master to send all queued data
	 */
	if (ao_packet_tx_used && ao_packet_master_sleeping)
		ao_wakeup(&ao_packet_master_sleeping);
}
#endif /* PACKET_HAS_MASTER */

void
ao_packet_putchar(char c) __reentrant
{
	/* No need to block interrupts, all variables here
	 * are only manipulated in task context
	 */
	while (ao_packet_tx_used == AO_PACKET_MAX && ao_packet_enable) {
#if PACKET_HAS_MASTER
		ao_packet_flush();
#endif
		ao_sleep(&tx_data);
	}

	if (ao_packet_enable)
		tx_data[ao_packet_tx_used++] = c;
}

/* May be called with interrupts blocked */
int
_ao_packet_pollchar(void)
{
	if (!ao_packet_enable)
		return AO_READ_AGAIN;

	if (ao_packet_rx_used == ao_packet_rx_len)
		return AO_READ_AGAIN;

	return rx_data[ao_packet_rx_used++];
}
