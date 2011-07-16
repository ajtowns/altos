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

static void
ao_radio_cmac_key(void) __reentrant
{
	uint8_t	i;

	for (i = 0; i < AO_CMAC_KEY_LEN; i++) {
		ao_cmd_hex();
		if (ao_cmd_status != ao_cmd_success)
			return;
		cmac_key[i] = ao_cmd_lex_i;
	}
}

static void
ao_radio_cmac_send(void) __reentrant
{
	uint8_t	i, b;

	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ao_cmd_lex_i > AO_CMAC_MAX_LEN || ao_cmd_lex_i % AO_CMAC_KEY_LEN != 0) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	for (i = 0; i < ao_cmd_lex_i; i++) {
		b = getnibble() << 4;
		b |= getnibble();
		if (ao_cmd_status != ao_cmd_success)
			return;
		cmac_data[i] = b;
	}
	ao_mutex_get(&ao_aes_mutex);
	ao_aes_set_mode(ao_aes_mode_cbc_mac);
	ao_aes_set_key(cmac_key);
	for (i = 0; i < ao_cmd_lex_i; i += AO_CMAC_KEY_LEN) {
		if (i + AO_CMAC_KEY_LEN < ao_cmd_lex_i)
			ao_aes_run(&cmac_data[i], NULL);
		else
			ao_aes_run(&cmac_data[i], &cmac_data[ao_cmd_lex_i]);
	}
	ao_mutex_put(&ao_aes_mutex);
	ao_set_monitor(0);
	ao_radio_send(cmac_data, ao_cmd_lex_i + AO_CMAC_KEY_LEN);
}

__code struct ao_cmds ao_radio_cmac_cmds[] = {
	{ ao_radio_cmac_key,	"k <byte> ...\0Set AES-CMAC key." },
	{ ao_radio_cmac_send,	"s <length>\0Send AES-CMAC packet. Bytes to send follow on next line" },
	{ ao_radio_cmac_recv,	"S <length> <timeout>\0Receive AES-CMAC packet. Timeout in ms" },
	{ 0, NULL },
};

void
ao_radio_cmac_init(void)
{
	ao_cmd_register(&ao_radio_cmac_cmds[0]);
	
}
