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
#include <math.h>

int
cc_timedata_min(struct cc_timedata *d, double min_time, double max_time)
{
	int	i;
	int	set = 0;
	int	min_i = -1;
	double	min;

	if (d->num == 0)
		return -1;
	for (i = 0; i < d->num; i++)
		if (min_time <= d->data[i].time && d->data[i].time <= max_time)
			if (!set || d->data[i].value < min) {
				min_i = i;
				min = d->data[i].value;
				set = 1;
			}
	return min_i;
}

int
cc_timedata_max(struct cc_timedata *d, double min_time, double max_time)
{
	int	i;
	double	max;
	int	max_i = -1;
	int	set = 0;

	if (d->num == 0)
		return -1;
	for (i = 0; i < d->num; i++)
		if (min_time <= d->data[i].time && d->data[i].time <= max_time)
			if (!set || d->data[i].value > max) {
				max_i = i;
				max = d->data[i].value;
				set = 1;
			}
	return max_i;
}

int
cc_perioddata_min(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	min;
	int	min_i;

	if (d->num == 0)
		return -1;
	start = (int) ceil((min_time - d->start) / d->step);
	if (start < 0)
		start = 0;
	stop = (int) floor((max_time - d->start) / d->step);
	if (stop >= d->num)
		stop = d->num - 1;
	if (stop < start)
		return -1;
	min = d->data[start];
	min_i = start;
	for (i = start + 1; i <= stop; i++)
		if (d->data[i] < min) {
			min = d->data[i];
			min_i = i;
		}
	return min_i;
}

int
cc_perioddata_max(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	max;
	int	max_i;

	if (d->num == 0)
		return -1;
	start = (int) ceil((min_time - d->start) / d->step);
	if (start < 0)
		start = 0;
	stop = (int) floor((max_time - d->start) / d->step);
	if (stop >= d->num)
		stop = d->num - 1;
	if (stop < start)
		return -1;
	max = d->data[start];
	max_i = start;
	for (i = start + 1; i <= stop; i++)
		if (fabs(d->data[i]) > max) {
			max = fabs(d->data[i]);
			max_i = i;
		}
	return max_i;
}
