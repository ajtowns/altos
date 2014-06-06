/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_distance.h>

static uint32_t
ao_dist(int32_t a, int32_t b)
{
	int32_t	d = a - b;
	if (d < 0)
		d = -d;

	return (uint32_t) ((int64_t) d * 111198 / 10000000);
}

static uint32_t
ao_lat_dist(int32_t lat_a, int32_t lat_b)
{
	return ao_dist(lat_a, lat_b);
}

static const uint8_t cos_table[] = {
   0, /*  0 */
   0, /*  1 */
   0, /*  2 */
 255, /*  3 */
 254, /*  4 */
 253, /*  5 */
 252, /*  6 */
 251, /*  7 */
 249, /*  8 */
 247, /*  9 */
 245, /* 10 */
 243, /* 11 */
 240, /* 12 */
 238, /* 13 */
 235, /* 14 */
 232, /* 15 */
 228, /* 16 */
 225, /* 17 */
 221, /* 18 */
 217, /* 19 */
 213, /* 20 */
 209, /* 21 */
 205, /* 22 */
 200, /* 23 */
 195, /* 24 */
 190, /* 25 */
 185, /* 26 */
 180, /* 27 */
 175, /* 28 */
 169, /* 29 */
 163, /* 30 */
 158, /* 31 */
 152, /* 32 */
 145, /* 33 */
 139, /* 34 */
 133, /* 35 */
 126, /* 36 */
 120, /* 37 */
 113, /* 38 */
 106, /* 39 */
 100, /* 40 */
  93, /* 41 */
  86, /* 42 */
  79, /* 43 */
  71, /* 44 */
  64, /* 45 */
  57, /* 46 */
  49, /* 47 */
  42, /* 48 */
  35, /* 49 */
  27, /* 50 */
  20, /* 51 */
  12, /* 52 */
   5, /* 53 */
   1, /* 54 */
};

static uint32_t
ao_lon_dist(int32_t lon_a, int32_t lon_b)
{
	uint8_t		c = cos_table[lon_a >> 24];
	uint32_t	lon_dist;

	/* check if it's shorter to go the other way around */
	if ((lon_a >> 1) < (lon_b >> 1) - (1800000000 >> 1))
		lon_a += 3600000000;
	lon_dist = ao_dist(lon_a, lon_b);
	if (c) {
		if (lon_dist & 0x7f800000)
			lon_dist = (lon_dist >> 8) * c;
		else
			lon_dist = (lon_dist * (int16_t) c) >> 8;
	}
	return lon_dist;
}

static uint32_t sqr(uint32_t x) { return x * x; }

uint32_t
ao_distance(int32_t lat_a, int32_t lon_a, int32_t lat_b, int32_t lon_b)
{
	uint32_t	lat_dist = ao_lat_dist(lat_a, lat_b);
	uint32_t	lon_dist = ao_lon_dist(lon_a, lon_b);

	return ao_sqrt (sqr(lat_dist) + sqr(lon_dist));
}
