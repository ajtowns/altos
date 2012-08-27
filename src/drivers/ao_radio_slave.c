/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
#include <ao_radio_spi.h>
#include <ao_radio_cmac.h>

static __xdata struct ao_radio_spi_reply ao_radio_spi_reply;

static __xdata struct ao_radio_spi_request ao_radio_spi_request;

static __xdata uint8_t	ao_radio_spi_recv_request;
static __xdata uint8_t	ao_radio_spi_recv_len;
static __xdata uint16_t	ao_radio_spi_recv_timeout;

static void
ao_radio_slave_signal(void)
{
	ao_gpio_set(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 0);
	ao_arch_nop();
	ao_arch_nop();
	ao_arch_nop();
	ao_gpio_set(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 0);
}

static void
ao_radio_slave_spi(void)
{
	for (;;) {
		ao_spi_get_slave(AO_RADIO_SLAVE_BUS);
		ao_spi_recv(&ao_radio_spi_request, (2 << 13) | sizeof (ao_radio_spi_request), AO_RADIO_SLAVE_BUS);
		ao_spi_put_slave(AO_RADIO_SLAVE_BUS);
		ao_led_for(AO_LED_RED, AO_MS_TO_TICKS(1000));
		switch (ao_radio_spi_request.request) {
		case AO_RADIO_SPI_RECV:
		case AO_RADIO_SPI_CMAC_RECV:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_radio_spi_recv_request = ao_radio_spi_request.request;
			ao_radio_spi_recv_len = ao_radio_spi_request.recv_len;
			ao_radio_spi_recv_timeout = ao_radio_spi_request.timeout;
			ao_wakeup(&ao_radio_spi_recv_len);
			break;
		case AO_RADIO_SPI_RECV_FETCH:
			ao_spi_get_slave(AO_RADIO_SLAVE_BUS);
			ao_spi_send(&ao_radio_spi_reply,
				    ao_radio_spi_request.recv_len + AO_RADIO_SPI_REPLY_HEADER_LEN,
				    AO_RADIO_SLAVE_BUS);
			ao_spi_put_slave(AO_RADIO_SLAVE_BUS);
			break;
		case AO_RADIO_SPI_RECV_ABORT:
			ao_radio_recv_abort();
			break;
		case AO_RADIO_SPI_SEND:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_radio_send(&ao_radio_spi_request.payload, ao_radio_spi_request.len - AO_RADIO_SPI_REQUEST_HEADER_LEN);
			ao_radio_slave_signal();
			break;

		case AO_RADIO_SPI_CMAC_SEND:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_radio_cmac_send(&ao_radio_spi_request.payload, ao_radio_spi_request.len - AO_RADIO_SPI_REQUEST_HEADER_LEN);
			ao_radio_slave_signal();
			break;
			
		case AO_RADIO_SPI_CMAC_KEY:
			ao_xmemcpy(&ao_config.aes_key, ao_radio_spi_request.payload, AO_AES_LEN);
			ao_radio_slave_signal();
			break;

		case AO_RADIO_SPI_TEST_ON:
			ao_radio_test(1);
			ao_radio_slave_signal();
			break;

		case AO_RADIO_SPI_TEST_OFF:
			ao_radio_test(0);
			ao_radio_slave_signal();
			break;
		}
	}
}

static void
ao_radio_slave_recv(void)
{
	uint8_t	len;
	for (;;) {
		while (!ao_radio_spi_recv_len)
			ao_sleep(&ao_radio_spi_recv_len);
		len = ao_radio_spi_recv_len;
		ao_radio_spi_recv_len = 0;
		if (ao_radio_spi_recv_request == AO_RADIO_SPI_RECV) {
			ao_radio_spi_reply.status = ao_radio_recv(&ao_radio_spi_reply.payload, len);
			ao_radio_spi_reply.rssi = 0;
		} else {
			ao_radio_spi_reply.status = ao_radio_cmac_recv(&ao_radio_spi_reply.payload, len,
								       ao_radio_spi_recv_timeout);
			ao_radio_spi_reply.rssi = ao_radio_cmac_rssi;
		}
		ao_radio_slave_signal();
	}
}

static __xdata struct ao_task ao_radio_slave_spi_task;
static __xdata struct ao_task ao_radio_slave_recv_task;

void
ao_radio_slave_init(void)
{
	ao_add_task(&ao_radio_slave_spi_task, ao_radio_slave_spi, "radio_spi");
	ao_add_task(&ao_radio_slave_recv_task, ao_radio_slave_recv, "radio_recv");
	ao_enable_output(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 1);
}
