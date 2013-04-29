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
#include <ao_exti.h>
#include <ao_radio_cmac.h>

static __xdata struct ao_radio_spi_reply	ao_radio_spi_reply;
static __xdata struct ao_radio_spi_request	ao_radio_spi_request;
static volatile __xdata uint8_t			ao_radio_wait_mode;
static volatile __xdata uint8_t			ao_radio_done = 0;
static volatile __xdata uint8_t			ao_radio_ready = 1;
static __xdata uint8_t				ao_radio_mutex;
static __xdata uint8_t				ao_radio_aes_seq;

__xdata int8_t					ao_radio_cmac_rssi;

#if 0
#define PRINTD(...) do { printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#else
#define PRINTD(...) 
#endif

static void
ao_radio_isr(void)
{
	if (ao_gpio_get(AO_RADIO_INT_PORT, AO_RADIO_INT_PIN, AO_RADIO_INT)) {
		ao_radio_ready = 1;
		ao_wakeup((void *) &ao_radio_ready);
	} else {
		ao_radio_done = 1;
		ao_wakeup((void *) &ao_radio_done);
	}
}

static void
ao_radio_master_start(void)
{
	ao_spi_get_bit(AO_RADIO_CS_PORT, AO_RADIO_CS_PIN, AO_RADIO_CS,
		       AO_RADIO_SPI_BUS,
		       AO_SPI_SPEED_2MHz);
}

static void
ao_radio_master_stop(void)
{
	ao_spi_put_bit(AO_RADIO_CS_PORT, AO_RADIO_CS_PIN, AO_RADIO_CS,
		       AO_RADIO_SPI_BUS);
}

static uint8_t
ao_radio_master_send(void)
{
	uint8_t	ret;

	PRINTD("send %d\n", ao_radio_spi_request.len);
	ao_radio_done = 0;

	/* Wait for radio chip to be ready for a command
	 */

	PRINTD("Waiting radio ready\n");
	ao_arch_block_interrupts();
	ao_radio_ready = ao_gpio_get(AO_RADIO_INT_PORT,
				     AO_RADIO_INT_PIN, AO_RADIO_INT);
	ret = 0;
	while (!ao_radio_ready) {
		ret = ao_sleep((void *) &ao_radio_ready);
		if (ret)
			break;
	}
	ao_arch_release_interrupts();
	if (ret)
		return 0;

	PRINTD("radio_ready %d radio_done %d\n", ao_radio_ready, ao_radio_done);

	/* Send the command
	 */
	ao_radio_wait_mode = 0;
	ao_radio_master_start();
	ao_spi_send(&ao_radio_spi_request,
		    ao_radio_spi_request.len,
		    AO_RADIO_SPI_BUS);
	ao_radio_master_stop();
	PRINTD("waiting for send done %d\n", ao_radio_done);
	ao_arch_block_interrupts();
	while (!ao_radio_done)
		if (ao_sleep((void *) &ao_radio_done))
			break;
	ao_arch_release_interrupts();
	PRINTD ("sent, radio done %d isr_0 %d isr_1 %d\n", ao_radio_done, isr_0_count, isr_1_count);
	return ao_radio_done;
}

static void
ao_radio_get(uint8_t req, uint8_t len)
{
	ao_config_get();
	ao_mutex_get(&ao_radio_mutex);
	ao_radio_spi_request.len = AO_RADIO_SPI_REQUEST_HEADER_LEN + len;
	ao_radio_spi_request.request = req;
	ao_radio_spi_request.setting = ao_config.radio_setting;
}

static void
ao_radio_put(void)
{
	ao_mutex_put(&ao_radio_mutex);
}

static void
ao_radio_get_data(__xdata void *d, uint8_t size)
{
	PRINTD ("fetch\n");
	ao_radio_master_start();
	ao_spi_recv(&ao_radio_spi_reply,
		    AO_RADIO_SPI_REPLY_HEADER_LEN + size,
		    AO_RADIO_SPI_BUS);
	ao_radio_master_stop();
	ao_xmemcpy(d, ao_radio_spi_reply.payload, size);
	PRINTD ("fetched %d\n", size);
}

void
ao_radio_recv_abort(void)
{
	ao_radio_get(AO_RADIO_SPI_RECV_ABORT, 0);
	ao_radio_master_send();
	ao_radio_put();
}

void
ao_radio_send(const void *d, uint8_t size)
{
	ao_radio_get(AO_RADIO_SPI_SEND, size);
	ao_xmemcpy(&ao_radio_spi_request.payload, d, size);
	ao_radio_master_send();
	ao_radio_put();
}


