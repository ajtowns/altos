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

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "ao_quaternion.h"

#if 0
static void
print_q(char *name, struct ao_quaternion *q)
{
	printf ("%8.8s: r%8.5f x%8.5f y%8.5f z%8.5f ", name,
		q->r, q->x, q->y, q->z);
}
#endif

#define STEPS	16

#define DEG	(1.0f * 3.1415926535f / 180.0f)

struct ao_rotation {
	int	steps;
	float	x, y, z;
};

static struct ao_rotation ao_path[] = {
	{ .steps = 45, .x =  2*DEG, .y = 0, .z = 0 },

	{ .steps = 45, .x = 0, .y = 2*DEG, .z = 0 },
	
	{ .steps = 45, .x = -2*DEG, .y = 0, .z = 0 },

	{ .steps = 45, .x = 0, .y = -2*DEG, .z = 0 },
};

#define NUM_PATH	(sizeof ao_path / sizeof ao_path[0])

static int close(float a, float b) {
	return fabsf (a - b) < 1e-5;
}

static int check_quaternion(char *where, struct ao_quaternion *got, struct ao_quaternion *expect) {
	if (!close (got->r, expect->r) ||
	    !close (got->x, expect->x) ||
	    !close (got->y, expect->y) ||
	    !close (got->z, expect->z))
	{
		printf ("%s: got r%8.5f x%8.5f y%8.5f z%8.5f expect r%8.5f x%8.5f y%8.5f z%8.5f\n",
			where,
			got->r, got->x, got->y, got->z,
			expect->r, expect->x, expect->y, expect->z);
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct ao_quaternion	position;
	struct ao_quaternion 	position_expect;
	struct ao_quaternion	rotation;
	struct ao_quaternion	rotated;
	struct ao_quaternion	little_rotation;
	int			i;
	int			p;
	int			ret = 0;

	/* vector */
	ao_quaternion_init_vector(&position, 1, 1, 1);
	ao_quaternion_init_vector(&position_expect, -1, -1, 1);

	/* zero rotation */
	ao_quaternion_init_zero_rotation(&rotation);

#define dump() do {							\
									\
		ao_quaternion_rotate(&rotated, &position, &rotation);	\
		print_q("rotated", &rotated);				\
		print_q("rotation", &rotation);				\
		printf ("\n");						\
	} while (0)

//	dump();

	for (p = 0; p < NUM_PATH; p++) {
		ao_quaternion_init_half_euler(&little_rotation,
					      ao_path[p].x / 2.0f,
					      ao_path[p].y / 2.0f,
					      ao_path[p].z / 2.0f);
//		printf ("\t\tx: %8.4f, y: %8.4f, z: %8.4f ", ao_path[p].x, ao_path[p].y, ao_path[p].z);
//		print_q("step", &little_rotation);
//		printf("\n");
		for (i = 0; i < ao_path[p].steps; i++) {
			ao_quaternion_multiply(&rotation, &little_rotation, &rotation);

			ao_quaternion_normalize(&rotation, &rotation);

//			dump();
		}
	}

	ao_quaternion_rotate(&rotated, &position, &rotation);

	ret += check_quaternion("rotation", &rotated, &position_expect);

	struct ao_quaternion	vertical;
	struct ao_quaternion	angle;
	struct ao_quaternion	rot;

	ao_quaternion_init_vector(&vertical, 0, 0, 1);
	ao_quaternion_init_vector(&angle, 0, 0, 1);

	ao_quaternion_init_half_euler(&rot,
				      M_PI * 3.0 / 8.0 , 0, 0);

	ao_quaternion_rotate(&angle, &angle, &rot);

	struct ao_quaternion	rot_compute;

	ao_quaternion_vectors_to_rotation(&rot_compute, &vertical, &angle);

	ret += check_quaternion("vector rotation", &rot_compute, &rot);

	struct ao_quaternion	rotd;

	ao_quaternion_rotate(&rotd, &vertical, &rot_compute);

	ret += check_quaternion("vector rotated", &rotd, &angle);

	return ret;
}

