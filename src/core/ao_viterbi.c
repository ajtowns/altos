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

struct ao_soft_sym {
	uint8_t	a, b;
};

static const struct ao_soft_sym ao_fec_decode_table[16] = {
/* next        0              1	         state */
	{ 0x00, 0x00 }, { 0xff, 0xff },	/* 000 */
	{ 0x00, 0xff }, { 0xff, 0x00 },	/* 001 */
	{ 0xff, 0xff }, { 0x00, 0x00 },	/* 010 */
	{ 0xff, 0x00 }, { 0x00, 0xff },	/* 011 */
	{ 0xff, 0xff }, { 0x00, 0x00 },	/* 100 */
	{ 0xff, 0x00 }, { 0x00, 0xff },	/* 101 */
	{ 0x00, 0x00 }, { 0xff, 0xff },	/* 110 */
	{ 0x00, 0xff }, { 0xff, 0x00 }	/* 111 */
};

struct ao_soft_sym
ao_soft_sym(uint8_t bits)
{
	struct ao_soft_sym	s;

	s.a = ((bits & 2) >> 1) * 0xff;
	s.b = (bits & 1) * 0xff;
	return s;
}

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

#define NUM_STATE	8

uint8_t
ao_fec_decode(uint8_t *in, int len, uint8_t *out)
{
	uint16_t	cost[2][NUM_STATE], min_cost;
	uint8_t		prev[len/2 + 1][NUM_STATE];
	uint16_t	prev_bits[2][NUM_STATE];
	int		i, b, dump;
	uint8_t		p, n;
	uint8_t		state = 0, min_state;

	p = 0;
	for (state = 0; state < NUM_STATE; state++) {
		cost[0][state] = 0xffff;
		prev_bits[0][state] = 0;
	}
	cost[0][0] = 0;

	min_state = 0;
	min_cost = 0;
	dump = 0;
	for (i = 0; i < len; i += 2) {
		b = i/2;
		n = p ^ 1;
		struct ao_soft_sym s = { .a = in[i], .b = in[i+1] };

		for (state = 0; state < NUM_STATE; state++)
			cost[n][state] = 0xffff;

		for (state = 0; state < NUM_STATE; state++) {
			int	zero_cost = ao_cost(s, ao_fec_decode_table[state * 2 + 0]);
			int	one_cost = ao_cost(s, ao_fec_decode_table[state * 2 + 1]);

			uint8_t	zero_state = ao_next_state(state, 0);
			uint8_t	one_state = ao_next_state(state, 1);

			zero_cost += cost[p][state];
			one_cost += cost[p][state];
			if (zero_cost < cost[n][zero_state]) {
				prev[b+1][zero_state] = state;
				cost[n][zero_state] = zero_cost;
				prev_bits[n][zero_state] = (prev_bits[p][state] << 1) | (state & 1);
			}

			if (one_cost < cost[n][one_state]) {
				prev[b+1][one_state] = state;
				cost[n][one_state] = one_cost;
				prev_bits[n][one_state] = (prev_bits[p][state] << 1) | (state & 1);
			}
		}

#if 0
		printf ("bit %3d symbol %2x %2x:", i/2, s.a, s.b);
		for (state = 0; state < NUM_STATE; state++) {
			printf (" %5d(%04x)", cost[n][state], prev_bits[n][state]);
		}
		printf ("\n");
#endif
		p = n;

		b++;
		if ((b - dump) > 16 || i + 2 >= len) {
			uint8_t	dist = b - (dump + 9);
			uint8_t	rev;
			min_cost = cost[p][0];
			min_state = 0;
			for (state = 1; state < NUM_STATE; state++) {
				if (cost[p][state] < min_cost) {
					min_cost = cost[p][state];
					min_state = state;
				}
			}
			for (rev = 0; rev < dist; rev++) {
				min_state = prev[b][min_state];
				b--;
			}
#if 0
			printf ("\tbit %3d min_cost %5d old bit %3d old_state %x bits %02x\n",
				i/2, min_cost, b-1, min_state, (prev_bits[p][min_state] >> dist) & 0xff);
#endif
			out[dump/8] = prev_bits[p][min_state] >> dist;
			dump = b - 1;
		}

	}
	return len/16;
}
