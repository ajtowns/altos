/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#include <stdint.h>
#define AO_CONVERT_TEST
#include "ao_host.h"
#include "ao_convert.c"

#define STEP	1

static inline i_abs(int i) { return i < 0 ? -i : i; }

main ()
{
	int	i;
	int16_t p_to_a, p_to_a_to_p;
	int16_t a_to_p, a_to_p_to_a;
	int max_p_error = 0, max_p_error_p = -1;
	int max_a_error = 0, max_a_error_a = -1;
	int p_error;
	int a_error;
	int ret = 0;

	for (i = 0; i < 32767 + STEP; i += STEP) {
		if (i > 32767)
			i = 32767;
		p_to_a = ao_pres_to_altitude(i);
		p_to_a_to_p = ao_altitude_to_pres(p_to_a);
		p_error = i_abs(p_to_a_to_p - i);
		if (p_error > max_p_error) {
			max_p_error = p_error;
			max_p_error_p = i;
		}
//		printf ("pres %d alt %d pres %d\n",
//			i, p_to_a, p_to_a_to_p);
	}
	for (i = -1578; i < 15835 + STEP; i += STEP) {
		if (i > 15835)
			i = 15835;
		a_to_p = ao_altitude_to_pres(i);
		a_to_p_to_a = ao_pres_to_altitude(a_to_p);
		a_error = i_abs(a_to_p_to_a - i);
		if (a_error > max_a_error) {
			max_a_error = a_error;
			max_a_error_a = i;
		}
//		printf ("alt %d pres %d alt %d\n",
//			i, a_to_p, a_to_p_to_a);
	}
	if (max_p_error > 2) {
		printf ("max p error %d at %d\n", max_p_error,
			max_p_error_p);
		ret++;
	}
	if (max_a_error > 1) {
		printf ("max a error %d at %d\n", max_a_error,
			max_a_error_a);
		ret++;
	}
	return ret;
}
