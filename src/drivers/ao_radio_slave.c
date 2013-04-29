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

static __xdata uint8_t	slave_state;

static void
ao_radio_slave_low(void)
{
	uint16_t	i;

	if (slave_state != 1)
		ao_panic(1);
	ao_gpio_set(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 0);
	for (i = 0; i < 1000; i++)
		ao_arch_nop();
	slave_state = 0;
}

static void
ao_radio_slave_high(void)
{
	if (slave_state != 0)
		ao_panic(2);
	ao_gpio_set(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 1);
	slave_state = 1;
}

static void
ao_radio_slave_spi(void)
{
	ao_spi_get_slave(AO_RADIO_SLAVE_BUS);
	for (;;) {
		ao_spi_recv(&ao_radio_spi_request,
			    (2 << 13) | sizeof (ao_radio_spi_request),
			    AO_RADIO_SLAVE_BUS);
		ao_radio_slave_high();
		ao_spi_recv_wait();
		switch (ao_radio_spi_request.request) {
		case AO_RADIO_SPI_RECV:

			/* XXX monitor CS to interrupt the receive */

			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_led_on(AO_LED_RX);
			ao_radio_spi_reply.status = ao_radio_recv(&ao_radio_spi_reply.payload,
								  ao_radio_spi_request.recv_len,
								  ao_radio_spi_request.timeout);
			ao_led_off(AO_LED_RX);
			ao_radio_spi_reply.rssi = 0;
			ao_spi_send(&ao_radio_spi_reply,
				    AO_RADIO_SPI_REPLY_HEADER_LEN + ao_radio_spi_request.recv_len,
				    AO_RADIO_SLAVE_BUS);
			ao_radio_slave_low();
			ao_spi_send_wait();
			continue;
		case AO_RADIO_SPI_CMAC_RECV:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_led_on(AO_LED_RX);
			ao_radio_spi_reply.status = ao_radio_cmac_recv(&ao_radio_spi_reply.payload,
								       ao_radio_spi_request.recv_len,
								       ao_radio_spi_request.timeout);
			ao_led_off(AO_LED_RX);
			ao_radio_spi_reply.rssi = ao_radio_cmac_rssi;
			ao_spi_send(&ao_radio_spi_reply,
				    AO_RADIO_SPI_REPLY_HEADER_LEN + ao_radio_spi_request.recv_len,
				    AO_RADIO_SLAVE_BUS);
			ao_radio_slave_low();
			ao_spi_send_wait();
			continue;
		case AO_RADIO_SPI_SEND:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_led_on(AO_LED_TX);
			ao_radio_send(&ao_radio_spi_request.payload,
				      ao_radio_spi_request.len - AO_RADIO_SPI_REQUEST_HEADER_LEN);
			ao_led_off(AO_LED_TX);
			break;

		case AO_RADIO_SPI_CMAC_SEND:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_led_on(AO_LED_TX);
			ao_radio_cmac_send(&ao_radio_spi_request.payload,
					   ao_radio_spi_request.len - AO_RADIO_SPI_REQUEST_HEADER_LEN);
			ao_led_off(AO_LED_TX);
			break;
			
		case AO_RADIO_SPI_CMAC_KEY:
			ao_xmemcpy(&ao_config.aes_key, ao_radio_spi_request.payload, AO_AES_LEN);
			break;

		case AO_RADIO_SPI_TEST_ON:
			ao_config.radio_setting = ao_radio_spi_request.setting;
			ao_radio_test(1);
			break;

		case AO_RADIO_SPI_TEST_OFF:
			ao_radio_test(0);
			break;
		}
		ao_radio_slave_low();
	}
}

static __xdata struct ao_task ao_radio_slave_spi_task;

void
ao_radio_slave_init(void)
{
	ao_add_task(&ao_radio_slave_spi_task, ao_radio_slave_spi, "radio_spi");
	ao_enable_output(AO_RADIO_SLAVE_INT_PORT, AO_RADIO_SLAVE_INT_BIT, AO_RADIO_SLAVE_INT_PIN, 0);
}
