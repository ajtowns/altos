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
analyse_flight(struct cc_flightraw *f, FILE *summary_file, FILE *detail_file)
{
	double	height;
	double	accel;
	double	speed;
	double	boost_start, boost_stop;
	double	min_pres;
	int	i;
	int	pres_i, accel_i, speed_i;
	int	boost_start_set = 0;
	int	boost_stop_set = 0;
	enum ao_flight_state	state;
	double	state_start, state_stop;
	struct cc_flightcooked *cooked;
	double	apogee;

	fprintf(summary_file, "Flight:  %9d\nSerial:  %9d\n",
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
	if (pres_i >= 0)
	{
		min_pres = f->pres.data[pres_i].value;
		height = cc_barometer_to_altitude(min_pres) -
			cc_barometer_to_altitude(f->ground_pres);
		fprintf(summary_file, "Max height: %9.2fm    %9.2fft   %9.2fs\n",
			height, height * 100 / 2.54 / 12,
			(f->pres.data[pres_i].time - boost_start) / 100.0);
		apogee = f->pres.data[pres_i].time;
	}

	cooked = cc_flight_cook(f);
	if (cooked) {
		speed_i = cc_perioddata_max(&cooked->accel_speed, boost_start, boost_stop);
		if (speed_i >= 0) {
			speed = cooked->accel_speed.data[speed_i];
			fprintf(summary_file, "Max speed:  %9.2fm/s  %9.2fft/s %9.2fs\n",
			       speed, speed * 100 / 2.4 / 12.0,
			       (cooked->accel_speed.start + speed_i * cooked->accel_speed.step - boost_start) / 100.0);
		}
	}
	accel_i = cc_timedata_min(&f->accel, boost_start, boost_stop);
	if (accel_i >= 0)
	{
		accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
							 f->ground_accel);
		fprintf(summary_file, "Max accel:  %9.2fm/s² %9.2fg    %9.2fs\n",
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
		fprintf(summary_file, "State: %s\n", state_names[state]);
		fprintf(summary_file, "\tStart:      %9.2fs\n", (state_start - boost_start) / 100.0);
		fprintf(summary_file, "\tDuration:   %9.2fs\n", (state_stop - state_start) / 100.0);
		accel_i = cc_timedata_min(&f->accel, state_start, state_stop);
		if (accel_i >= 0)
		{
			accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
								 f->ground_accel);
			fprintf(summary_file, "\tMax accel:  %9.2fm/s² %9.2fg    %9.2fs\n",
			       accel, accel / 9.80665,
			       (f->accel.data[accel_i].time - boost_start) / 100.0);
		}

		if (cooked) {
			if (state < ao_flight_drogue) {
				speed_i = cc_perioddata_max(&cooked->accel_speed, state_start, state_stop);
				if (speed_i >= 0)
					speed = cooked->accel_speed.data[speed_i];
			} else {
				speed_i = cc_perioddata_max(&cooked->pres_speed, state_start, state_stop);
				if (speed_i >= 0)
					speed = cooked->pres_speed.data[speed_i];
			}
			if (speed_i >= 0)
				fprintf(summary_file, "\tMax speed:  %9.2fm/s  %9.2fft/s %9.2fs\n",
				       speed, speed * 100 / 2.4 / 12.0,
				       (cooked->accel_speed.start + speed_i * cooked->accel_speed.step - boost_start) / 100.0);
		}
		pres_i = cc_timedata_min(&f->pres, state_start, state_stop);
		if (pres_i >= 0)
		{
			min_pres = f->pres.data[pres_i].value;
			height = cc_barometer_to_altitude(min_pres) -
				cc_barometer_to_altitude(f->ground_pres);
			fprintf(summary_file, "\tMax height: %9.2fm    %9.2fft   %9.2fs\n",
				height, height * 100 / 2.54 / 12,
				(f->pres.data[pres_i].time - boost_start) / 100.0);
		}
	}
	if (cooked && detail_file) {
		double	apogee_time;
		double	max_height = 0;
		int	i;

		for (i = 0; i < cooked->pres_pos.num; i++) {
			if (cooked->pres_pos.data[i] > max_height) {
				max_height = cooked->pres_pos.data[i];
				apogee_time = cooked->pres_pos.start + cooked->pres_pos.step * i;
			}
		}
		fprintf(detail_file, "%9s %9s %9s %9s\n",
		       "time", "height", "speed", "accel");
		for (i = 0; i < cooked->pres_pos.num; i++) {
			double	time = (cooked->accel_accel.start + i * cooked->accel_accel.step - boost_start) / 100.0;
			double	accel = cooked->accel_accel.data[i];
			double	pos = cooked->pres_pos.data[i];
			double	speed;
			if (cooked->pres_pos.start + cooked->pres_pos.step * i < apogee_time)
				speed = cooked->accel_speed.data[i];
			else
				speed = cooked->pres_speed.data[i];
			fprintf(detail_file, "%9.2f %9.2f %9.2f %9.2f\n",
			       time, pos, speed, accel);
		}
	}
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 1, .val = 'S' },
	{ .name = "detail", .has_arg = 1, .val = 'D' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--summary=<summary-file>] [--detail=<detail-file] {flight-log} ...\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	FILE			*file;
	FILE			*summary_file;
	FILE			*detail_file;
	int			i;
	int			ret = 0;
	struct cc_flightraw	*raw;
	int			c;
	int			serial;
	char			*s;
	char			*summary_name = NULL, *detail_name = NULL;

	while ((c = getopt_long(argc, argv, "S:D:", options, NULL)) != -1) {
		switch (c) {
		case 'S':
			summary_name = optarg;
			break;
		case 'D':
			detail_name = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	summary_file = stdout;
	detail_file = NULL;
	if (summary_name) {
		summary_file = fopen(summary_name, "w");
		if (!summary_file) {
			perror (summary_name);
			exit(1);
		}
	}
	if (detail_name) {
		if (!strcmp (summary_name, detail_name))
			detail_file = summary_file;
		else {
			detail_file = fopen(detail_name, "w");
			if (!detail_file) {
				perror(detail_name);
				exit(1);
			}
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
		analyse_flight(raw, summary_file, detail_file);
		cc_flightraw_free(raw);
	}
	return ret;
}
