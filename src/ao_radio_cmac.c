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

#define AO_CMAC_KEY_LEN		16
#define AO_CMAC_MAX_LEN		(128 - AO_CMAC_KEY_LEN)

static __xdata uint8_t cmac_key[AO_CMAC_KEY_LEN];
static __xdata uint8_t cmac_data[AO_CMAC_MAX_LEN + AO_CMAC_KEY_LEN + 2 + AO_CMAC_KEY_LEN];
static __pdata uint8_t ao_radio_cmac_len;

static uint8_t
getnibble(void)
{
	__pdata char	c;

	c = getchar();
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - ('a' - 10);
	if ('A' <= c && c <= 'F')
		return c - ('A' - 10);
	ao_cmd_status = ao_cmd_lex_error;
	return 0;
}

static uint8_t
getbyte(void)
{
	uint8_t	b;
	b = getnibble() << 4;
	b |= getnibble();
	return b;
}
	
static void
ao_radio_cmac_key(void) __reentrant
{
	uint8_t	i;

	for (i = 0; i < AO_CMAC_KEY_LEN; i++) {
		cmac_key[i] = getbyte();
		if (ao_cmd_status != ao_cmd_success)
			return;
	}
}

static void
ao_radio_cmac_send(void) __reentrant
{
	uint8_t	i;
	uint8_t	len;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i < AO_CMAC_KEY_LEN ||
	    ao_cmd_lex_i > AO_CMAC_MAX_LEN ||
	    ao_cmd_lex_i % AO_CMAC_KEY_LEN != 0)
	{
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	flush();
	len = ao_cmd_lex_i;
	for (i = 0; i < len; i++) {
		cmac_data[i] = getbyte();
		if (ao_cmd_status != ao_cmd_success)
			return;
	}
	ao_mutex_get(&ao_aes_mutex);
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(cmac_key);
	ao_aes_zero_iv();
	for (i = 0; i < len; i += AO_CMAC_KEY_LEN) {
		if (i + AO_CMAC_KEY_LEN < len)
			ao_aes_run(&cmac_data[i], NULL);
		else
			ao_aes_run(&cmac_data[i], &cmac_data[len]);
	}
	ao_mutex_put(&ao_aes_mutex);
#if HAS_MONITOR
	ao_set_monitor(0);
#endif
	printf("send:");
	for (i = 0; i < len + AO_CMAC_KEY_LEN; i++)
		printf(" %02x", cmac_data[i]);
	printf("\n"); flush();
	ao_radio_send(cmac_data, len + AO_CMAC_KEY_LEN);
}

static void
ao_radio_cmac_recv(void) __reentrant
{
	uint8_t		len, i;
	uint16_t	timeout;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i < AO_CMAC_KEY_LEN ||
	    ao_cmd_lex_i > AO_CMAC_MAX_LEN ||
	    ao_cmd_lex_i % AO_CMAC_KEY_LEN != 0)
	{
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	len = ao_cmd_lex_i;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	timeout = ao_cmd_lex_i;
#if HAS_MONITOR
	ao_set_monitor(0);
#endif
	if (timeout)
		ao_alarm(timeout);
	if (!ao_radio_recv(cmac_data, len + AO_CMAC_KEY_LEN + 2)) {
		printf("timeout\n");
		return;
	}
	ao_mutex_get(&ao_aes_mutex);
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(cmac_key);
	ao_aes_zero_iv();
	for (i = 0; i < len; i += AO_CMAC_KEY_LEN) {
		if (i + AO_CMAC_KEY_LEN < len)
			ao_aes_run(&cmac_data[i], NULL);
		else
			ao_aes_run(&cmac_data[i], &cmac_data[len + AO_CMAC_KEY_LEN + 2]);
	}
	printf ("PACKET ");
	for (i = 0; i < len + AO_CMAC_KEY_LEN + 2 + AO_CMAC_KEY_LEN; i++)
		printf("%02x", cmac_data[i]);
	printf ("\n");
}

static __code struct ao_cmds ao_radio_cmac_cmds[] = {
	{ ao_radio_cmac_key,	"k\0Set AES-CMAC key. 16 key bytes follow on next line" },
	{ ao_radio_cmac_send,	"s <length>\0Send AES-CMAC packet. Bytes to send follow on next line" },
	{ ao_radio_cmac_recv,	"S <length> <timeout>\0Receive AES-CMAC packet. Timeout in ms" },
	{ 0, NULL },
};

void
ao_radio_cmac_init(void)
{
	ao_cmd_register(&ao_radio_cmac_cmds[0]);
}
