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

static inline uint16_t ao_interleave_index(uint16_t i) {
	uint8_t		l = i & 0x1e;
	uint16_t	h = i & ~0x1e;
	uint8_t		o = 0x1e ^ (((l >> 2) & 0x6) | ((l << 2) & 0x18));
	return h | o;
}

struct ao_soft_sym {
	uint8_t	a, b;
};

#define NUM_STATE	8
#define NUM_HIST	8
#define MOD_HIST(b)	((b) & 7)

#define V_0		0xc0
#define V_1		0x40

static const struct ao_soft_sym ao_fec_decode_table[NUM_STATE][2] = {
/* next        0              1	         state */
	{ { V_0, V_0 }, { V_1, V_1 } } ,	/* 000 */
	{ { V_0, V_1 }, { V_1, V_0 } },	/* 001 */
	{ { V_1, V_1 }, { V_0, V_0 } },	/* 010 */
	{ { V_1, V_0 }, { V_0, V_1 } },	/* 011 */
	{ { V_1, V_1 }, { V_0, V_0 } },	/* 100 */
	{ { V_1, V_0 }, { V_0, V_1 } },	/* 101 */
	{ { V_0, V_0 }, { V_1, V_1 } },	/* 110 */
	{ { V_0, V_1 }, { V_1, V_0 } }	/* 111 */
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
ao_fec_decode(uint8_t *in, uint16_t len, uint8_t *out, uint8_t out_len)
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
	const uint8_t	*whiten = ao_fec_whiten_table;
	uint16_t	interleave;			/* input byte array index */
	struct ao_soft_sym	s;			/* input symbol pair */

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

		/* Fetch one pair of input bytes, de-interleaving
		 * the input.
		 */
		interleave = ao_interleave_index(i);
		s.a = in[interleave];
		s.b = in[interleave+1];

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
			printf ("\tbit %3d min_cost %5d old bit %3d old_state %x bits %02x whiten %0x\n",
				i/2, min_cost, o + 8, min_state, (bits[p][min_state] >> dist) & 0xff, *whiten);
#endif
			if (out_len) {
				*out++ = (bits[p][min_state] >> dist) ^ *whiten++;
				--out_len;
			}
			o += 8;
		}
	}
	return len/16;
}
