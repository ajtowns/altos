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

static const char *kml_state_colours[] = {
	"FF000000",
	"FF000000",
	"FF000000",
	"FF0000FF",
	"FF4080FF",
	"FF00FFFF",
	"FFFF0000",
	"FF00FF00",
	"FF000000",
	"FFFFFFFF"
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

static const char kml_header_start[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
	"<Document>\n"
	"  <name>%s</name>\n"
	"  <description>\n";
static const char kml_header_end[] =
	"  </description>\n"
	"  <open>0</open>\n";

static const char kml_style_start[] =
	"  <Style id=\"ao-flightstate-%s\">\n"
	"    <LineStyle><color>%s</color><width>4</width></LineStyle>\n"
	"    <BalloonStyle>\n"
	"      <text>\n";
static const char kml_style_end[] =
	"      </text>\n"
	"    </BalloonStyle>\n"
	"  </Style>\n";

static const char kml_placemark_start[] =
	"  <Placemark>\n"
	"    <name>%s</name>\n"
	"    <styleUrl>#ao-flightstate-%s</styleUrl>\n"
	"    <LineString>\n"
	"      <tessellate>1</tessellate>\n"
	"      <altitudeMode>absolute</altitudeMode>\n"
	"      <coordinates>\n";

static const char kml_coord_fmt[] =
	"        %12.7f, %12.7f, %12.7f <!-- alt %12.7f time %12.7f sats %d -->\n";

static const char kml_placemark_end[] =
	"      </coordinates>\n"
	"    </LineString>\n"
	"  </Placemark>\n";

static const char kml_footer[] =
	"      </coordinates>\n"
	"    </LineString>\n"
	"  </Placemark>\n"
	"</Document>\n"
	"</kml>\n";

static unsigned
gps_daytime(struct cc_gpselt *gps)
{
	return ((gps->hour * 60 +
		 gps->minute) * 60 +
		gps->second) * 1000;
}

int
daytime_hour(unsigned daytime)
{
	return daytime / 1000 / 60 / 60;
}

int
daytime_minute(unsigned daytime)
{
	return (daytime / 1000 / 60) % 60;
}

int
daytime_second(unsigned daytime)
{
	return (daytime / 1000) % 60;
}

int
daytime_millisecond(unsigned daytime)
{
	return daytime % 1000;
}

static unsigned
compute_daytime_ms(double time, struct cc_gpsdata *gps)
{
	int	i;
	unsigned	gps_start_daytime, gps_stop_daytime;

	if (time <= gps->data[0].time) {
		gps_stop_daytime = gps_daytime(&gps->data[0]);
		return gps_stop_daytime - (gps->data[0].time - time) * 10;
	}
	for (i = 0; i < gps->num - 1; i++)
		if (time > gps->data[i].time)
			break;
	gps_start_daytime = gps_daytime(&gps->data[i]);
	if (i == gps->num - 1) {
		return gps_start_daytime + (time - gps->data[i].time) * 10;
	} else {
		unsigned	gps_period_daytime;
		double		gps_period_time;
		double		time_since_start;

		gps_stop_daytime = gps_daytime(&gps->data[i + 1]);

		/* range of gps daytime values */
		gps_period_daytime = gps_stop_daytime - gps_start_daytime;

		/* range of gps time values */
		gps_period_time = gps->data[i+1].time - gps->data[i].time;

		/* sample time after first gps time */
		time_since_start = time - gps->data[i].time;

		return gps_start_daytime +
			gps_period_daytime * time_since_start / gps_period_time;
	}
}

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
	char	buf[128];

	if (kml_file) {
                snprintf(buf, sizeof (buf), "AO Flight#%d S/N: %03d", f->flight, f->serial);
		fprintf(kml_file, kml_header_start, buf);
	}

	fprintf(summary_file,
		"Serial:  %9d\n"
		"Flight:  %9d\n",
		f->serial, f->flight);

	if (f->year) {
		snprintf(buf, sizeof (buf),
			"Date:   %04d-%02d-%02d\n",
			f->year, f->month, f->day);
		fprintf(summary_file, "%s", buf);
		if (kml_file) fprintf(kml_file, "%s", buf);
	}
	if (f->gps.num) {
		snprintf(buf, sizeof (buf),
			"Time:     %2d:%02d:%02d\n",
			f->gps.data[0].hour,
			f->gps.data[0].minute,
			f->gps.data[0].second);
		fprintf(summary_file, "%s", buf);
		if (kml_file) fprintf(kml_file, "%s", buf);
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
		apogee = f->pres.data[pres_i].time;
		snprintf(buf, sizeof (buf), "Max height: %9.2fm    %9.2fft   %9.2fs\n",
			height, height * 100 / 2.54 / 12,
			(f->pres.data[pres_i].time - boost_start) / 100.0);
		fprintf(summary_file, "%s", buf);
		if (kml_file) fprintf(kml_file, "%s", buf);
	}

	cooked = cc_flight_cook(f);
	if (cooked) {
		speed_i = cc_perioddata_max(&cooked->accel_speed, boost_start, boost_stop);
		if (speed_i >= 0) {
			speed = cooked->accel_speed.data[speed_i];
			snprintf(buf, sizeof (buf), "Max speed:  %9.2fm/s  %9.2fft/s %9.2fs\n",
			       speed, speed * 100 / 2.4 / 12.0,
			       (cooked->accel_speed.start + speed_i * cooked->accel_speed.step - boost_start) / 100.0);
			fprintf(summary_file, "%s", buf);
			if (kml_file) fprintf(kml_file, "%s", buf);
		}
	}
	accel_i = cc_timedata_min(&f->accel, boost_start, boost_stop);
	if (accel_i >= 0)
	{
		accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
							 f->ground_accel);
		snprintf(buf, sizeof (buf), "Max accel:  %9.2fm/s² %9.2fg    %9.2fs\n",
			accel, accel /  9.80665,
			(f->accel.data[accel_i].time - boost_start) / 100.0);
		fprintf(summary_file, "%s", buf);
		if (kml_file) fprintf(kml_file, "%s", buf);
	}

	if (kml_file)
		fprintf(kml_file, "%s", kml_header_end);

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
		if (kml_file) {
			fprintf(kml_file, kml_style_start, state_names[state], kml_state_colours[state]);
			fprintf(kml_file, "\tState: %s\n", state_names[state]);
			fprintf(kml_file, "\tStart:      %9.2fs\n", (state_start - boost_start) / 100.0);
			fprintf(kml_file, "\tDuration:   %9.2fs\n", (state_stop - state_start) / 100.0);
		}

		accel_i = cc_timedata_min(&f->accel, state_start, state_stop);
		if (accel_i >= 0)
		{
			accel = cc_accelerometer_to_acceleration(f->accel.data[accel_i].value,
								 f->ground_accel);
			snprintf(buf, sizeof (buf), "\tMax accel:  %9.2fm/s² %9.2fg    %9.2fs\n",
			       accel, accel / 9.80665,
			       (f->accel.data[accel_i].time - boost_start) / 100.0);
			fprintf(summary_file, "%s", buf);
			if (kml_file) fprintf(kml_file, "%s", buf);
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
				snprintf(buf, sizeof (buf), "\tMax speed:  %9.2fm/s  %9.2fft/s %9.2fs\n",
				       speed, speed * 100 / 2.4 / 12.0,
				       (cooked->accel_speed.start + speed_i * cooked->accel_speed.step - boost_start) / 100.0);
				fprintf(summary_file, "%s", buf);
				if (kml_file) fprintf(kml_file, "%s", buf);

				snprintf(buf, sizeof (buf), "\tAvg speed:  %9.2fm/s  %9.2fft/s\n",
					avg_speed, avg_speed * 100 / 2.4 / 12.0);
				fprintf(summary_file, "%s", buf);
				if (kml_file) fprintf(kml_file, "%s", buf);
			}
		}
		pres_i = cc_timedata_min(&f->pres, state_start, state_stop);
		if (pres_i >= 0)
		{
			min_pres = f->pres.data[pres_i].value;
			height = cc_barometer_to_altitude(min_pres) -
				cc_barometer_to_altitude(f->ground_pres);
			snprintf(buf, sizeof (buf), "\tMax height: %9.2fm    %9.2fft   %9.2fs\n",
				height, height * 100 / 2.54 / 12,
				(f->pres.data[pres_i].time - boost_start) / 100.0);
			fprintf(summary_file, "%s", buf);
			if (kml_file) fprintf(kml_file, "%s", buf);
		}
		if (kml_file) fprintf(kml_file, "%s", kml_style_end);
	}
	if (cooked && detail_file) {
		double	max_height = 0;
		int	i;
		double	*times;

		fprintf(detail_file, "%9s %9s %9s %9s %9s\n",
			"time", "height", "speed", "accel", "daytime");
		for (i = 0; i < cooked->pres_pos.num; i++) {
			double	clock_time = cooked->accel_accel.start + i * cooked->accel_accel.step;
			double	time = (clock_time - boost_start) / 100.0;
			double	accel = cooked->accel_accel.data[i];
			double	pos = cooked->pres_pos.data[i];
			double	speed;
			unsigned	daytime;
			if (cooked->pres_pos.start + cooked->pres_pos.step * i < apogee)
				speed = cooked->accel_speed.data[i];
			else
				speed = cooked->pres_speed.data[i];
			if (f->gps.num)
				daytime = compute_daytime_ms(clock_time, &f->gps);
			else
				daytime = 0;
			fprintf(detail_file, "%9.2f %9.2f %9.2f %9.2f %02d:%02d:%02d.%03d\n",
				time, pos, speed, accel,
				daytime_hour(daytime),
				daytime_minute(daytime),
				daytime_second(daytime),
				daytime_millisecond(daytime));
		}
	}
	if (raw_file) {
		fprintf(raw_file, "%9s %9s %9s %9s\n",
			"time", "height", "accel", "daytime");
		for (i = 0; i < cooked->pres.num; i++) {
			double time = cooked->pres.data[i].time;
			double pres = cooked->pres.data[i].value;
			double accel = cooked->accel.data[i].value;
			unsigned	daytime;
			if (f->gps.num)
				daytime = compute_daytime_ms(time, &f->gps);
			else
				daytime = 0;
			fprintf(raw_file, "%9.2f %9.2f %9.2f %02d:%02d:%02d.%03d\n",
				time, pres, accel,
				daytime_hour(daytime),
				daytime_minute(daytime),
				daytime_second(daytime),
				daytime_millisecond(daytime));
		}
	}
	if (gps_file || kml_file) {
		int	j = 0, baro_pos;
		double	baro_offset;
		double	baro = 0.0;
		int	state_idx = 0;

		if (gps_file)
			fprintf(gps_file, "%2s %2s %2s %9s %12s %12s %9s %8s %5s\n",
				"hr", "mn", "sc",
				"time", "lat", "lon", "alt", "baro", "nsat");
		if (kml_file)
			fprintf(kml_file, kml_placemark_start,
				state_names[(int)f->state.data[state_idx].value],
				state_names[(int)f->state.data[state_idx].value]); 

		if (f->gps.num)
			baro_offset = f->gps.data[0].alt;
		else
			baro_offset = 0;
		baro_pos = 0;
		for (i = 0; i < f->gps.num; i++) {
			int	nsat = 0;
			int	k;
			while (j < f->gps.numsats - 1) {
				if (f->gps.sats[j].sat[0].time <= f->gps.data[i].time &&
				    f->gps.data[i].time < f->gps.sats[j+1].sat[0].time)
					break;
				j++;
			}
			if (cooked) {
				while (baro_pos < cooked->pres_pos.num) {
					double  baro_time = cooked->accel_accel.start + baro_pos * cooked->accel_accel.step;
					if (baro_time >= f->gps.data[i].time)
						break;
					baro_pos++;
				}
				if (baro_pos < cooked->pres_pos.num)
					baro = cooked->pres_pos.data[baro_pos];
			}
			if (gps_file)
				fprintf(gps_file, "%2d %2d %2d %12.7f %12.7f %12.7f %7.1f %7.1f",
					f->gps.data[i].hour,
					f->gps.data[i].minute,
					f->gps.data[i].second,
					(f->gps.data[i].time - boost_start) / 100.0,
					f->gps.data[i].lat,
					f->gps.data[i].lon,
					f->gps.data[i].alt,
					baro + baro_offset);

			nsat = 0;
			if (f->gps.sats) {
				for (k = 0; k < f->gps.sats[j].nsat; k++) {
					if (f->gps.sats[j].sat[k].svid != 0)
						nsat++;
				}
				if (gps_file) {
					fprintf(gps_file, " %4d", nsat);
					for (k = 0; k < f->gps.sats[j].nsat; k++) {
						if (f->gps.sats[j].sat[k].svid != 0) {
							fprintf (gps_file, " %3d(%4.1f)",
								 f->gps.sats[j].sat[k].svid,
								 (double) f->gps.sats[j].sat[k].c_n);
						}
					}
					fprintf(gps_file, "\n");
				}
			}

			if (kml_file) {
	                	snprintf(buf, sizeof (buf), kml_coord_fmt,
					f->gps.data[i].lon,
					f->gps.data[i].lat,
					baro + baro_offset,
					f->gps.data[i].alt,
					(f->gps.data[i].time - boost_start) / 100.0,
					nsat);
				fprintf(kml_file, "%s", buf);
				if (state_idx + 1 < f->state.num && f->state.data[state_idx + 1].time <= f->gps.data[i].time) {
					state_idx++;
					if (f->state.data[state_idx - 1].value != f->state.data[state_idx].value) {
						fprintf(kml_file, "%s", kml_placemark_end);
						fprintf(kml_file, kml_placemark_start,
							state_names[(int)f->state.data[state_idx].value],
							state_names[(int)f->state.data[state_idx].value]); 
						fprintf(kml_file, "%s", buf);
					}
				}
			}

		}
		if (kml_file)
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
	{ .name = "summary", .has_arg = 2, .val = 's' },
	{ .name = "detail", .has_arg = 2, .val = 'd' },
	{ .name = "plot", .has_arg = 2, .val = 'p' },
	{ .name = "raw", .has_arg = 2, .val = 'r' },
	{ .name = "gps", .has_arg = 2, .val = 'g' },
	{ .name = "kml", .has_arg = 2, .val = 'k' },
	{ .name = "all", .has_arg = 0, .val = 'a' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s\n"
		"\t[--all] [-a]\n"
		"\t[--summary=<summary-file>] [-s <summary-file>]\n"
		"\t[--detail=<detail-file] [-d <detail-file>]\n"
		"\t[--raw=<raw-file> -r <raw-file]\n"
		"\t[--plot=<plot-file> -p <plot-file>]\n"
		"\t[--gps=<gps-file> -g <gps-file>]\n"
		"\t[--kml=<kml-file> -k <kml-file>]\n"
		"\t{flight-log} ...\n", program);
	exit(1);
}

