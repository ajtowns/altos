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

#if AO_FEC_DEBUG
void
ao_fec_dump_bytes(const uint8_t *bytes, uint16_t len, const char *name)
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
#endif

uint16_t
ao_fec_crc(const uint8_t *bytes, uint8_t len)
{
	uint16_t	crc = AO_FEC_CRC_INIT;

	while (len--)
		crc = ao_fec_crc_byte(*bytes++, crc);
	return crc;
}

/*
 * len is the length of the data; the crc will be
 * the fist two bytes after that
 */

uint8_t
ao_fec_check_crc(const uint8_t *bytes, uint8_t len)
{
	uint16_t	computed_crc = ao_fec_crc(bytes, len);
	uint16_t	received_crc = (bytes[len] << 8) | (bytes[len+1]);

	return computed_crc == received_crc;
}

/*
 * Compute CRC and trellis-terminator/interleave-pad bytes
 */
static uint8_t
ao_fec_prepare(const uint8_t *in, uint8_t len, uint8_t *extra)
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
ao_fec_encode(const uint8_t *in, uint8_t len, uint8_t *out)
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
