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

#define AO_CMAC_KEY_LEN		AO_AES_LEN
#define AO_CMAC_MAX_LEN		(128 - AO_CMAC_KEY_LEN)

static __xdata uint8_t ao_radio_cmac_mutex;
__pdata int16_t ao_radio_cmac_rssi;
static __xdata uint8_t cmac_data[AO_CMAC_MAX_LEN + AO_CMAC_KEY_LEN + 2 + AO_CMAC_KEY_LEN];
static __pdata uint8_t ao_radio_cmac_len;

static uint8_t
getnibble(void)
{
	int8_t	b;

	b = ao_cmd_hexchar(getchar());
	if (b < 0) {
		ao_cmd_status = ao_cmd_lex_error;
		return 0;
	}
	return (uint8_t) b;
}

static uint8_t
getbyte(void)
{
	uint8_t	b;
	b = getnibble() << 4;
	b |= getnibble();
	return b;
}
	
static uint8_t
round_len(uint8_t len)
{
	uint8_t	rem;

	/* Make sure we transfer at least one packet, and
	 * then make sure every packet is full. Note that
	 * there is no length encoded, and that the receiver
	 * must deal with any extra bytes in the packet
	 */
	if (len < AO_CMAC_KEY_LEN)
		len = AO_CMAC_KEY_LEN;
	rem = len % AO_CMAC_KEY_LEN;
	if (rem != 0)
		len += (AO_CMAC_KEY_LEN - rem);
	return len;
}

/*
 * Sign and deliver the data sitting in the cmac buffer
 */
static void
radio_cmac_send(uint8_t len) __reentrant
{
	uint8_t	i;

	len = round_len(len);
	/* Make sure the AES key is loaded */
	ao_config_get();

#if HAS_MONITOR
	ao_monitor_set(0);
#endif

	ao_mutex_get(&ao_aes_mutex);
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(ao_config.aes_key);
	ao_aes_zero_iv();
	for (i = 0; i < len; i += AO_CMAC_KEY_LEN) {
		if (i + AO_CMAC_KEY_LEN < len)
			ao_aes_run(&cmac_data[i], NULL);
		else
			ao_aes_run(&cmac_data[i], &cmac_data[len]);
	}
	ao_mutex_put(&ao_aes_mutex);

	ao_radio_send(cmac_data, len + AO_CMAC_KEY_LEN);
}

/*
 * Receive and validate an incoming packet
 */

static int8_t
radio_cmac_recv(uint8_t len, uint16_t timeout) __reentrant
{
	uint8_t	i;

	len = round_len(len);
#if HAS_MONITOR
	ao_monitor_set(0);
#endif
	if (timeout)
		ao_alarm(timeout);

	i = ao_radio_recv(cmac_data, len + AO_CMAC_KEY_LEN + 2);
	ao_clear_alarm();

	if (!i) {
		ao_radio_cmac_rssi = 0;
		return AO_RADIO_CMAC_TIMEOUT;
	}

	ao_radio_cmac_rssi = (int16_t) (((int8_t) cmac_data[len + AO_CMAC_KEY_LEN]) >> 1) - 74;
	if (!(cmac_data[len + AO_CMAC_KEY_LEN +1] & AO_RADIO_STATUS_CRC_OK))
		return AO_RADIO_CMAC_CRC_ERROR;

	ao_config_get();

	/* Compute the packet signature
	 */
	ao_mutex_get(&ao_aes_mutex);
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(ao_config.aes_key);
	ao_aes_zero_iv();
	for (i = 0; i < len; i += AO_CMAC_KEY_LEN) {
		if (i + AO_CMAC_KEY_LEN < len)
			ao_aes_run(&cmac_data[i], NULL);
		else
			ao_aes_run(&cmac_data[i], &cmac_data[len + AO_CMAC_KEY_LEN + 2]);
	}
	ao_mutex_put(&ao_aes_mutex);

	/* Check the packet signature against the signature provided
	 * over the link
	 */
	 
	if (memcmp(&cmac_data[len],
		   &cmac_data[len + AO_CMAC_KEY_LEN + 2],
		   AO_CMAC_KEY_LEN) != 0) {
		return AO_RADIO_CMAC_MAC_ERROR;
	}

	return AO_RADIO_CMAC_OK;
}

int8_t
ao_radio_cmac_send(__xdata void *packet, uint8_t len) __reentrant
{
	if (len > AO_CMAC_MAX_LEN)
		return AO_RADIO_CMAC_LEN_ERROR;
	ao_mutex_get(&ao_radio_cmac_mutex);
	ao_xmemcpy(cmac_data, packet, len);
#if AO_LED_TX
	ao_led_on(AO_LED_TX);
#endif
	radio_cmac_send(len);
#if AO_LED_TX
	ao_led_off(AO_LED_TX);
#endif
	ao_mutex_put(&ao_radio_cmac_mutex);
	return AO_RADIO_CMAC_OK;
}

int8_t
ao_radio_cmac_recv(__xdata void *packet, uint8_t len, uint16_t timeout) __reentrant
{
	uint8_t	i;
	if (len > AO_CMAC_MAX_LEN)
		return AO_RADIO_CMAC_LEN_ERROR;
	ao_mutex_get(&ao_radio_cmac_mutex);
#if AO_LED_RX
	ao_led_on(AO_LED_RX);
#endif
	i = radio_cmac_recv(len, timeout);
#if AO_LED_RX
	ao_led_off(AO_LED_RX);
#endif
	if (i == AO_RADIO_CMAC_OK)
		ao_xmemcpy(packet, cmac_data, len);
	ao_mutex_put(&ao_radio_cmac_mutex);
	return i;
}

