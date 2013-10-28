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

#ifndef _AO_QUATERNION_H_
#define _AO_QUATERNION_H_

#include <math.h>

struct ao_quaternion {
	float	r;		/* real bit */
	float	x, y, z;	/* imaginary bits */
};

static inline void ao_quaternion_multiply(struct ao_quaternion *r,
					  const struct ao_quaternion *a,
					  const struct ao_quaternion *b)
{
	struct ao_quaternion	t;
#define T(_a,_b)	(((a)->_a) * ((b)->_b))

/*
 * Quaternions
 *
 *	ii = jj = kk = ijk = -1;
 *
 *	kji = 1;
 *
 * 	ij = k;		ji = -k;
 *	kj = -i;	jk = i;
 *	ik = -j;	ki = j;
 *
 * Multiplication p * q:
 *
 *	(pr + ipx + jpy + kpz) (qr + iqx + jqy + kqz) =
 *
 *		( pr * qr +  pr * iqx +  pr * jqy +  pr * kqz) +
 *		(ipx * qr + ipx * iqx + ipx * jqy + ipx * kqz) +
 *		(jpy * qr + jpy * iqx + jpy * jqy + jpy * kqz) +
 *		(kpz * qr + kpz * iqx + kpz * jqy + kpz * kqz) =
 *
 *
 *		 (pr * qr) + i(pr * qx) + j(pr * qy) + k(pr * qz) +
 *		i(px * qr) -  (px * qx) + k(px * qy) - j(px * qz) +
 *		j(py * qr) - k(py * qx) -  (py * qy) + i(py * qz) +
 *		k(pz * qr) + j(pz * qx) - i(pz * qy) -  (pz * qz) =
 *
 *		1 * ( (pr * qr) - (px * qx) - (py * qy) - (pz * qz) ) +
 *		i * ( (pr * qx) + (px * qr) + (py * qz) - (pz * qy) ) +
 *		j * ( (pr * qy) - (px * qz) + (py * qr) + (pz * qx) ) +
 *		k * ( (pr * qz) + (px * qy) - (py * qx) + (pz * qr);
 */

	t.r = T(r,r) - T(x,x) - T(y,y) - T(z,z);
	t.x = T(r,x) + T(x,r) + T(y,z) - T(z,y);
	t.y = T(r,y) - T(x,z) + T(y,r) + T(z,x);
	t.z = T(r,z) + T(x,y) - T(y,x) + T(z,r);
#undef T
	*r = t;
}

static inline void ao_quaternion_conjugate(struct ao_quaternion *r,
					   const struct ao_quaternion *a)
{
	r->r = a->r;
	r->x = -a->x;
	r->y = -a->y;
	r->z = -a->z;
}

static inline float ao_quaternion_normal(const struct ao_quaternion *a)
{
#define S(_a)	(((a)->_a) * ((a)->_a))
	return S(r) + S(x) + S(y) + S(z);
#undef S
}

static inline void ao_quaternion_scale(struct ao_quaternion *r,
				       const struct ao_quaternion *a,
				       float b)
{
	r->r = a->r * b;
	r->x = a->x * b;
	r->y = a->y * b;
	r->z = a->z * b;
}

static inline void ao_quaternion_normalize(struct ao_quaternion *r,
					   const struct ao_quaternion *a)
{
	float	n = ao_quaternion_normal(a);

	if (n > 0)
		ao_quaternion_scale(r, a, 1/sqrtf(n));
	else
		*r = *a;
}

static inline void ao_quaternion_rotate(struct ao_quaternion *r,
					struct ao_quaternion *a,
					struct ao_quaternion *b)
{
	struct ao_quaternion	c;
	struct ao_quaternion	t;

	ao_quaternion_conjugate(&c, b);
	ao_quaternion_multiply(&t, b, a);
	ao_quaternion_multiply(r, &t, &c);
}

static inline void ao_quaternion_init_vector(struct ao_quaternion *r,
					     float x, float y, float z)
{
	r->r = 0;
	r->x = x;
	r->y = y;
	r->z = z;
}

static inline void ao_quaternion_init_rotation(struct ao_quaternion *r,
					       float x, float y, float z,
					       float s, float c)
{
	r->r = c;
	r->x = s * x;
	r->y = s * y;
	r->z = s * z;
}

static inline void ao_quaternion_init_zero_rotation(struct ao_quaternion *r)
{
	r->r = 1;
	r->x = r->y = r->z = 0;
}

#endif /* _AO_QUATERNION_H_ */
