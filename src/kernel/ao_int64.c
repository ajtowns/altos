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

#include <ao_int64.h>

void ao_plus64(__pdata ao_int64_t *r, __pdata ao_int64_t *a, __pdata ao_int64_t *b) __FATTR {
	__LOCAL uint32_t	t;

	r->high = a->high + b->high;
	t = a->low + b->low;
	if (t < a->low)
		r->high++;
	r->low = t;
}

void ao_minus64(__pdata ao_int64_t *r, __pdata ao_int64_t *a, __pdata ao_int64_t *b) __FATTR {
	__LOCAL uint32_t	t;

	r->high = a->high - b->high;
	t = a->low - b->low;
	if (t > a->low)
		r->high--;
	r->low = t;
}

void ao_rshift64(__pdata ao_int64_t *r, __pdata ao_int64_t *a, uint8_t d) __FATTR {
	if (d < 32) {
		r->low = a->low >> d;
		if (d)
			r->low |= a->high << (32 - d);
		r->high = (int32_t) a->high >> d;
	} else {
		d &= 0x1f;
		r->low = (int32_t) a->high >> d;
		r->high = 0;
	}
}

void ao_lshift64(__pdata ao_int64_t *r, __pdata ao_int64_t *a, uint8_t d) __FATTR {
	if (d < 32) {
		r->high = a->high << d;
		if (d)
			r->high |= a->low >> (32 - d);
		r->low = a->low << d;
	} else {
		d &= 0x1f;
		r->high = a->low << d;
		r->low = 0;
	}
}

static void ao_umul64_32_32(__ARG ao_int64_t *r, uint32_t a, uint32_t b) __reentrant {
	__LOCAL uint32_t	s;
	__LOCAL ao_int64_t	t;
	r->low = (uint32_t) (uint16_t) a * (uint16_t) b;
	r->high = (uint32_t) (uint16_t) (a >> 16) * (uint16_t) (b >> 16);

	s = (uint32_t) (uint16_t) (a >> 16) * (uint16_t) b;

	t.high = s >> 16;
	t.low = s << 16;
	ao_plus64(r, r, &t);

	s = (uint32_t) (uint16_t) a * (uint16_t) (b >> 16);

	t.high = s >> 16;
	t.low = s << 16;
	ao_plus64(r, r, &t);
}

void ao_neg64(__pdata ao_int64_t *r, __pdata ao_int64_t *a) __FATTR {
	r->high = ~a->high;
	if (!(r->low = ~a->low + 1))
		r->high++;
}

void ao_mul64_32_32(__ARG ao_int64_t *r, int32_t a, int32_t b) __FATTR {
	uint8_t		negative = 0;

	if (a < 0) {
		a = -a;
		++negative;
	}
	if (b < 0) {
		b = -b;
		--negative;
	}
	ao_umul64_32_32(r, a, b);
	if (negative)
		ao_neg64(r, r);
}

static void ao_umul64(__ARG ao_int64_t *r, __ARG ao_int64_t *a, __ARG ao_int64_t *b) __reentrant {
	__LOCAL ao_int64_t	r2, r3;

	ao_umul64_32_32(&r2, a->high, b->low);
	ao_umul64_32_32(&r3, a->low, b->high);
	ao_umul64_32_32(r, a->low, b->low);

	r->high += r2.low + r3.low;
}

static __ARG ao_int64_t	ap, bp;

void ao_mul64(__ARG ao_int64_t *r, __ARG ao_int64_t *a, __ARG ao_int64_t *b) __FATTR {
	uint8_t	negative = 0;

	if (ao_int64_negativep(a)) {
		ao_neg64(&ap, a);
		a = &ap;
		++negative;
	}
	if (ao_int64_negativep(b)) {
		ao_neg64(&bp, b);
		b = &bp;
		--negative;
	}
	ao_umul64(r, a, b);
	if (negative)
		ao_neg64(r, r);
}

static void ao_umul64_64_16(__ARG ao_int64_t *r, __ARG ao_int64_t *a, uint16_t b) __reentrant {
	__LOCAL uint32_t h;

	h = a->high * b;
	ao_umul64_32_32(r, a->low, b);
	r->high += h;
}

void ao_mul64_64_16(__ARG ao_int64_t *r, __ARG ao_int64_t *a, __ARG uint16_t b) __FATTR {
	uint8_t		negative = 0;

	if ((int32_t) a->high < 0) {
		ao_neg64(&ap, a);
		a = &ap;
		negative++;
	}
	ao_umul64_64_16(r, a, b);
	if (negative)
		ao_neg64(r, r);
}
