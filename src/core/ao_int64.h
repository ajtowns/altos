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

#ifndef _AO_INT64_H_
#define _AO_INT64_H_

#include <stdint.h>

typedef struct {
	uint32_t	high;
	uint32_t	low;
} ao_int64_t;

void ao_plus64(ao_int64_t *r, ao_int64_t *a, ao_int64_t *b);
void ao_neg64(ao_int64_t *r, ao_int64_t *a);
void ao_lshift64_16(ao_int64_t *r, uint16_t a, uint8_t d);
void ao_rshift64(ao_int64_t *r, ao_int64_t *a, uint8_t d);
void ao_lshift64(ao_int64_t *r, ao_int64_t *a, uint8_t d);
void ao_mul64_64_64(ao_int64_t *r, ao_int64_t *a, ao_int64_t *b);
void ao_mul64_32_32(ao_int64_t *r, int32_t a, int32_t b);
void ao_mul64_64_16(ao_int64_t *r, ao_int64_t *a, uint16_t b);

#define ao_int64_init32(r, a) (((r)->high = 0), (r)->low = (a))
#define ao_int64_init64(r, a, b) (((r)->high = (a)), (r)->low = (b))

#define ao_cast64(a) (((int64_t) (a)->high << 32) | (a)->low)

#define ao_int64_negativep(a)	(((int32_t) (a)->high) < 0)

#endif /* _AO_INT64_H_ */
