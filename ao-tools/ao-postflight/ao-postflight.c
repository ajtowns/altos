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
#include <plplot/plplot.h>

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

static int plot_colors[3][3] = {
	{ 0, 0x90, 0 },	/* height */
	{ 0xa0, 0, 0 },	/* speed */
	{ 0, 0, 0xc0 },	/* accel */
};

#define PLOT_HEIGHT	0
#define PLOT_SPEED	1
#define PLOT_ACCEL	2

static void
plot_perioddata(struct cc_perioddata *d, char *axis_label, char *plot_label,
		double min_time, double max_time, int plot_type)
{
	double	*times;
	double	ymin, ymax;
	int	ymin_i, ymax_i;
	int	i;
	int	start, stop;

	if (!cc_perioddata_limits(d, min_time, max_time, &start, &stop))
		return;

	times = calloc(stop - start + 1, sizeof (double));
	for (i = start; i <= stop; i++)
		times[i-start] = i * d->step / 100.0;

	ymin_i = cc_perioddata_min(d, min_time, max_time);
	ymax_i = cc_perioddata_max(d, min_time, max_time);
	ymin = d->data[ymin_i];
	ymax = d->data[ymax_i];
	plscol0(1, 0, 0, 0);
	plscol0(2, plot_colors[plot_type][0],  plot_colors[plot_type][1],  plot_colors[plot_type][2]);
	plcol0(1);
	plenv(times[0], times[stop-start],
	      ymin, ymax, 0, 2);
	pllab("Time", axis_label, plot_label);
	plcol0(2);
	plline(stop - start + 1, times, d->data + start);
	free(times);
}

static void
plot_timedata(struct cc_timedata *d, char *axis_label, char *plot_label,
	      double min_time, double max_time, int plot_type)
{
	double	*times;
	double	*values;
	double	ymin, ymax;
	int	ymin_i, ymax_i;
	int	i;
	int	start = -1, stop = -1;
	double	start_time = 0, stop_time = 0;
	int	num;

	for (i = 0; i < d->num; i++) {
		if (start < 0 && d->data[i].time >= min_time) {
			start_time = d->data[i].time;
			start = i;
		}
		if (d->data[i].time <= max_time) {
			stop_time = d->data[i].time;
			stop = i;
		}
	}

	times = calloc(stop - start + 1, sizeof (double));
	values = calloc(stop - start + 1, sizeof (double));

	ymin_i = cc_timedata_min(d, min_time, max_time);
	ymax_i = cc_timedata_max(d, min_time, max_time);
	ymin = d->data[ymin_i].value;
	ymax = d->data[ymax_i].value;
	for (i = start; i <= stop; i++) {
		times[i-start] = (d->data[i].time - start_time)/100.0;
		values[i-start] = d->data[i].value;
	}
	plscol0(1, 0, 0, 0);
	plscol0(2, plot_colors[plot_type][0],  plot_colors[plot_type][1],  plot_colors[plot_type][2]);
	plcol0(1);
	plenv(times[0], times[stop-start], ymin, ymax, 0, 2);
	pllab("Time", axis_label, plot_label);
	plcol0(2);
	plline(stop - start + 1, times, values);
	free(times);
	free(values);
}

static struct cc_perioddata *
merge_data(struct cc_perioddata *first, struct cc_perioddata *last, double split_time)
{
	int			i;
	struct cc_perioddata	*pd;
	int			num;
	double			start_time, stop_time;
	double			t;

	pd = calloc(1, sizeof (struct cc_perioddata));
	start_time = first->start;
	stop_time = last->start + last->step * last->num;
	num = (stop_time - start_time) / first->step;
	pd->num = num;
	pd->data = calloc(num, sizeof (double));
	pd->start = first->start;
	pd->step = first->step;
	for (i = 0; i < num; i++) {
		t = pd->start + i * pd->step;
		if (t <= split_time) {
			pd->data[i] = first->data[i];
		} else {
			int	j;

			j = (t - last->start) / last->step;
			if (j < 0 || j >= last->num)
				pd->data[i] = 0;
			else
				pd->data[i] = last->data[j];
		}
	}
	return pd;
}

