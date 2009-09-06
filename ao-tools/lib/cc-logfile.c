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
#include <stdio.h>
#include <stdlib.h>

static int
timedata_add(struct cc_timedata *data, double time, double value)
{
	struct cc_timedataelt	*newdata;
	int			newsize;
	if (data->size == data->num) {
		if (data->size == 0)
			newdata = malloc((newsize = 256) * sizeof (struct cc_timedataelt));
		else
			newdata = realloc (data->data, (newsize = data->size * 2)
					   * sizeof (struct cc_timedataelt));
		if (!newdata)
			return 0;
		data->size = newsize;
		data->data = newdata;
	}
	if (data->num && data->data[data->num-1].time > time)
		time += 65536;
	data->data[data->num].time = time;
	data->data[data->num].value = value;
	data->num++;
	return 1;
}

static void
timedata_free(struct cc_timedata *data)
{
	if (data->data)
		free(data->data);
}

static int
gpsdata_add(struct cc_gpsdata *data, struct cc_gpselt *elt)
{
	struct cc_gpselt	*newdata;
	int			newsize;
	if (data->size == data->num) {
		if (data->size == 0)
			newdata = malloc((newsize = 256) * sizeof (struct cc_gpselt));
		else
			newdata = realloc (data->data, (newsize = data->size * 2)
					   * sizeof (struct cc_gpselt));
		if (!newdata)
			return 0;
		data->size = newsize;
		data->data = newdata;
	}
	data->data[data->num] = *elt;
	data->num++;
	return 1;
}

static void
gpsdata_free(struct cc_gpsdata *data)
{
	if (data->data)
		free(data->data);
}

#define AO_LOG_FLIGHT		'F'
#define AO_LOG_SENSOR		'A'
#define AO_LOG_TEMP_VOLT	'T'
#define AO_LOG_DEPLOY		'D'
#define AO_LOG_STATE		'S'
#define AO_LOG_GPS_TIME		'G'
#define AO_LOG_GPS_LAT		'N'
#define AO_LOG_GPS_LON		'W'
#define AO_LOG_GPS_ALT		'H'
#define AO_LOG_GPS_SAT		'V'

#define AO_LOG_POS_NONE		(~0UL)

static int
read_eeprom(const char *line, struct cc_flightraw *f, double *ground_pres, int *ground_pres_count)
{
	char	type;
	int	tick;
	int	a, b;
	struct cc_gpselt	gps;
	int	serial;

	if (sscanf(line, "serial-number %u", &serial) == 1) {
		f->serial = serial;
		return 1;
	}
	if (sscanf(line, "%c %x %x %x", &type, &tick, &a, &b) != 4)
		return 0;
	switch (type) {
	case AO_LOG_FLIGHT:
		f->ground_accel = a;
		f->ground_pres = 0;
		f->flight = b;
		*ground_pres = 0;
		*ground_pres_count = 0;
		break;
	case AO_LOG_SENSOR:
		timedata_add(&f->accel, tick, a);
		timedata_add(&f->pres, tick, b);
		if (*ground_pres_count < 20) {
			*ground_pres += b;
			(*ground_pres_count)++;
			if (*ground_pres_count >= 20)
				f->ground_pres = *ground_pres / *ground_pres_count;
		}
		break;
	case AO_LOG_TEMP_VOLT:
		timedata_add(&f->temp, tick, a);
		timedata_add(&f->volt, tick, b);
		break;
	case AO_LOG_DEPLOY:
		timedata_add(&f->drogue, tick, a);
		timedata_add(&f->main, tick, b);
		break;
	case AO_LOG_STATE:
		timedata_add(&f->state, tick, a);
		break;
	case AO_LOG_GPS_TIME:
		gps.time = tick;
		break;
	case AO_LOG_GPS_LAT:
		gps.lat = ((int32_t) (a + (b << 16))) / 10000000.0;
		break;
	case AO_LOG_GPS_LON:
		gps.lon = ((int32_t) (a + (b << 16))) / 10000000.0;
		break;
	case AO_LOG_GPS_ALT:
		gps.alt = ((int32_t) (a + (b << 16)));
		gpsdata_add(&f->gps, &gps);
		break;
	case AO_LOG_GPS_SAT:
		break;
	default:
		return 0;
	}
	return 1;
}

static int
read_telem(const char *line, struct cc_flightraw *f)
{
	struct cc_telem		telem;
	struct cc_gpselt	gps;
	if (!cc_telem_parse(line, &telem))
		return 0;
	f->ground_accel = telem.ground_accel;
	f->ground_pres = telem.ground_pres;
	f->flight = 0;
	timedata_add(&f->accel, telem.tick, telem.flight_accel);
	timedata_add(&f->pres, telem.tick, telem.flight_pres);
	timedata_add(&f->temp, telem.tick, telem.temp);
	timedata_add(&f->volt, telem.tick, telem.batt);
	timedata_add(&f->drogue, telem.tick, telem.drogue);
	timedata_add(&f->main, telem.tick, telem.main);
	if (telem.gps.gps_locked) {
		gps.time = telem.tick;
		gps.lat = telem.gps.lat;
		gps.lon = telem.gps.lon;
		gps.alt = telem.gps.alt;
		gpsdata_add(&f->gps, &gps);
	}
	return 1;
}

struct cc_flightraw *
cc_log_read(FILE *file)
{
	struct cc_flightraw	*f;
	char			line[8192];
	double			ground_pres;
	int			ground_pres_count;

	f = calloc(1, sizeof (struct cc_flightraw));
	if (!f)
		return NULL;
	while (fgets(line, sizeof (line), file)) {
		if (read_eeprom(line, f, &ground_pres, &ground_pres_count))
			continue;
		if (read_telem(line, f))
			continue;
		fprintf (stderr, "invalid line: %s", line);
	}
	return f;
}

void
cc_flightraw_free(struct cc_flightraw *raw)
{
	timedata_free(&raw->accel);
	timedata_free(&raw->pres);
	timedata_free(&raw->temp);
	timedata_free(&raw->volt);
	timedata_free(&raw->main);
	timedata_free(&raw->drogue);
	timedata_free(&raw->state);
	gpsdata_free(&raw->gps);
	free(raw);
}
