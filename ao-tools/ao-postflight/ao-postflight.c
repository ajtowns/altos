/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cc-usb.h"
#include "cc.h"

#define NUM_BLOCK	512

static const struct option options[] = {
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s {flight-log} ...\n", program);
	exit(1);
}

static const char *state_names[] = {
	"startup",
	"idle",
	"pad",
	"boost",
	"fast",
	"coast",
	"drogue",
	"main",
	"landed",
	"invalid"
};

void
analyse_flight(struct cc_flightraw *f)
{
	double	height;
	double	accel;
	double	boost_start, boost_stop;
	double	min_pres;
	int	i;
	int	pres_i, accel_i;
	int	boost_start_set = 0;
	int	boost_stop_set = 0;
	enum ao_flight_state	state;
	double	state_start, state_stop;

	printf ("Flight:  %9d\nSerial:  %9d\n",
		f->flight, f->serial);
	boost_start = f->accel.data[0].time;
	boost_stop = f->accel.data[f->accel.num-1].time;
	for (i = 0; i < f->state.num; i++) {
		if (f->state.data[i].value == ao_flight_boost && !boost_start_set) {
			boost_start = f->state.data[i].time;
			boost_start_set = 1;
		}
		if (f->state.data[i].value > ao_flight_boost && !boost_stop_set) {
			boost_stop = f->state.data[i].time;
			boost_stop_set = 1;
		}
	}

	pres_i = cc_timedata_min(&f->pres, f->pres.data[0].time,
				 f->pres.data[f->pres.num-1].time);
	if (pres_i)
	{
		min_pres = f->pres.data[pres_i].value;
		height = cc_barometer_to_altitude(min_pres) -
			cc_barometer_to_altitude(f->ground_pres);
		printf ("Max height: %9.2fm    %9.2fft %9.2fs\n",
			height, height * 100 / 2.54 / 12,
			(f->pres.data[pres_i].time - boost_start) / 100.0);
	}

	accel_i = cc_timedata_min(&f->accel, boost_start, boost_stop);
	if (accel_i)
	{
		accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
							 f->ground_accel);
		printf ("Max accel:  %9.2fm/s² %9.2fg  %9.2fs\n",
			accel, accel /  9.80665,
			(f->accel.data[accel_i].time - boost_start) / 100.0);
	}
	for (i = 0; i < f->state.num; i++) {
		state = f->state.data[i].value;
		state_start = f->state.data[i].time;
		while (i < f->state.num - 1 && f->state.data[i+1].value == state)
			i++;
		if (i < f->state.num - 1)
			state_stop = f->state.data[i + 1].time;
		else
			state_stop = f->accel.data[f->accel.num-1].time;
		printf("State: %s\n", state_names[state]);
		printf("\tStart:      %9.2fs\n", (state_start - boost_start) / 100.0);
		printf("\tDuration:   %9.2fs\n", (state_stop - state_start) / 100.0);
		accel_i = cc_timedata_min(&f->accel, state_start, state_stop);
		if (accel_i >= 0)
		{
			accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
								 f->ground_accel);
			printf("\tMax accel:  %9.2fm/s² %9.2fg  %9.2fs\n",
			       accel, accel / 9.80665,
			       (f->accel.data[accel_i].time - boost_start) / 100.0);
		}

		pres_i = cc_timedata_min(&f->pres, state_start, state_stop);
		if (pres_i >= 0)
		{
			min_pres = f->pres.data[pres_i].value;
			height = cc_barometer_to_altitude(min_pres) -
				cc_barometer_to_altitude(f->ground_pres);
			printf ("\tMax height: %9.2fm    %9.2fft %9.2fs\n",
				height, height * 100 / 2.54 / 12,
				(f->pres.data[pres_i].time - boost_start) / 100.0);
		}
	}
}

int
main (int argc, char **argv)
{
	FILE			*file;
	int			i;
	int			ret = 0;
	struct cc_flightraw	*raw;
	int			c;
	int			serial;
	char			*s;

	while ((c = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (c) {
		default:
			usage(argv[0]);
			break;
		}
	}
	for (i = optind; i < argc; i++) {
		file = fopen(argv[i], "r");
		if (!file) {
			perror(argv[i]);
			ret++;
			continue;
		}
		s = strstr(argv[i], "-serial-");
		if (s)
			serial = atoi(s + 8);
		else
			serial = 0;
		raw = cc_log_read(file);
		if (!raw) {
			perror(argv[i]);
			ret++;
			continue;
		}
		if (!raw->serial)
			raw->serial = serial;
		analyse_flight(raw);
		cc_flightraw_free(raw);
	}
	return ret;
}