char *
replace_extension(char *file, char *extension)
{
	char	*slash;
	char	*dot;
	char	*new;
	int	newlen;

	slash = strrchr(file, '/');
	dot = strrchr(file, '.');
	if (!dot || (slash && dot < slash))
		dot = file + strlen(file);
	newlen = (dot - file) + strlen (extension) + 1;
	new = malloc (newlen);
	strncpy (new, file, dot - file);
	new[dot-file] = '\0';
	strcat (new, extension);
	return new;
}

FILE *
open_output(char *outname, char *inname, char *extension)
{
	char	*o;
	FILE	*out;

	if (outname)
		o = outname;
	else
		o = replace_extension(inname, extension);
	out = fopen(o, "w");
	if (!out) {
		perror (o);
		exit(1);
	}
	if (o != outname)
		free(o);
	return out;
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
	int			has_summary = 0;
	int			has_detail = 0;
	int			has_plot = 0;
	int			has_raw = 0;
	int			has_gps = 0;
	int			has_kml = 0;
	char			*this_plot_name = NULL;;

	while ((c = getopt_long(argc, argv, "s:d:p:r:g:k:a", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary_name = optarg;
			has_summary = 1;
			break;
		case 'd':
			detail_name = optarg;
			has_detail = 1;
			break;
		case 'p':
			plot_name = optarg;
			has_plot = 1;
			break;
		case 'r':
			raw_name = optarg;
			has_raw = 1;
			break;
		case 'g':
			gps_name = optarg;
			has_gps = 1;
			break;
		case 'k':
			kml_name = optarg;
			has_kml = 1;
			break;
		case 'a':
			has_summary = has_detail = has_plot = has_raw = has_gps = has_kml = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (!has_summary)
		summary_file = stdout;
	for (i = optind; i < argc; i++) {
		file = fopen(argv[i], "r");
		if (!file) {
			perror(argv[i]);
			ret++;
			continue;
		}
		if (has_summary && !summary_file)
			summary_file = open_output(summary_name, argv[i], ".summary");
		if (has_detail && !detail_file)
			detail_file = open_output(detail_name, argv[i], ".detail");
		if (has_plot) {
			if (plot_name)
				this_plot_name = plot_name;
			else
				this_plot_name = replace_extension(argv[i], ".plot");
		}
		if (has_raw && !raw_file)
			raw_file = open_output(raw_name, argv[i], ".raw");
		if (has_gps && !gps_file)
			gps_file = open_output(gps_name, argv[i], ".gps");
		if (has_kml && !kml_file)
			kml_file = open_output(kml_name, argv[i], ".kml");
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
		analyse_flight(raw, summary_file, detail_file, raw_file, this_plot_name, gps_file, kml_file);
		cc_flightraw_free(raw);
		if (has_summary && !summary_name) {
			fclose(summary_file); summary_file = NULL;
		}
		if (has_detail && !detail_name) {
			fclose(detail_file); detail_file = NULL;
		}
		if (this_plot_name && this_plot_name != plot_name) {
			free (this_plot_name); this_plot_name = NULL;
		}
		if (has_raw && !raw_name) {
			fclose(raw_file); raw_file = NULL;
		}
		if (has_gps && !gps_name) {
			fclose(gps_file); gps_file = NULL;
		}
		if (has_kml && !kml_name) {
			fclose(kml_file); kml_file = NULL;
		}
	}
	return ret;
}
