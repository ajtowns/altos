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

#define __xdata
#define __data
#define __pdata
#define __reentrant

#include <stdint.h>
#include <ao_ms5607.h>

struct ao_ms5607_prom ao_ms5607_prom = {
	0x002c,
	0xa6e0,
	0x988e,
	0x6814,
	0x5eff,
	0x8468,
	0x6c86,
	0xa271,
};

int32_t D1_mm = 6179630;
int32_t D2_mm = 8933155;

#include <ao_ms5607_convert.c>
#define ao_ms5607_convert ao_ms5607_convert_8051
#include <ao_ms5607_convert_8051.c>
#include <ao_int64.c>
#include <stdio.h>
#include <stdlib.h>

struct ao_ms5607_sample ao_sample = {
	6179630,
	8933155
};

int errors;

void test(int trial, struct ao_ms5607_sample *sample)
{
	struct ao_ms5607_value	value, value_8051;

	ao_ms5607_convert(sample, &value);
	ao_ms5607_convert_8051(sample, &value_8051);
	if (value.temp != value_8051.temp || value.pres != value_8051.pres) {
		++errors;
		printf ("trial %d: %d, %d -> %d, %d (should be %d, %d)\n",
			trial,
			sample->pres, sample->temp,
			value_8051.pres, value_8051.temp,
			value.pres, value.temp);
	}
}

#define TESTS	10000000

#include <stdlib.h>

static int32_t rand24(void) { return random() & 0xffffff; }

int
main(int argc, char **argv)
{
	struct ao_ms5607_sample sample;
	int	i, start;

	if (argv[1])
		start = atoi(argv[1]);
	else
		start = 0;

	srandom(10000);
	test(-1, &ao_sample);
	for (i = 0; i < TESTS; i++) {
		sample.pres = rand24();
		sample.temp = rand24();
		if (i >= start)
			test(i, &sample);
	}
	return errors;
}
