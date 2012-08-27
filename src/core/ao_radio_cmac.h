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

#ifndef _AO_RADIO_CMAC_H_
#define _AO_RADIO_CMAC_H_

#include <ao_aes.h>

#define AO_CMAC_KEY_LEN		AO_AES_LEN
#define AO_CMAC_MAX_LEN		(128 - AO_CMAC_KEY_LEN)

extern __pdata int8_t ao_radio_cmac_rssi;

int8_t
ao_radio_cmac_send(__xdata void *packet, uint8_t len) __reentrant;

#define AO_RADIO_CMAC_OK	0
#define AO_RADIO_CMAC_LEN_ERROR	-1
#define AO_RADIO_CMAC_CRC_ERROR	-2
#define AO_RADIO_CMAC_MAC_ERROR	-3
#define AO_RADIO_CMAC_TIMEOUT	-4

int8_t
ao_radio_cmac_recv(__xdata void *packet, uint8_t len, uint16_t timeout) __reentrant;

void
ao_radio_cmac_init(void);

#endif /* _AO_RADIO_CMAC_H_ */
