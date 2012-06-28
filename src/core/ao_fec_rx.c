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

#ifdef MEGAMETRUM
#include <ao.h>
#endif

#if AO_PROFILE
#include <ao_profile.h>

uint32_t	ao_fec_decode_start, ao_fec_decode_end;
#endif

/* 
 * byte order repeats through 3 2 1 0
 * 	
 * bit-pair order repeats through
 *
 *  1/0 3/2 5/4 7/6
 *
 * So, the over all order is:
 *
 *	3,1/0 	2,1/0	1,1/0	0,1/0
 *	3,3/2 	2,3/2	1,3/2	0,3/2
 *	3,5/4	2,5/4	1,5/4	0,5/4
 *	3,7/6	2,7/6	1,7/6	0,7/6
 *
 * The raw bit order is thus
 *
 *	1e/1f   16/17   0e/0f   06/07
 *	1c/1d	14/15	0c/0d	04/05
 *	1a/1b	12/13	0a/0b	02/03
 *	18/19	10/11	08/09	00/01
 */

static const uint8_t ao_interleave_order[] = {
	0x1e, 0x16, 0x0e, 0x06,
	0x1c, 0x14, 0x0c, 0x04,
	0x1a, 0x12, 0x0a, 0x02,
	0x18, 0x10, 0x08, 0x00
};

static inline uint16_t ao_interleave_index(uint16_t i) {
	return (i & ~0x1e) | ao_interleave_order[(i & 0x1e) >> 1];
}

#define NUM_STATE	8
#define NUM_HIST	8

#define V_0		0xff
#define V_1		0x00

/*
 * These are just the 'zero' states; the 'one' states mirror them
 */
static const uint8_t ao_fec_decode_table[NUM_STATE*2] = {
	V_0, V_0,	/* 000 */
	V_0, V_1,	/* 001 */
	V_1, V_1,	/* 010 */
	V_1, V_0,	/* 011 */
	V_1, V_1,	/* 100 */
	V_1, V_0,	/* 101 */
	V_0, V_0,	/* 110 */
	V_0, V_1	/* 111 */
};

static inline uint8_t
ao_next_state(uint8_t state, uint8_t bit)
{
	return ((state << 1) | bit) & 0x7;
}

/*
 * 'in' is 8-bits per symbol soft decision data
 * 'len' is input byte length. 'out' must be
 * 'len'/16 bytes long
 */

