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

#ifndef _AO_RADIO_SPI_H_
#define _AO_RADIO_SPI_H_

#define AO_RADIO_SPI_RECV	0
#define AO_RADIO_SPI_RECV_ABORT	1
#define AO_RADIO_SPI_RECV_FETCH	2
#define AO_RADIO_SPI_SEND	3

#define AO_RADIO_SPI_CMAC_KEY	4
#define AO_RADIO_SPI_CMAC_RECV	5
#define AO_RADIO_SPI_CMAC_SEND	6

#define AO_RADIO_SPI_TEST_ON	7
#define AO_RADIO_SPI_TEST_OFF	8

#define AO_RADIO_SPI_MAX_PAYLOAD	128

struct ao_radio_spi_request {
	uint8_t		len;		/* required to be first by cc1111 DMA engine */
	uint8_t		request;
	uint8_t		recv_len;
	uint8_t		pad;
	uint32_t	setting;
	uint16_t	timeout;
	uint8_t		payload[AO_RADIO_SPI_MAX_PAYLOAD];
};

#define AO_RADIO_SPI_REQUEST_HEADER_LEN	(sizeof (struct ao_radio_spi_request) - AO_RADIO_SPI_MAX_PAYLOAD)

struct ao_radio_spi_reply {
	uint8_t		status;
	int8_t		rssi;
	uint8_t		payload[AO_RADIO_SPI_MAX_PAYLOAD];
};

#define AO_RADIO_SPI_REPLY_HEADER_LEN	(sizeof (struct ao_radio_spi_reply) - AO_RADIO_SPI_MAX_PAYLOAD)

void
ao_radio_slave_init(void);

#endif /* _AO_RADIO_SPI_H_ */
