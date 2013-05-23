/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#define __data
#define __pdata
#define __xdata
#define __reentrant

#include <ao_int64.h>
#include <ao_int64.c>
#include <stdio.h>
#include <stdlib.h>

int	errors;

#define test_o(op,func,mod,a,b,ao_a,ao_b) do {				\
		r = (a) op (b);						\
		func(&ao_r, ao_a, ao_b);				\
		c = ao_cast64(&ao_r);					\
		if (c != r) {						\
			printf ("trial %4d: %lld " #func mod " %lld = %lld (should be %lld)\n", \
				trial, (int64_t) (a), (int64_t) b, c, r); \
			++errors;					\
		}							\
	} while (0)

#define test(op,func,a,b,ao_a,ao_b) test_o(op,func,"",a,b,ao_a,ao_b)

#define test_a(op,func,a,b,ao_a,ao_b) do {	\
		ao_r = *ao_a;			\
		test_o(op,func,"_a",a,b,&ao_r,ao_b);	\
	} while (0)

#define test_b(op,func,a,b,ao_a,ao_b) do {	\
		ao_r = *ao_b;			\
		test_o(op,func,"_b",a,b,ao_a,&ao_r);	\
	} while (0)

#define test_x(op,func,a,b,ao_a,ao_b) do {	\
		ao_r = *ao_a;			\
		test_o(op,func,"_xa",a,a,&ao_r,&ao_r);	\
		ao_r = *ao_b;			\
		test_o(op,func,"_xb",b,b,&ao_r,&ao_r);	\
	} while (0)

void
do_test(int trial, int64_t a, int64_t b)
{
	int64_t	r, c;
	ao_int64_t	ao_a, ao_b, ao_r;

	ao_int64_init64(&ao_a, a >> 32, a);
	ao_int64_init64(&ao_b, b >> 32, b);

	test(+, ao_plus64, a, b, &ao_a, &ao_b);
	test_a(+, ao_plus64, a, b, &ao_a, &ao_b);
	test_b(+, ao_plus64, a, b, &ao_a, &ao_b);
	test_x(+, ao_plus64, a, b, &ao_a, &ao_b);
	test(-, ao_minus64, a, b, &ao_a, &ao_b);
	test_a(-, ao_minus64, a, b, &ao_a, &ao_b);
	test_b(-, ao_minus64, a, b, &ao_a, &ao_b);
	test_x(-, ao_minus64, a, b, &ao_a, &ao_b);
	test(*, ao_mul64_32_32,(int64_t) (int32_t) a, (int32_t) b, (int32_t) a, (int32_t) b);
	test(*, ao_mul64, a, b, &ao_a, &ao_b);
	test_a(*, ao_mul64, a, b, &ao_a, &ao_b);
	test_b(*, ao_mul64, a, b, &ao_a, &ao_b);
	test_x(*, ao_mul64, a, b, &ao_a, &ao_b);
	test(*, ao_mul64_64_16, a, (uint16_t) b, &ao_a, (uint16_t) b);
	test_a(*, ao_mul64_64_16, a, (uint16_t) b, &ao_a, (uint16_t) b);
	test(>>, ao_rshift64, a, (uint8_t) b & 0x3f, &ao_a, (uint8_t) b & 0x3f);
	test_a(>>, ao_rshift64, a, (uint8_t) b & 0x3f, &ao_a, (uint8_t) b & 0x3f);
	test(<<, ao_lshift64, a, (uint8_t) b & 0x3f, &ao_a, (uint8_t) b & 0x3f);
	test_a(<<, ao_lshift64, a, (uint8_t) b & 0x3f, &ao_a, (uint8_t) b & 0x3f);
}

#define TESTS	10000000

static int64_t
random64(void)
{
	return (int64_t) random() + ((int64_t) random() << 31) /* + ((int64_t) random() << 33) */;
}

int
main (int argc, char **argv)
{
	int	i, start;

	if (argv[1])
		start = atoi(argv[1]);
	else
		start = 0;
	srandom(1000);
	for (i = 0; i < TESTS; i++) {
		int64_t a = random64();
		int64_t b = random64();
		if (i >= start)
			do_test(i, a, b);
	}
	return errors;
}
