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

struct cc_timedata *
cc_timedata_convert(struct cc_timedata *d, double (*f)(double v, double a), double a)
{
	struct cc_timedata	*r;
	int			n;

	r = calloc (1, sizeof (struct cc_timedata));
	r->num = d->num;
	r->size = d->num;
	r->data = calloc (r->size, sizeof (struct cc_timedataelt));
	r->time_offset = d->time_offset;
	for (n = 0; n < d->num; n++) {
		r->data[n].time = d->data[n].time;
		r->data[n].value = f(d->data[n].value, a);
	}
	return r;
}

struct cc_timedata *
cc_timedata_integrate(struct cc_timedata *d, double min_time, double max_time)
{
	struct cc_timedata	*i;
	int			n, m;
	int			start, stop;

	cc_timedata_limits(d, min_time, max_time, &start, &stop);
	i = calloc (1, sizeof (struct cc_timedata));
	i->num = stop - start + 1;
	i->size = i->num;
	i->data = calloc (i->size, sizeof (struct cc_timedataelt));
	i->time_offset = d->data[start].time;
	for (n = 0; n < i->num; n++) {
		m = n + start;
		i->data[n].time = d->data[m].time;
		if (n == 0) {
			i->data[n].value = 0;
		} else {
			i->data[n].value = i->data[n-1].value +
				(d->data[m].value + d->data[m-1].value) / 2 *
				((d->data[m].time - d->data[m-1].time) / 100.0);
		}
	}
	return i;
}

struct cc_perioddata *
cc_perioddata_differentiate(struct cc_perioddata *i)
{
	struct cc_perioddata	*d;
	int			n;

	d = calloc (1, sizeof (struct cc_perioddata));
	d->num = i->num;
	d->start = i->start;
	d->step = i->step;
	d->data = calloc (d->num, sizeof(double));
	for (n = 1; n < d->num; n++)
		d->data[n] = (i->data[n] - i->data[n-1]) / (i->step / 100.0);
	d->data[0] = d->data[1];
	return d;
}
