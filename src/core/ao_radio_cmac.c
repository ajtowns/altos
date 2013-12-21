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

#include <ao.h>
#include <ao_radio_cmac.h>

static __xdata uint8_t ao_radio_cmac_mutex;
__pdata int8_t ao_radio_cmac_rssi;
static __xdata uint8_t cmac_data[AO_CMAC_MAX_LEN + AO_CMAC_KEY_LEN + 2 + AO_CMAC_KEY_LEN];

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
	i = ao_radio_recv(cmac_data, len + AO_CMAC_KEY_LEN + 2, timeout);

	if (!i) {
		ao_radio_cmac_rssi = 0;
		return AO_RADIO_CMAC_TIMEOUT;
	}

	ao_radio_cmac_rssi = ao_radio_rssi;
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
	int8_t	i;
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

