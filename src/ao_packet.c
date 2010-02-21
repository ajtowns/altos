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

static __xdata char tx_data[AO_PACKET_MAX];
static __xdata char rx_data[AO_PACKET_MAX];
static __pdata uint8_t rx_seq;

__xdata struct ao_task	ao_packet_task;
__xdata uint8_t ao_packet_enable;
__xdata uint8_t ao_packet_master_sleeping;

void
ao_packet_send(void)
{
	ao_led_on(AO_LED_RED);
	ao_radio_get();

	/* If any tx data is pending then copy it into the tx packet */
	if (ao_packet_tx_used && ao_tx_packet.len == 0) {
		memcpy(&ao_tx_packet.d, tx_data, ao_packet_tx_used);
		ao_tx_packet.len = ao_packet_tx_used;
		ao_tx_packet.seq++;
		ao_packet_tx_used = 0;
		ao_wakeup(&tx_data);
	}
	ao_radio_done = 0;
	ao_dma_set_transfer(ao_radio_dma,
			    &ao_tx_packet,
			    &RFDXADDR,
			    sizeof (struct ao_packet),
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_STX;
	__critical while (!ao_radio_done)
		ao_sleep(&ao_radio_done);
	ao_radio_put();
	ao_led_off(AO_LED_RED);
}

uint8_t
ao_packet_recv(void)
{
	uint8_t	dma_done;

	ao_led_on(AO_LED_GREEN);
	ao_radio_get();
	ao_dma_set_transfer(ao_radio_dma,
			    &RFDXADDR,
			    &ao_rx_packet,
			    sizeof (struct ao_packet_recv),
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_RADIO,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_HIGH);
	ao_dma_start(ao_radio_dma);
	RFST = RFST_SRX;
	__critical while (!ao_radio_dma_done)
			   if (ao_sleep(&ao_radio_dma_done) != 0)
				   ao_radio_abort();
	dma_done = ao_radio_dma_done;
	ao_radio_put();
	ao_led_off(AO_LED_GREEN);

	if (dma_done & AO_DMA_DONE) {
		if (!(ao_rx_packet.status & PKT_APPEND_STATUS_1_CRC_OK))
			return AO_DMA_ABORTED;
		if (ao_rx_packet.packet.len == AO_PACKET_SYN) {
			rx_seq = ao_rx_packet.packet.seq;
			ao_tx_packet.seq = ao_rx_packet.packet.ack;
			ao_tx_packet.ack = rx_seq;
		} else if (ao_rx_packet.packet.len) {
			if (ao_rx_packet.packet.seq == (uint8_t) (rx_seq + (uint8_t) 1) && ao_packet_rx_used == ao_packet_rx_len) {
				memcpy(rx_data, ao_rx_packet.packet.d, ao_rx_packet.packet.len);
				ao_packet_rx_used = 0;
				ao_packet_rx_len = ao_rx_packet.packet.len;
				rx_seq = ao_rx_packet.packet.seq;
				ao_tx_packet.ack = rx_seq;
				ao_wakeup(&ao_stdin_ready);
			}
		}
		if (ao_rx_packet.packet.ack == ao_tx_packet.seq) {
			ao_tx_packet.len = 0;
			ao_wakeup(&ao_tx_packet);
		}
	}
	return dma_done;
}

#ifndef PACKET_HAS_MASTER
#define PACKET_HAS_MASTER 1
#endif

#if PACKET_HAS_MASTER
void
ao_packet_flush(void)
{
	/* If there is data to send, and this is the master,
	 * then poke the master to send all queued data
	 */
	if (ao_packet_tx_used && ao_packet_master_sleeping)
		ao_wake_task(&ao_packet_task);
}
#endif /* PACKET_HAS_MASTER */

void
ao_packet_putchar(char c) __reentrant
{
	while (ao_packet_tx_used == AO_PACKET_MAX && ao_packet_enable) {
#if PACKET_HAS_MASTER
		ao_packet_flush();
#endif
		ao_sleep(&tx_data);
	}

	if (ao_packet_enable)
		tx_data[ao_packet_tx_used++] = c;
}

char
ao_packet_pollchar(void) __critical
{
	if (!ao_packet_enable)
		return AO_READ_AGAIN;

	if (ao_packet_rx_used == ao_packet_rx_len)
		return AO_READ_AGAIN;

	return rx_data[ao_packet_rx_used++];
}