static void
radio_cmac_send_cmd(void) __reentrant
{
	uint8_t	i;
	uint8_t	len;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	len = ao_cmd_lex_i;
	if (len > AO_CMAC_MAX_LEN) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	flush();
	ao_mutex_get(&ao_radio_cmac_mutex);
	len = ao_cmd_lex_i;
	for (i = 0; i < len; i++) {
		cmac_data[i] = getbyte();
		if (ao_cmd_status != ao_cmd_success)
			return;
	}
	radio_cmac_send(len);
	ao_mutex_put(&ao_radio_cmac_mutex);
}

static void
radio_cmac_recv_cmd(void) __reentrant
{
	uint8_t		len, i;
	uint16_t	timeout;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	len = ao_cmd_lex_i;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	timeout = AO_MS_TO_TICKS(ao_cmd_lex_i);
	ao_mutex_get(&ao_radio_cmac_mutex);
	i = radio_cmac_recv(len, timeout);
	if (i == AO_RADIO_CMAC_OK) {
		printf ("PACKET ");
		for (i = 0; i < len; i++)
			printf("%02x", cmac_data[i]);
		printf (" %d\n", ao_radio_cmac_rssi);
	} else
		printf ("ERROR %d %d\n", i, ao_radio_cmac_rssi);
	ao_mutex_put(&ao_radio_cmac_mutex);
}

static __xdata struct ao_launch_command	command;
static __xdata struct ao_launch_query	query;
static __pdata uint16_t	launch_serial;
static __pdata uint8_t	launch_channel;
static __pdata uint16_t	tick_offset;

static void
launch_args(void) __reentrant
{
	ao_cmd_decimal();
	launch_serial = ao_cmd_lex_i;
	ao_cmd_decimal();
	launch_channel = ao_cmd_lex_i;
}

static int8_t
launch_query(void)
{
	uint8_t	i;
	int8_t	r = AO_RADIO_CMAC_OK;

	tick_offset = ao_time();
	for (i = 0; i < 10; i++) {
		printf ("."); flush();
		command.tick = ao_time();
		command.serial = launch_serial;
		command.cmd = AO_LAUNCH_QUERY;
		command.channel = launch_channel;
		ao_radio_cmac_send(&command, sizeof (command));
		r = ao_radio_cmac_recv(&query, sizeof (query), AO_MS_TO_TICKS(500));
		if (r == AO_RADIO_CMAC_OK)
			break;
	}
	tick_offset -= query.tick;
	printf("\n"); flush();
	return r;
}

static void
launch_report_cmd(void) __reentrant
{
	int8_t		r;

	launch_args();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = launch_query();
	switch (r) {
	case AO_RADIO_CMAC_OK:
		if (query.valid) {
			switch (query.arm_status) {
			case ao_igniter_ready:
			case ao_igniter_active:
				printf ("Armed: ");
				break;
			default:
				printf("Disarmed: ");
			}
			switch (query.igniter_status) {
			default:
				printf("unknown\n");
				break;
			case ao_igniter_ready:
				printf("igniter good\n");
				break;
			case ao_igniter_open:
				printf("igniter bad\n");
				break;
			}
		} else {
			printf("Invalid channel %d\n", launch_channel);
		}
		printf("Rssi: %d\n", ao_radio_cmac_rssi);
		break;
	default:
		printf("Error %d\n", r);
		break;
	}
}

static void
launch_arm(void) __reentrant
{
	command.tick = ao_time() - tick_offset;
	command.serial = launch_serial;
	command.cmd = AO_LAUNCH_ARM;
	command.channel = launch_channel;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_ignite(void) __reentrant
{
	command.tick = ao_time() - tick_offset;
	command.serial = launch_serial;
	command.cmd = AO_LAUNCH_FIRE;
	command.channel = 0;
	ao_radio_cmac_send(&command, sizeof (command));
}

static void
launch_fire_cmd(void) __reentrant
{
	static __xdata struct ao_launch_command	command;
	uint8_t		secs;
	uint8_t		i;
	int8_t		r;

	launch_args();
	ao_cmd_decimal();
	secs = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = launch_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}

	for (i = 0; i < 4; i++) {
		printf("arm %d\n", i); flush();
		launch_arm();
	}

	secs = secs * 10 - 5;
	if (secs > 100)
		secs = 100;
	for (i = 0; i < secs; i++) {
		printf("fire %d\n", i); flush();
		launch_ignite();
		ao_delay(AO_MS_TO_TICKS(100));
	}
}

static void
launch_arm_cmd(void) __reentrant
{
	uint8_t	i;
	int8_t  r;
	launch_args();
	r = launch_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}
	for (i = 0; i < 4; i++)
		launch_arm();
}

static void
launch_ignite_cmd(void) __reentrant
{
	uint8_t i;
	launch_args();
	for (i = 0; i < 4; i++)
		launch_ignite();
}

static __code struct ao_cmds ao_radio_cmac_cmds[] = {
	{ radio_cmac_send_cmd,	"s <length>\0Send AES-CMAC packet. Bytes to send follow on next line" },
	{ radio_cmac_recv_cmd,	"S <length> <timeout>\0Receive AES-CMAC packet. Timeout in ms" },
	{ launch_report_cmd,    "l <serial> <channel>\0Get remote launch status" },
	{ launch_fire_cmd,	"f <serial> <channel> <secs>\0Fire remote igniter" },
	{ launch_arm_cmd,	"a <serial> <channel>\0Arm remote igniter" },
	{ launch_ignite_cmd,	"i <serial> <channel>\0Pulse remote igniter" },
	{ 0, NULL },
};

void
ao_radio_cmac_init(void)
{
	ao_cmd_register(&ao_radio_cmac_cmds[0]);
}