uint8_t
ao_radio_recv(__xdata void *d, uint8_t size, uint8_t timeout)
{
	int8_t	ret;
	uint8_t	recv;

	/* Recv the data
	 */
	
	ao_radio_get(AO_RADIO_SPI_RECV, 0);
	ao_radio_spi_request.recv_len = size;
	ao_radio_spi_request.timeout = timeout;
	recv = ao_radio_master_send();
	if (!recv) {
		ao_radio_put();
		ao_radio_recv_abort();
		return 0;
	}
	ao_radio_get_data(d, size);
	recv = ao_radio_spi_reply.status;
	ao_radio_put();

	return recv;
}

static void
ao_radio_cmac_set_key(void)
{
	if (ao_radio_aes_seq == ao_config_aes_seq)
		return;
	/* Set the key.
	 */
	PRINTD ("set key\n");
	ao_radio_get(AO_RADIO_SPI_CMAC_KEY, AO_AES_LEN);
	ao_xmemcpy(&ao_radio_spi_request.payload, &ao_config.aes_key, AO_AES_LEN);
	ao_radio_master_send();
	ao_radio_put();
	PRINTD ("key set\n");
	ao_radio_aes_seq = ao_config_aes_seq;
}

int8_t
ao_radio_cmac_send(__xdata void *packet, uint8_t len) __reentrant
{
	if (len > AO_CMAC_MAX_LEN)
		return AO_RADIO_CMAC_LEN_ERROR;

	ao_radio_cmac_set_key();

	PRINTD ("cmac_send: send %d\n", len);

	/* Send the data
	 */
	
	PRINTD ("sending packet\n");
	ao_radio_get(AO_RADIO_SPI_CMAC_SEND, len);
	ao_xmemcpy(&ao_radio_spi_request.payload, packet, len);
	ao_radio_master_send();
	ao_radio_put();
	PRINTD ("packet sent\n");
	return AO_RADIO_CMAC_OK;
}

int8_t
ao_radio_cmac_recv(__xdata void *packet, uint8_t len, uint16_t timeout) __reentrant
{
	int8_t	ret;
	int8_t	recv;

	if (len > AO_CMAC_MAX_LEN)
		return AO_RADIO_CMAC_LEN_ERROR;

	ao_radio_cmac_set_key();

	/* Recv the data
	 */
	PRINTD ("queuing recv\n");
	ao_radio_get(AO_RADIO_SPI_CMAC_RECV, 0);
	ao_radio_spi_request.recv_len = len;
	ao_radio_spi_request.timeout = timeout;
	recv = ao_radio_master_send();
	PRINTD ("recv queued: %d\n", recv);
	if (!recv) {
		ao_radio_put();
		ao_radio_recv_abort();
		return AO_RADIO_CMAC_TIMEOUT;
	}

	PRINTD ("fetching data\n");
	ao_radio_get_data(packet, len);
	recv = ao_radio_spi_reply.status;
	ao_radio_cmac_rssi = ao_radio_spi_reply.rssi;
	ao_radio_put();
	PRINTD ("data fetched: %d %d\n", recv, ao_radio_cmac_rssi);
	return recv;
}

static uint8_t	ao_radio_test_on;

void
ao_radio_test(uint8_t on)
{
	if (on) {
		if (!ao_radio_test_on) {
			ao_radio_get(AO_RADIO_SPI_TEST_ON, 0);
			ao_radio_test_on = 1;
			ao_radio_master_send();
		}
	} else {
		if (ao_radio_test_on) {
			ao_radio_spi_request.len = AO_RADIO_SPI_REQUEST_HEADER_LEN;
			ao_radio_spi_request.request = AO_RADIO_SPI_TEST_OFF;
			ao_radio_master_send();
			ao_radio_test_on = 0;
			ao_radio_put();
		}
	}
}

static void
ao_radio_test_cmd(void)
{
	uint8_t	mode = 2;
	ao_cmd_white();
	if (ao_cmd_lex_c != '\n') {
		ao_cmd_decimal();
		mode = (uint8_t) ao_cmd_lex_u32;
	}
	mode++;
	if ((mode & 2))
		ao_radio_test(1);
	if (mode == 3) {
		printf ("Hit a character to stop..."); flush();
		getchar();
		putchar('\n');
	}
	if ((mode & 1))
		ao_radio_test(0);
}

__code struct ao_cmds ao_radio_cmds[] = {
	{ ao_radio_test_cmd,	"C <1 start, 0 stop, none both>\0Radio carrier test" },
	{ 0,	NULL },
};

void
ao_radio_init(void)
{
	ao_spi_init_cs(AO_RADIO_CS_PORT, (1 << AO_RADIO_CS_PIN));

	ao_enable_port(AO_RADIO_INT_PORT);
	ao_exti_setup(AO_RADIO_INT_PORT,
		      AO_RADIO_INT_PIN,
		      AO_EXTI_MODE_RISING|AO_EXTI_MODE_FALLING,
		      ao_radio_isr);
	ao_exti_enable(AO_RADIO_INT_PORT, AO_RADIO_INT_PIN);
	ao_cmd_register(&ao_radio_cmds[0]);
}