static const char kml_header[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<kml xmlns=\"http://earth.google.com/kml/2.0\">\n"
	"  <Placemark>\n"
	"    <name>gps</name>\n"
	"    <Style id=\"khStyle690\">\n"
	"      <LineStyle id=\"khLineStyle694\">\n"
	"        <color>ff00ffff</color>\n"
	"        <width>4</width>\n"
	"      </LineStyle>\n"
	"      </Style>\n"
	"    <MultiGeometry id=\"khMultiGeometry697\">\n"
	"      <LineString id=\"khLineString698\">\n"
	"        <tessellate>1</tessellate>\n"
	"        <altitudeMode>absolute</altitudeMode>\n"
	"        <coordinates>\n";

static const char kml_footer[] =
	"</coordinates>\n"
	"    </LineString>\n"
	"  </MultiGeometry>\n"
	"</Placemark>\n"
	"</kml>\n";

static void
analyse_flight(struct cc_flightraw *f, FILE *summary_file, FILE *detail_file,
	       FILE *raw_file, char *plot_name, FILE *gps_file, FILE *kml_file)
{
	double	height;
	double	accel;
	double	speed;
	double	avg_speed;
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

	fprintf(summary_file,
		"Serial:  %9d\n"
		"Flight:  %9d\n",
		f->serial, f->flight);
	if (f->year) {
		fprintf(summary_file,
			"Date:   %04d-%02d-%02d\n",
			f->year, f->month, f->day);
	}
	if (f->gps.num) {
		fprintf(summary_file,
			"Time:     %2d:%02d:%02d\n",
			f->gps.data[0].hour,
			f->gps.data[0].minute,
			f->gps.data[0].second);
	}
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
				speed_i = cc_perioddata_max_mag(&cooked->accel_speed, state_start, state_stop);
				if (speed_i >= 0)
					speed = cooked->accel_speed.data[speed_i];
				avg_speed = cc_perioddata_average(&cooked->accel_speed, state_start, state_stop);
			} else {
				speed_i = cc_perioddata_max_mag(&cooked->pres_speed, state_start, state_stop);
				if (speed_i >= 0)
					speed = cooked->pres_speed.data[speed_i];
				avg_speed = cc_perioddata_average(&cooked->pres_speed, state_start, state_stop);
			}
			if (speed_i >= 0)
			{
				fprintf(summary_file, "\tMax speed:  %9.2fm/s  %9.2fft/s %9.2fs\n",
				       speed, speed * 100 / 2.4 / 12.0,
				       (cooked->accel_speed.start + speed_i * cooked->accel_speed.step - boost_start) / 100.0);
				fprintf(summary_file, "\tAvg speed:  %9.2fm/s  %9.2fft/s\n",
					avg_speed, avg_speed * 100 / 2.4 / 12.0);
			}
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
		double	max_height = 0;
		int	i;
		double	*times;

		fprintf(detail_file, "%9s %9s %9s %9s\n",
		       "time", "height", "speed", "accel");
		for (i = 0; i < cooked->pres_pos.num; i++) {
			double	time = (cooked->accel_accel.start + i * cooked->accel_accel.step - boost_start) / 100.0;
			double	accel = cooked->accel_accel.data[i];
			double	pos = cooked->pres_pos.data[i];
			double	speed;
			if (cooked->pres_pos.start + cooked->pres_pos.step * i < apogee)
				speed = cooked->accel_speed.data[i];
			else
				speed = cooked->pres_speed.data[i];
			fprintf(detail_file, "%9.2f %9.2f %9.2f %9.2f\n",
			       time, pos, speed, accel);
		}
	}
	if (raw_file) {
		fprintf(raw_file, "%9s %9s %9s\n",
		       "time", "height", "accel");
		for (i = 0; i < cooked->pres.num; i++) {
			double time = cooked->pres.data[i].time;
			double pres = cooked->pres.data[i].value;
			double accel = cooked->accel.data[i].value;
			fprintf(raw_file, "%9.2f %9.2f %9.2f %9.2f\n",
				time, pres, accel);
		}
	}
	if (gps_file) {
		int	j = 0;
		fprintf(gps_file, "%9s %12s %12s %12s\n",
			"time", "lat", "lon", "alt");
		for (i = 0; i < f->gps.num; i++) {
			int	nsat = 0;
			int	k;
			while (j < f->gps.numsats - 1) {
				if (f->gps.sats[j].sat[0].time <= f->gps.data[i].time &&
				    f->gps.data[i].time < f->gps.sats[j+1].sat[0].time)
					break;
				j++;
			}
			fprintf(gps_file, "%12.7f %12.7f %12.7f %12.7f",
				(f->gps.data[i].time - boost_start) / 100.0,
				f->gps.data[i].lat,
				f->gps.data[i].lon,
				f->gps.data[i].alt);
			nsat = 0;
			for (k = 0; k < f->gps.sats[j].nsat; k++) {
				fprintf (gps_file, " %12.7f", (double) f->gps.sats[j].sat[k].c_n);
				if (f->gps.sats[j].sat[k].svid != 0)
					nsat++;
			}
			fprintf(gps_file, " %d\n", nsat);
		}
	}
	if (kml_file) {
		int	j = 0;

		fprintf(kml_file, "%s", kml_header);
		for (i = 0; i < f->gps.num; i++) {
			int	nsat = 0;
			int	k;
			while (j < f->gps.numsats - 1) {
				if (f->gps.sats[j].sat[0].time <= f->gps.data[i].time &&
				    f->gps.data[i].time < f->gps.sats[j+1].sat[0].time)
					break;
				j++;
			}
			nsat = 0;
			for (k = 0; k < f->gps.sats[j].nsat; k++)
				if (f->gps.sats[j].sat[k].svid != 0)
					nsat++;

			fprintf(kml_file, "%12.7f, %12.7f, %12.7f <!-- time %12.7f sats %d -->",
				f->gps.data[i].lon,
				f->gps.data[i].lat,
				f->gps.data[i].alt,
				(f->gps.data[i].time - boost_start) / 100.0,
				nsat);
			if (i < f->gps.num - 1)
				fprintf(kml_file, ",\n");
			else
				fprintf(kml_file, "\n");
		}
		fprintf(kml_file, "%s", kml_footer);
	}
	if (cooked && plot_name) {
		struct cc_perioddata	*speed;
		plsdev("svgcairo");
		plsfnam(plot_name);
#define PLOT_DPI	96
		plspage(PLOT_DPI, PLOT_DPI, 8 * PLOT_DPI, 8 * PLOT_DPI, 0, 0);
		plscolbg(0xff, 0xff, 0xff);
		plscol0(1,0,0,0);
		plstar(2, 3);
		speed = merge_data(&cooked->accel_speed, &cooked->pres_speed, apogee);

		plot_perioddata(&cooked->pres_pos, "meters", "Height",
				-1e10, 1e10, PLOT_HEIGHT);
		plot_perioddata(&cooked->pres_pos, "meters", "Height to Apogee",
				boost_start, apogee + (apogee - boost_start) / 10.0, PLOT_HEIGHT);
		plot_perioddata(speed, "meters/second", "Speed",
				-1e10, 1e10, PLOT_SPEED);
		plot_perioddata(speed, "meters/second", "Speed to Apogee",
				boost_start, apogee + (apogee - boost_start) / 10.0, PLOT_SPEED);
		plot_perioddata(&cooked->accel_accel, "meters/second²", "Acceleration",
				-1e10, 1e10, PLOT_ACCEL);
/*		plot_perioddata(&cooked->accel_accel, "meters/second²", "Acceleration during Boost",
		boost_start, boost_stop + (boost_stop - boost_start) / 2.0, PLOT_ACCEL); */
		plot_timedata(&cooked->accel, "meters/second²", "Acceleration during Boost",
				boost_start, boost_stop + (boost_stop - boost_start) / 2.0, PLOT_ACCEL);
		free(speed->data);
		free(speed);
		plend();
	}
	if (cooked)
		cc_flightcooked_free(cooked);
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 1, .val = 's' },
	{ .name = "detail", .has_arg = 1, .val = 'd' },
	{ .name = "plot", .has_arg = 1, .val = 'p' },
	{ .name = "raw", .has_arg = 1, .val = 'r' },
	{ .name = "gps", .has_arg = 1, .val = 'g' },
	{ .name = "kml", .has_arg = 1, .val = 'k' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s\n"
		"\t[--summary=<summary-file>] [-s <summary-file>]\n"
		"\t[--detail=<detail-file] [-d <detail-file>]\n"
		"\t[--raw=<raw-file> -r <raw-file]\n"
		"\t[--plot=<plot-file> -p <plot-file>]\n"
		"\t[--gps=<gps-file> -g <gps-file>]\n"
		"\t[--kml=<kml-file> -k <kml-file>]\n"
		"\t{flight-log} ...\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	FILE			*file;
	FILE			*summary_file = NULL;
	FILE			*detail_file = NULL;
	FILE			*raw_file = NULL;
	FILE			*gps_file = NULL;
	FILE			*kml_file = NULL;
	int			i;
	int			ret = 0;
	struct cc_flightraw	*raw;
	int			c;
	int			serial;
	char			*s;
	char			*summary_name = NULL;
	char			*detail_name = NULL;
	char			*raw_name = NULL;
	char			*plot_name = NULL;
	char			*gps_name = NULL;
	char			*kml_name = NULL;

	while ((c = getopt_long(argc, argv, "s:d:p:r:g:k:", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary_name = optarg;
			break;
		case 'd':
			detail_name = optarg;
			break;
		case 'p':
			plot_name = optarg;
			break;
		case 'r':
			raw_name = optarg;
			break;
		case 'g':
			gps_name = optarg;
			break;
		case 'k':
			kml_name = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	summary_file = stdout;
	if (summary_name) {
		summary_file = fopen(summary_name, "w");
		if (!summary_file) {
			perror (summary_name);
			exit(1);
		}
	}
	if (detail_name) {
		if (summary_name && !strcmp (summary_name, detail_name))
			detail_file = summary_file;
		else {
			detail_file = fopen(detail_name, "w");
			if (!detail_file) {
				perror(detail_name);
				exit(1);
			}
		}
	}
	if (raw_name) {
		raw_file = fopen (raw_name, "w");
		if (!raw_file) {
			perror(raw_name);
			exit(1);
		}
	}
	if (gps_name) {
		gps_file = fopen(gps_name, "w");
		if (!gps_file) {
			perror(gps_name);
			exit(1);
		}
	}
	if (kml_name) {
		kml_file = fopen(kml_name, "w");
		if (!kml_file) {
			perror(kml_name);
			exit(1);
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
		analyse_flight(raw, summary_file, detail_file, raw_file, plot_name, gps_file, kml_file);
		cc_flightraw_free(raw);
	}
	return ret;
}
