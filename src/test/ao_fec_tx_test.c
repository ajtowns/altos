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

int
main(int argc, char **argv)
{
	uint8_t		input[4] = { 3, 1, 2, 3 };
	uint8_t		prepare[sizeof(input) + AO_FEC_PREPARE_EXTRA];
	uint8_t		encode[sizeof(prepare) * 2];
	uint8_t		interleave[sizeof(encode)];
	uint8_t		prepare_len;
	uint8_t		encode_len;
	uint8_t		interleave_len;

	ao_fec_dump_bytes(input, sizeof(input), "Input");

	prepare_len = ao_fec_prepare(input, sizeof (input), prepare);

	ao_fec_dump_bytes(prepare, prepare_len, "Prepare");
	
	encode_len = ao_fec_encode(prepare, prepare_len, encode);

	ao_fec_dump_bytes(encode, encode_len, "Encode");
	
	interleave_len = ao_fec_interleave(encode, encode_len, interleave);

	ao_fec_dump_bytes(interleave, interleave_len, "Interleave");
}