uint8_t
ao_fec_decode(const uint8_t *in, uint16_t len, uint8_t *out, uint8_t out_len, uint16_t (*callback)())
{
	static uint32_t	cost[2][NUM_STATE];		/* path cost */
	static uint16_t	bits[2][NUM_STATE];		/* save bits to quickly output them */

	uint16_t	i;				/* input byte index */
	uint16_t	b;				/* encoded symbol index (bytes/2) */
	uint16_t	o;				/* output bit index */
	uint8_t		p;				/* previous cost/bits index */
	uint8_t		n;				/* next cost/bits index */
	uint8_t		state;				/* state index */
	const uint8_t	*whiten = ao_fec_whiten_table;
	uint16_t	interleave;			/* input byte array index */
	uint8_t		s0, s1;
	uint16_t	avail;
	uint16_t	crc = AO_FEC_CRC_INIT;
#if AO_PROFILE
	uint32_t	start_tick;
#endif

	p = 0;
	for (state = 0; state < NUM_STATE; state++) {
		cost[0][state] = 0x7fffffff;
		bits[0][state] = 0;
	}
	cost[0][0] = 0;

	if (callback)
		avail = 0;
	else
		avail = len;

#if AO_PROFILE
	if (!avail) {
		avail = callback();
		if (!avail)
			return 0;
	}
	start_tick = ao_profile_tick();
#endif
	o = 0;
	for (i = 0; i < len; i += 2) {
		b = i/2;
		n = p ^ 1;

		if (!avail) {
			avail = callback();
			if (!avail)
				return 0;
		}

		/* Fetch one pair of input bytes, de-interleaving
		 * the input.
		 */
		interleave = ao_interleave_index(i);
		s0 = in[interleave];
		s1 = in[interleave+1];

		avail -= 2;

		/* Reset next costs to 'impossibly high' values so that
		 * the first path through this state is cheaper than this
		 */
		for (state = 0; state < NUM_STATE; state++)
			cost[n][state] = 0x7fffffff;

		/* Compute path costs and accumulate output bit path
		 * for each state and encoded bit value. Unrolling
		 * this loop is worth about > 30% performance boost.
		 * Decoding 76-byte remote access packets is reduced
		 * from 14.700ms to 9.3ms
		 */
#define DO_STATE(state) {						\
			uint32_t	bitcost = ((uint32_t) (s0 ^ ao_fec_decode_table[(state<<1)]) + \
						   (uint32_t) (s1 ^ ao_fec_decode_table[(state<<1)+1])); \
			{						\
				uint32_t	cost0 = cost[p][state] + bitcost; \
				uint8_t		state0 = ao_next_state(state, 0); \
									\
				if (cost0 < cost[n][state0]) {		\
					cost[n][state0] = cost0;	\
					bits[n][state0] = (bits[p][state] << 1) | (state & 1); \
				}					\
			}						\
			{						\
				uint32_t	cost1 = cost[p][state] + 510 - bitcost; \
				uint8_t		state1 = ao_next_state(state, 1); \
									\
				if (cost1 < cost[n][state1]) {		\
					cost[n][state1] = cost1;	\
					bits[n][state1] = (bits[p][state] << 1) | (state & 1); \
				}					\
			}						\
		}

		DO_STATE(0);
		DO_STATE(1);
		DO_STATE(2);
		DO_STATE(3);
		DO_STATE(4);
		DO_STATE(5);
		DO_STATE(6);
		DO_STATE(7);

#if 0
		printf ("bit %3d symbol %2x %2x:", i/2, s0, s1);
		for (state = 0; state < NUM_STATE; state++) {
			printf (" %5d(%04x)", cost[n][state], bits[n][state]);
		}
		printf ("\n");
#endif
		p = n;

		/* A loop is needed to handle the last output byte. It
		 * won't have any bits of future data to perform full
		 * error correction, but we might as well give the
		 * best possible answer anyways.
		 */
		while ((b - o) >= (8 + NUM_HIST) || (i + 2 >= len && b > o)) {

			/* Compute number of bits to the end of the
			 * last full byte of data. This is generally
			 * NUM_HIST, unless we've reached
			 * the end of the input, in which case
			 * it will be seven.
			 */
			int8_t		dist = b - (o + 8);	/* distance to last ready-for-writing bit */
			uint32_t	min_cost;		/* lowest cost */
			uint8_t		min_state;		/* lowest cost state */
			uint8_t		byte;

			/* Find the best fit at the current point
			 * of the decode.
			 */
			min_cost = cost[p][0];
			min_state = 0;
			for (state = 1; state < NUM_STATE; state++) {
				if (cost[p][state] < min_cost) {
					min_cost = cost[p][state];
					min_state = state;
				}
			}

			/* The very last byte of data has the very last bit
			 * of data left in the state value; just smash the
			 * bits value in place and reset the 'dist' from
			 * -1 to 0 so that the full byte is read out
			 */
			if (dist < 0) {
				bits[p][min_state] = (bits[p][min_state] << 1) | (min_state & 1);
				dist = 0;
			}

#if 0
			printf ("\tbit %3d min_cost %5d old bit %3d old_state %x bits %02x whiten %0x\n",
				i/2, min_cost, o + 8, min_state, (bits[p][min_state] >> dist) & 0xff, *whiten);
#endif
			byte = (bits[p][min_state] >> dist) ^ *whiten++;
			*out++ = byte;
			if (out_len > 2)
				crc = ao_fec_crc_byte(byte, crc);

			if (!--out_len) {
				if ((out[-2] == (uint8_t) (crc >> 8)) &&
				    out[-1] == (uint8_t) crc)
					out[-1] = AO_FEC_DECODE_CRC_OK;
				else
					out[-1] = 0;
				out[-2] = 0;
				goto done;
			}
			o += 8;
		}
	}
done:
#if AO_PROFILE
	ao_fec_decode_start = start_tick;
	ao_fec_decode_end = ao_profile_tick();
#endif
	return 1;
}
