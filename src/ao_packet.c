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

static __xdata struct ao_packet_recv rx_packet;
static __xdata struct ao_packet tx_packet;
static __xdata char tx_data[AO_PACKET_MAX];
static __xdata char rx_data[AO_PACKET_MAX];
static __pdata uint8_t rx_len, rx_used, tx_used;
static __pdata uint8_t rx_seq;

static __xdata struct ao_task	ao_packet_task;
static __xdata uint8_t ao_packet_enable;
static __xdata uint8_t ao_packet_master_sleeping;

void
ao_packet_send(void)
{
	ao_config_get();
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	ao_radio_done = 0;
	RF_CHANNR = ao_config.radio_channel;
	ao_dma_set_transfer(ao_radio_dma,
			    &tx_packet,
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
	ao_mutex_put(&ao_radio_mutex);
}

uint8_t
ao_packet_recv(void)
{
	uint8_t	dma_done;

	ao_config_get();
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_idle();
	RF_CHANNR = ao_config.radio_channel;
	ao_dma_set_transfer(ao_radio_dma,
			    &RFDXADDR,
			    &rx_packet,
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
			   if (ao_sleep(&ao_radio_dma_done) != 0) {
				   printf("recv timeout\n"); flush();
				   ao_radio_abort();
			   }
	dma_done = ao_radio_dma_done;
	ao_mutex_put(&ao_radio_mutex);

	if (dma_done & AO_DMA_DONE) {
		if (!(rx_packet.status & PKT_APPEND_STATUS_1_CRC_OK)) {
			printf ("bad crc\n"); flush();
			return AO_DMA_ABORTED;
		}
		if (rx_packet.packet.len == AO_PACKET_SYN) {
			rx_seq = rx_packet.packet.seq;
			tx_packet.seq = rx_packet.packet.ack;
			tx_packet.ack = rx_seq;
		} else if (rx_packet.packet.len) {
			if (rx_packet.packet.seq == rx_seq + 1 && rx_used == rx_len)
			{
				printf ("rx len %3d seq %3d ack %3d\n",
					rx_packet.packet.len,
					rx_packet.packet.seq,
					rx_packet.packet.ack);
				flush();
				memcpy(rx_data, rx_packet.packet.d, rx_packet.packet.len);
				rx_used = 0;
				rx_len = rx_packet.packet.len;
				rx_seq = rx_packet.packet.seq;
				tx_packet.ack = rx_seq;
				ao_wakeup(&rx_data);
			}
		}
		if (rx_packet.packet.ack == tx_packet.seq) {
			tx_packet.len = 0;
			ao_wakeup(&tx_packet);
		}
	}
	return dma_done;
}

void
ao_packet_slave(void)
{
	ao_radio_set_packet();
	tx_packet.addr = ao_serial_number;
	tx_packet.len = AO_PACKET_SYN;
	while (ao_packet_enable) {
		ao_led_on(AO_LED_GREEN);
		ao_packet_recv();
		ao_led_off(AO_LED_GREEN);
		ao_led_on(AO_LED_RED);
		ao_delay(AO_MS_TO_TICKS(100));
		ao_packet_send();
		ao_led_off(AO_LED_RED);
	}
	ao_exit();
}

/* Thread for the master side of the packet link */

void
ao_packet_master(void)
{
	uint8_t	status;

	ao_radio_set_packet();
	tx_packet.addr = ao_serial_number;
	tx_packet.len = AO_PACKET_SYN;
	while (ao_packet_enable) {
		ao_led_on(AO_LED_RED);
		ao_delay(AO_MS_TO_TICKS(100));
		ao_packet_send();
		ao_led_off(AO_LED_RED);
		ao_led_on(AO_LED_GREEN);
		ao_alarm(AO_MS_TO_TICKS(1000));
		status = ao_packet_recv();
		ao_led_off(AO_LED_GREEN);
		if (status & AO_DMA_DONE) {
			ao_packet_master_sleeping = 1;
			ao_delay(AO_MS_TO_TICKS(1000));
			ao_packet_master_sleeping = 0;
		}
	}
	ao_exit();
}

void
ao_packet_flush(void)
{
	if (!tx_used)
		return;

	/* Wait for previous packet to be received
	 */
	while (tx_packet.len)
		ao_sleep(&tx_packet);

	/* Prepare next packet
	 */
	if (tx_used) {
		memcpy(&tx_packet.d, tx_data, tx_used);
		tx_packet.len = tx_used;
		tx_packet.seq++;
		tx_used = 0;

		if (ao_packet_master_sleeping)
			ao_wake_task(&ao_packet_task);
	}
}

void
ao_packet_putchar(char c)
{
	while (tx_used == AO_PACKET_MAX && ao_packet_enable)
		ao_packet_flush();

	if (ao_packet_enable)
		tx_data[tx_used++] = c;
}

char
ao_packet_getchar(void) __critical
{
	while (rx_used == rx_len && ao_packet_enable)
		ao_sleep(&rx_data);

	if (!ao_packet_enable)
		return 0;

	return rx_data[rx_used++];
}

static void
ao_packet_echo(void) __reentrant
{
	uint8_t	c;
	while (ao_packet_enable) {
		c = ao_packet_getchar();
		if (ao_packet_enable)
			putchar(c);
	}
	ao_exit();
}

static __xdata struct ao_task	ao_packet_echo_task;

static void
ao_packet_forward(void) __reentrant
{
	char c;
	ao_packet_enable = 1;
	ao_cmd_white();

	if (ao_cmd_lex_c == 'm')
		ao_add_task(&ao_packet_task, ao_packet_master, "master");
	else
		ao_add_task(&ao_packet_task, ao_packet_slave, "slave");
	ao_add_task(&ao_packet_echo_task, ao_packet_echo, "echo");
	while ((c = getchar()) != '~') {
		ao_packet_putchar(c);
		if (c == '\n')
			ao_packet_flush();
	}
	ao_packet_enable = 0;
	ao_radio_abort();
	while (ao_packet_echo_task.wchan || ao_packet_task.wchan) {
		ao_wake_task(&ao_packet_echo_task);
		ao_wake_task(&ao_packet_task);
	}
#endif
}

__code struct ao_cmds ao_packet_cmds[] = {
	{ 'p',	ao_packet_forward,	"p {m|s}                            Remote packet link. m=master, s=slave" },
	{ 0,	ao_packet_forward,	NULL },
};

void
ao_packet_init(void)
{
	ao_cmd_register(&ao_packet_cmds[0]);
}
