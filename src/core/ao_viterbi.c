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
 * 'input' is 8-bits per symbol soft decision data
 * 'len' is output byte length
 */

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

struct ao_soft_sym {
	uint8_t	a, b;
};

struct ao_soft_sym
ao_soft_sym(uint8_t bits)
{
	struct ao_soft_sym	s;

	s.a = ((bits & 2) >> 1) * 0xff;
	s.b = (bits & 1) * 0xff;
	return s;
}

uint8_t
ao_next_state(uint8_t state, uint8_t bit)
{
	return ((state << 1) | bit) & 0x7;
}

static inline abs(int x) { return x < 0 ? -x : x; }

int
ao_cost(struct ao_soft_sym a, struct ao_soft_sym b)
{
	return abs(a.a - b.a) + abs(a.b - b.b);
}

uint8_t
ao_fec_decode(uint8_t *in, int len, uint8_t *out)
{
	int	cost[len/2 + 1][8];
	uint8_t	prev[len/2 + 1][8];
	int	c;
	int	i, b;
	uint8_t	state = 0, min_state;
	uint8_t	bits[len/2];

	for (c = 0; c < 8; c++)
		cost[0][c] = 10000;
	cost[0][0] = 0;

	for (i = 0; i < len; i += 2) {
		b = i/2;
		struct ao_soft_sym s = { .a = in[i], .b = in[i+1] };

		for (state = 0; state < 8; state++)
			cost[b+1][state] = 10000;

		for (state = 0; state < 8; state++) {
			struct ao_soft_sym zero = ao_soft_sym(ao_fec_encode_table[state * 2 + 0]);
			struct ao_soft_sym one = ao_soft_sym(ao_fec_encode_table[state * 2 + 1]);
			uint8_t	zero_state = ao_next_state(state, 0);
			uint8_t	one_state = ao_next_state(state, 1);
			int	zero_cost = ao_cost(s, zero);
			int	one_cost = ao_cost(s, one);

#if 0
			printf ("saw %02x %02x expected %02x %02x (%d) or %02x %02x (%d)\n",
				s.a, s.b, zero.a, zero.b, zero_cost, one.a, one.b, one_cost);
#endif
			zero_cost += cost[b][state];
			one_cost += cost[b][state];
			if (zero_cost < cost[b+1][zero_state]) {
				prev[b+1][zero_state] = state;
				cost[b+1][zero_state] = zero_cost;
			}

			if (one_cost < cost[b+1][one_state]) {
				prev[b+1][one_state] = state;
				cost[b+1][one_state] = one_cost;
			}
		}

		printf ("bit %3d symbol %2x %2x:", i/2, s.a, s.b);
		for (state = 0; state < 8; state++) {
			printf (" %5d", cost[b+1][state]);
		}
		printf ("\n");
	}

	b = len / 2;
	c = cost[b][0];
	min_state = 0;
	for (state = 1; state < 8; state++) {
		if (cost[b][state] < c) {
			c = cost[b][state];
			min_state = state;
		}
	}

	for (b = len/2; b > 0; b--) {
		bits[b-1] = min_state & 1;
		min_state = prev[b][min_state];
	}

	for (i = 0; i < len/2; i += 8) {
		uint8_t	byte;

		byte = 0;
		for (b = 0; b < 8; b++)
			byte = (byte << 1) | bits[i + b];
		out[i/8] = byte;
	}
	return len/16;
}
