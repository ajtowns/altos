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

void
cc_timedata_limits(struct cc_timedata *d, double min_time, double max_time, int *start, int *stop)
{
	int	i;

	*start = -1;
	for (i = 0; i < d->num; i++) {
		if (*start < 0 && min_time <= d->data[i].time)
			*start = i;
		if (d->data[i].time <= max_time)
			*stop = i;
	}
}

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
cc_timedata_min_mag(struct cc_timedata *d, double min_time, double max_time)
{
	int	i;
	int	set = 0;
	int	min_i = -1;
	double	min;

	if (d->num == 0)
		return -1;
	for (i = 0; i < d->num; i++)
		if (min_time <= d->data[i].time && d->data[i].time <= max_time)
			if (!set || fabs(d->data[i].value) < min) {
				min_i = i;
				min = fabs(d->data[i].value);
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
cc_timedata_max_mag(struct cc_timedata *d, double min_time, double max_time)
{
	int	i;
	double	max;
	int	max_i = -1;
	int	set = 0;

	if (d->num == 0)
		return -1;
	for (i = 0; i < d->num; i++)
		if (min_time <= d->data[i].time && d->data[i].time <= max_time)
			if (!set || fabs(d->data[i].value) > max) {
				max_i = i;
				max = fabs(d->data[i].value);
				set = 1;
			}
	return max_i;
}

double
cc_timedata_average(struct cc_timedata *td, double start_time, double stop_time)
{
	int			i;
	double			prev_time;
	double			next_time;
	double			interval;
	double			sum = 0.0;
	double			period = 0.0;

	prev_time = start_time;
	for (i = 0; i < td->num; i++) {
		if (start_time <= td->data[i].time && td->data[i].time <= stop_time) {
			if (i < td->num - 1 && td->data[i+1].time < stop_time)
				next_time = (td->data[i].time + td->data[i+1].time) / 2.0;
			else
				next_time = stop_time;
			interval = next_time - prev_time;
			sum += td->data[i].value * interval;
			period += interval;
			prev_time = next_time;
		}
	}
	return sum / period;
}

int
cc_perioddata_limits(struct cc_perioddata *d, double min_time, double max_time, int *start, int *stop)
{
	double	start_d, stop_d;

	if (d->num == 0)
		return 0;
	start_d = ceil((min_time - d->start) / d->step);
	if (start_d < 0)
		start_d = 0;
	stop_d = floor((max_time - d->start) / d->step);
	if (stop_d >= d->num)
		stop_d = d->num - 1;
	if (stop_d < start_d)
		return 0;
	*start = (int) start_d;
	*stop = (int) stop_d;
	return 1;
}

int
cc_perioddata_min(struct cc_perioddata *d, double min_time, double max_time)
{
	int	i;
	double	min;
	int	min_i;
	int	start, stop;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
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
cc_perioddata_min_mag(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	min;
	int	min_i;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
		return -1;
	min = d->data[start];
	min_i = start;
	for (i = start + 1; i <= stop; i++)
		if (fabs(d->data[i]) < min) {
			min = fabs(d->data[i]);
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

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
		return -1;
	max = d->data[start];
	max_i = start;
	for (i = start + 1; i <= stop; i++)
		if (d->data[i] > max) {
			max = d->data[i];
			max_i = i;
		}
	return max_i;
}

int
cc_perioddata_max_mag(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	max;
	int	max_i;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
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

double
cc_perioddata_average(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	sum = 0.0;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
		return 0.0;
	for (i = start; i <= stop; i++)
		sum += d->data[i];
	return sum / (stop - start + 1);
}

double
cc_perioddata_average_mag(struct cc_perioddata *d, double min_time, double max_time)
{
	int	start, stop;
	int	i;
	double	sum = 0.0;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
		return 0.0;
	for (i = start; i <= stop; i++)
		sum += fabs(d->data[i]);
	return sum / (stop - start + 1);
}
