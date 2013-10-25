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

static void
print_q(char *name, struct ao_quaternion *q)
{
	printf ("%8.8s: r%8.5f x%8.5f y%8.5f z%8.5f ", name,
		q->r, q->x, q->y, q->z);
}

int main(int argc, char **argv)
{
	struct ao_quaternion	position;
	struct ao_quaternion	rotation;
	struct ao_quaternion	little_rotation;
	int			i;

	/* unit x vector */
	ao_quaternion_init_vector(&position, 1, 0, 0);

	/* zero rotation */
	ao_quaternion_init_zero_rotation(&rotation);

	/* π/16 rotation around Z axis */
	ao_quaternion_init_rotation(&little_rotation, 0, 0, 1,
				    sin((M_PI/16)/2),
				    cos((M_PI/16)/2));
	for (i = 0; i <= 16; i++) {
		struct ao_quaternion	rotated;

		ao_quaternion_rotate(&rotated, &position, &rotation);
		print_q("position", &position);
		print_q("rotated", &rotated);
		print_q("rotation", &rotation);
		printf ("\n");
		ao_quaternion_multiply(&rotation, &rotation, &little_rotation);
		ao_quaternion_normalize(&rotation, &rotation);
	}
	return 0;
}

