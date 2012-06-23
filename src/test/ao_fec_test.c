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
#include <string.h>

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
#define DECODE_LEN(input_len)		((input_len) + AO_FEC_PREPARE_EXTRA)
#define EXPAND_LEN(input_len)		(ENCODE_LEN(input_len) * 8)

static int
ao_expand(uint8_t *bits, int bits_len, uint8_t *bytes)
{
	int	i, bit;
	uint8_t	b;

	for (i = 0; i < bits_len; i++) {
		b = bits[i];
		for (bit = 7; bit >= 0; bit--)
			*bytes++ = ((b >> bit) & 1) * 0xff;
	}

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
	return errors;
}

static uint8_t
ao_random_data(uint8_t	*out, uint8_t out_len)
{
	uint8_t	len = random() % (out_len + 1);
	uint8_t	i;
	
	for (i = 0; i < len; i++)
		out[i] = random();
	return len;
}	


int
main(int argc, char **argv)
{
	int		trial;

	uint8_t		original[120];
	uint8_t		original_len;

	uint8_t		encode[ENCODE_LEN(sizeof(original))];
	int		encode_len;

	uint8_t		transmit[EXPAND_LEN(sizeof(original))];
	int		transmit_len;

	uint8_t		receive[EXPAND_LEN(sizeof(original))];
	int		receive_len, receive_errors;

	uint8_t		decode[DECODE_LEN(sizeof(original))];
	int		decode_len;

	int		errors = 0;
	int		error;

	srandom(0);
	for (trial = 0; trial < 10000; trial++) {

		/* Compute some random data */
		original_len = ao_random_data(original, sizeof(original));

		/* Encode it */
		encode_len = ao_fec_encode(original, original_len, encode);

		/* Expand from 1-bit-per-symbol to 1-byte-per-symbol */
		transmit_len = ao_expand(encode, encode_len, transmit);

		/* Add gaussian noise to the signal */
		receive_errors = ao_fuzz(transmit, transmit_len, receive, 0x30);
		receive_len = transmit_len;
		
		/* Decode it */
		decode_len = ao_fec_decode(receive, receive_len, decode);

		/* Check to see if we received the right data */
		error = 0;

		if (decode_len < original_len + 2) {
			printf ("len mis-match\n");
			error++;
		}

		if (!ao_fec_check_crc(decode, original_len)) {
			printf ("crc mis-match\n");
			error++;
		}

		if (memcmp(original, decode, original_len) != 0) {
			printf ("data mis-match\n");
			error++;
		}
		if (error) {
			printf ("Errors: %d\n", receive_errors);
			ao_fec_dump_bytes(original, original_len, "Input");
			ao_fec_dump_bytes(decode, original_len, "Decode");
			errors += error;
		}
	}
	return errors;
}


