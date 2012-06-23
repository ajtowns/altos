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

#include <ao_fec.h>
#include <stdio.h>

void
ao_fec_dump_bytes(uint8_t *bytes, uint16_t len, char *name)
{
	uint16_t	i;

	printf ("%s (%d):", name, len);
	for (i = 0; i < len; i++) {
		if ((i & 7) == 0)
			printf ("\n\t%02x:", i);
		printf(" %02x", bytes[i]);
	}
	printf ("\n");
}

static uint16_t inline
crc_byte(uint8_t byte, uint16_t crc)
{
	uint8_t	bit;

	for (bit = 0; bit < 8; bit++) {
		if (((crc & 0x8000) >> 8) ^ (byte & 0x80))
			crc = (crc << 1) ^ 0x8005;
		else
			crc = (crc << 1);
		byte <<= 1;
	}
	return crc;
}

uint16_t
ao_fec_crc(uint8_t *bytes, uint8_t len)
{
	uint16_t	crc = AO_FEC_CRC_INIT;

	while (len--)
		crc = crc_byte(*bytes++, crc);
	return crc;
}

/*
 * len is the length of the data; the crc will be
 * the fist two bytes after that
 */

uint8_t
ao_fec_check_crc(uint8_t *bytes, uint8_t len)
{
	uint16_t	computed_crc = ao_fec_crc(bytes, len);
	uint16_t	received_crc = (bytes[len] << 8) | (bytes[len+1]);

	return computed_crc == received_crc;
}

uint8_t
ao_fec_prepare(uint8_t *in, uint8_t len, uint8_t *extra)
{
	uint16_t	crc = ao_fec_crc (in, len);
	uint8_t		i = 0;
	uint8_t		num_fec;

	/* Append CRC */
	extra[i++] = crc >> 8;
	extra[i++] = crc;

	/* Append FEC -- 1 byte if odd, two bytes if even */
	num_fec = 2 - (i & 1);
	while (num_fec--)
		extra[i++] = AO_FEC_TRELLIS_TERMINATOR;
	return i;
}

const uint8_t ao_fec_whiten_table[] = {
#include "ao_whiten.h"
};

#if 0
void
ao_fec_whiten(uint8_t *in, uint8_t len, uint8_t *out)
{
	const uint8_t	*w = ao_fec_whiten_table;

	while (len--)
		*out++ = *in++ ^ *w++;
}

/*
 * Unused as interleaving is now built in to ao_fec_encode
 */

static void
ao_fec_interleave(uint8_t *d, uint8_t len)
{
	uint8_t	i, j;

	for (i = 0; i < len; i += 4) {
		uint32_t	interleaved = 0;

		for (j = 0; j < 4 * 4; j++) {
			interleaved <<= 2;
			interleaved |= (d[i + (~j & 0x3)] >> (2 * ((j & 0xc) >> 2))) & 0x03;
		}
		d[i+0] = interleaved >> 24;
		d[i+1] = interleaved >> 16;
		d[i+2] = interleaved >> 8;
		d[i+3] = interleaved;
	}
}
#endif

static const uint8_t ao_fec_encode_table[16] = {
/* next 0  1	  state */
	0, 3,	/* 000 */
	1, 2,	/* 001 */
	3, 0,	/* 010 */
	2, 1,	/* 011 */
	3, 0,	/* 100 */
	2, 1,	/* 101 */
	0, 3,	/* 110 */
	1, 2	/* 111 */
};

uint8_t
ao_fec_encode(uint8_t *in, uint8_t len, uint8_t *out)
{
	uint8_t		extra[AO_FEC_PREPARE_EXTRA];
	uint8_t 	extra_len;
	uint32_t	encode, interleave;
	uint8_t		pair, byte, bit;
	uint16_t	fec = 0;
	const uint8_t	*whiten = ao_fec_whiten_table;

	extra_len = ao_fec_prepare(in, len, extra);
	for (pair = 0; pair < len + extra_len; pair += 2) {
		encode = 0;
		for (byte = 0; byte < 2; byte++) {
			if (pair + byte == len)
				in = extra;
			fec |= *in++ ^ *whiten++;
			for (bit = 0; bit < 8; bit++) {
				encode = encode << 2 | ao_fec_encode_table[fec >> 7];
				fec = (fec << 1) & 0x7ff;
			}
		}

		interleave = 0;
		for (bit = 0; bit < 4 * 4; bit++) {
			uint8_t	byte_shift = (bit & 0x3) << 3;
			uint8_t	bit_shift = (bit & 0xc) >> 1;

			interleave = (interleave << 2) | ((encode >> (byte_shift + bit_shift)) & 0x3);
		}
		*out++ = interleave >> 24;
		*out++ = interleave >> 16;
		*out++ = interleave >> 8;
		*out++ = interleave >> 0;
	}
	return (len + extra_len) * 2;
}
