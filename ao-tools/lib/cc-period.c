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

struct cc_perioddata *
cc_period_make(struct cc_timedata *td, double start_time, double stop_time)
{
	int			len = stop_time - start_time + 1;
	struct cc_perioddata	*pd;
	int			i;
	double			prev_time;
	double			next_time;
	double			interval;

	pd = calloc(1, sizeof (struct cc_perioddata));
	pd->start = start_time;
	pd->step = 1;
	pd->num = len;
	pd->data = calloc(len, sizeof(double));
	prev_time = start_time;
	for (i = 0; i < td->num; i++) {
		if (start_time <= td->data[i].time && td->data[i].time <= stop_time) {
			int	pos = td->data[i].time - start_time;

			if (i < td->num - 1 && td->data[i+1].time < stop_time)
				next_time = (td->data[i].time + td->data[i+1].time) / 2.0;
			else
				next_time = stop_time;
			interval = next_time - prev_time;
			pd->data[pos] = td->data[i].value * interval;
			prev_time = next_time;
		}
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
