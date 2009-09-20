/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "cc.h"
#include <stdlib.h>
#include <math.h>

struct cc_perioddata *
cc_period_make(struct cc_timedata *td, double start_time, double stop_time)
{
	int			len = stop_time - start_time + 1;
	struct cc_perioddata	*pd;
	int			i, j;
	double			t;

	pd = calloc(1, sizeof (struct cc_perioddata));
	pd->start = start_time;
	pd->step = 1;
	pd->num = len;
	pd->data = calloc(len, sizeof(double));
	j = 0;
	for (i = 0; i < pd->num; i++) {
		t = start_time + i * pd->step;
		while (j < td->num - 1 && fabs(t - td->data[j].time) >= fabs(t - td->data[j+1].time))
			j++;
		pd->data[i] = td->data[j].value;
	}
	return pd;
}

struct cc_perioddata *
cc_period_low_pass(struct cc_perioddata *raw, double omega_pass, double omega_stop, double error)
{
	struct cc_perioddata *filtered;

	filtered = calloc (1, sizeof (struct cc_perioddata));
	filtered->start = raw->start;
	filtered->step = raw->step;
	filtered->num = raw->num;
	filtered->data = cc_low_pass(raw->data, raw->num, omega_pass, omega_stop, error);
	return filtered;
}
