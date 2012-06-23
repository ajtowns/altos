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
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef RANDOM_MAX
#define RANDOM_MAX 0x7fffffff
#endif

static double
rand_real(void) {
	return (double) random() / (double) RANDOM_MAX;
}

static double
gaussian_random(double mean, double dev)
{
	static int	save_x_valid = 0;
	static double	save_x;
	double		x;

	if (save_x_valid)
	{
		x = save_x;
		save_x_valid = 0;
	}
	else
	{
		double    w;
		double    normal_x1, normal_x2;

		do {
			normal_x1 = 2 * rand_real () - 1;
			normal_x2 = 2 * rand_real () - 1;
			w = normal_x1*normal_x1 + normal_x2*normal_x2;
		} while (w >= 1 || w < 1E-30);

		w = sqrt(log(w)*(-2./w));

		/*
		 * normal_x1 and normal_x2 are independent normally
		 * distributed variates
		 */

		x = normal_x1 * w;
		/* save normal_x2 for next call */
		save_x = normal_x2 * w;
		save_x_valid = 1;
	}
	return x * dev + mean;
}

#define PREPARE_LEN(input_len)		((input_len) + AO_FEC_PREPARE_EXTRA)
#define ENCODE_LEN(input_len)		(PREPARE_LEN(input_len) * 2)
#define INTERLEAVE_LEN(input_len)	ENCODE_LEN(input_len)

static int
ao_encode(uint8_t *input, int input_len, uint8_t *output)
{
	uint8_t		prepare[PREPARE_LEN(input_len)];
	uint8_t		encode[ENCODE_LEN(input_len)];
	uint8_t		interleave[INTERLEAVE_LEN(input_len)];
	uint8_t		prepare_len;
	uint8_t		encode_len;
	uint8_t		interleave_len;

	ao_fec_dump_bytes(input, input_len, "Input");

	prepare_len = ao_fec_prepare(input, input_len, prepare);

	ao_fec_dump_bytes(prepare, prepare_len, "Prepare");
	
	encode_len = ao_fec_encode(prepare, prepare_len, encode);

	ao_fec_dump_bytes(encode, encode_len, "Encode");
	
	interleave_len = ao_fec_interleave(encode, encode_len, output);

	ao_fec_dump_bytes(output, interleave_len, "Interleave");

	return interleave_len;
}

#define RADIO_LEN(input_len)	(INTERLEAVE_LEN(input_len) * 8)

static int
ao_radio(uint8_t *bits, int bits_len, uint8_t *bytes)
{
	uint8_t	b, *bytes_orig = bytes;
	uint8_t	interleave[bits_len];
	int	i, bit;

	ao_fec_interleave(bits, bits_len, interleave);

	ao_fec_dump_bytes(interleave, bits_len, "De-interleave");

	for (i = 0; i < bits_len; i++) {
		b = interleave[i];
		for (bit = 7; bit >= 0; bit--)
			*bytes++ = ((b >> bit) & 1) * 0xff;
	}

	ao_fec_dump_bytes(bytes_orig, bits_len * 8, "Bytes");

	return bits_len * 8;
}

static int
ao_fuzz (uint8_t *in, int in_len, uint8_t *out, double dev)
{
	int	i;
	int	errors = 0;
	
	for (i = 0; i < in_len; i++) {
		double	error = gaussian_random(0, dev);
		uint8_t	byte = in[i];

		if (error > 0) {
			if (error > 0xff)
				error = 0xff;
			if (error >= 0x80)
				errors++;
			if (byte < 0x80)
				byte += error;
			else
				byte -= error;
		}
		out[i] = byte;
	}

	printf ("Introduced %d errors\n", errors);
	ao_fec_dump_bytes(out, in_len, "Fuzz");
	return in_len;
}

static int
ao_decode(uint8_t *bytes, int bytes_len, uint8_t *bits)
{
	int	bits_len;

	bits_len = ao_fec_decode(bytes, bytes_len, bits);

	ao_fec_dump_bytes(bits, bits_len, "Decode");
}

int
main(int argc, char **argv)
{
	uint8_t		original[4] = { 3, 1, 2, 3 };
	uint8_t		encode[INTERLEAVE_LEN(sizeof(original))];
	int		encode_len;

	uint8_t		transmit[RADIO_LEN(sizeof(original))];
	int		transmit_len;

	uint8_t		receive[RADIO_LEN(sizeof(original))];
	int		receive_len;

	uint8_t		decode[INTERLEAVE_LEN(sizeof(original))];
	int		decode_len;

	encode_len = ao_encode(original, sizeof(original), encode);

	transmit_len = ao_radio(encode, encode_len, transmit);

	/* apply gaussian noise to test viterbi code against errors */
	receive_len = ao_fuzz(transmit, transmit_len, receive, 0x80);

	decode_len = ao_decode(receive, receive_len, decode);
}


