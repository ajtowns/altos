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

struct ao_soft_sym {
	uint8_t	a, b;
};

#define NUM_STATE	8
#define NUM_HIST	8
#define MOD_HIST(b)	((b) & 7)

static const struct ao_soft_sym ao_fec_decode_table[NUM_STATE][2] = {
/* next        0              1	         state */
	{ { 0x00, 0x00 }, { 0xff, 0xff } } ,	/* 000 */
	{ { 0x00, 0xff }, { 0xff, 0x00 } },	/* 001 */
	{ { 0xff, 0xff }, { 0x00, 0x00 } },	/* 010 */
	{ { 0xff, 0x00 }, { 0x00, 0xff } },	/* 011 */
	{ { 0xff, 0xff }, { 0x00, 0x00 } },	/* 100 */
	{ { 0xff, 0x00 }, { 0x00, 0xff } },	/* 101 */
	{ { 0x00, 0x00 }, { 0xff, 0xff } },	/* 110 */
	{ { 0x00, 0xff }, { 0xff, 0x00 } }	/* 111 */
};

static inline uint8_t
ao_next_state(uint8_t state, uint8_t bit)
{
	return ((state << 1) | bit) & 0x7;
}

static inline uint16_t ao_abs(int16_t x) { return x < 0 ? -x : x; }

static inline uint16_t
ao_cost(struct ao_soft_sym a, struct ao_soft_sym b)
{
	return ao_abs(a.a - b.a) + ao_abs(a.b - b.b);
}

/*
 * 'in' is 8-bits per symbol soft decision data
 * 'len' is input byte length. 'out' must be
 * 'len'/16 bytes long
 */

uint8_t
ao_fec_decode(uint8_t *in, uint16_t len, uint8_t *out)
{
	static uint16_t	cost[2][NUM_STATE];		/* path cost */
	static uint16_t	bits[2][NUM_STATE];		/* save bits to quickly output them */
	uint16_t	i;				/* input byte index */
	uint16_t	b;				/* encoded symbol index (bytes/2) */
	uint16_t	o;				/* output bit index */
	uint8_t		p;				/* previous cost/bits index */
	uint8_t		n;				/* next cost/bits index */
	uint8_t		state;				/* state index */
	uint8_t		bit;				/* original encoded bit index */

	p = 0;
	for (state = 0; state < NUM_STATE; state++) {
		cost[0][state] = 0xffff;
		bits[0][state] = 0;
	}
	cost[0][0] = 0;

	o = 0;
	for (i = 0; i < len; i += 2) {
		b = i/2;
		n = p ^ 1;
		struct ao_soft_sym s = { .a = in[i], .b = in[i+1] };

		/* Reset next costs to 'impossibly high' values so that
		 * the first path through this state is cheaper than this
		 */
		for (state = 0; state < NUM_STATE; state++)
			cost[n][state] = 0xffff;

		/* Compute path costs and accumulate output bit path
		 * for each state and encoded bit value
		 */
		for (state = 0; state < NUM_STATE; state++) {
			for (bit = 0; bit < 2; bit++) {
				int	bit_cost = cost[p][state] + ao_cost(s, ao_fec_decode_table[state][bit]);
				uint8_t	bit_state = ao_next_state(state, bit);

				/* Only track the minimal cost to reach
				 * this state; the best path can never
				 * go through the higher cost paths as
				 * total path cost is cumulative
				 */
				if (bit_cost < cost[n][bit_state]) {
					cost[n][bit_state] = bit_cost;
					bits[n][bit_state] = (bits[p][state] << 1) | (state & 1);
				}
			}
		}

#if 0
		printf ("bit %3d symbol %2x %2x:", i/2, s.a, s.b);
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
			uint16_t	min_cost;		/* lowest cost */
			uint8_t		min_state;		/* lowest cost state */

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
			printf ("\tbit %3d min_cost %5d old bit %3d old_state %x bits %02x\n",
				i/2, min_cost, o + 8, min_state, (bits[p][min_state] >> dist) & 0xff);
#endif
			out[o >> 3] = bits[p][min_state] >> dist;
			o += 8;
		}
	}
	return len/16;
}
