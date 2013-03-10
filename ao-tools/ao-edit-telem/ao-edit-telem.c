/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cc.h"

static const struct option options[] = {
	{ .name = "lat", .has_arg = 1, .val = 'L' },
	{ .name = "lon", .has_arg = 1, .val = 'l' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--lat <pad-lat>] [--lon <pad-lon>]\n"
		"\t{flight-log} ...\n", program);
	exit(1);
}

#define bool(b)	((b) ? "true" : "false")

struct telem_ent {
	struct telem_ent	*next;
	union ao_telemetry_all	telem;
};

static struct telem_ent *pad, **last = &pad;

static void
save_telem(union ao_telemetry_all *telem)
{
	struct telem_ent *t = malloc (sizeof *t);
	t->telem = *telem;
	t->next = NULL;
	*last = t;
	last = &t->next;
}

static void
dump_telem(union ao_telemetry_all *telem)
{
	char s[CC_TELEMETRY_BUFSIZE];

	cc_telemetry_unparse(telem, s);
	printf("%s\n", s);
}

double	pad_lat = 0, pad_lon = 0;
double	target_pad_lat = 0, target_pad_lon = 0;
double	lat_off = 0, lon_off = 0;
int pending = 1;

static void
dump_saved(void);

void
doit(union ao_telemetry_all *telem)
{
	double lat, lon;

	switch (telem->generic.type) {
	case AO_TELEMETRY_SENSOR_TELEMETRUM:
	case AO_TELEMETRY_SENSOR_TELEMINI:
	case AO_TELEMETRY_SENSOR_TELENANO:
		if (telem->sensor.state > ao_flight_pad && pad) {
			pending = 0;
			if (target_pad_lat)
				lat_off = target_pad_lat - pad_lat;
			if (target_pad_lon)
				lon_off = target_pad_lon - pad_lon;
			dump_saved();
		}
		break;
	case AO_TELEMETRY_LOCATION: {
		lat = telem->location.latitude / 1.0e7;
		lon = telem->location.longitude / 1.0e7;
		if (pending) {
			if (telem->location.flags & (1 << 4)) {
				if (pad_lat) {
					pad_lat = pad_lat - pad_lat / 32 + lat / 32.0;
					pad_lon = pad_lon - pad_lon / 32 + lon / 32.0;
				} else {
					pad_lat = lat;
					pad_lon = lon;
				}
			}
		} else {
			lat += lat_off;
			lon += lon_off;
			if (lat > 90)
				lat = 90;
			if (lat < -90)
				lat = -90;
			while (lon > 180)
				lon -= 360;
			while (lon < -180)
				lon += 360;
			telem->location.latitude = lat * 1.0e7;
			telem->location.longitude = lon * 1.0e7;
		}
		break;
	}
	}
}

static void
dump_saved(void)
{
	struct telem_ent	*t, *n;

	for (t = pad; t; t = n) {
		n = t->next;
		doit(&t->telem);
		dump_telem(&t->telem);
		free(t);
	}
	pad = NULL;
	last = &pad;
}

int
main (int argc, char **argv)
{
	char	line[80];
	int c, i, ret;
	char *s;
	FILE *file;
	int serial;
	while ((c = getopt_long(argc, argv, "l:L:", options, NULL)) != -1) {
		switch (c) {
		case 'L':
			target_pad_lat = strtod(optarg, NULL);
			break;
		case 'l':
			target_pad_lon = strtod(optarg, NULL);
			break;
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
		while (fgets(line, sizeof (line), file)) {
			union ao_telemetry_all telem;

			if (cc_telemetry_parse(line, &telem)) {
				if ((telem.generic.status & (1 << 7)) == 0) {
					dump_telem(&telem);
					continue;
				}
				doit (&telem);
				if (pending)
					save_telem(&telem);
				else
					dump_telem(&telem);
			}
		}
		fclose (file);

	}
	return ret;
}
