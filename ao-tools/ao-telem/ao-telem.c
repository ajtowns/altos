/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s\n"
		"\t{flight-log} ...\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	char	line[80];
	int c, i, ret;
	char *s;
	FILE *file;
	int serial;
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
		while (fgets(line, sizeof (line), file)) {
			union ao_telemetry_all telem;
			char call[AO_MAX_CALLSIGN+1];
			char version[AO_MAX_VERSION+1];

			if (cc_telemetry_parse(line, &telem)) {
				printf ("serial %5d tick %5d type %3d ",
					telem.generic.serial, telem.generic.tick, telem.generic.type);
				switch (telem.generic.type) {
				case AO_TELEMETRY_SENSOR_TELEMETRUM:
				case AO_TELEMETRY_SENSOR_TELEMINI:
				case AO_TELEMETRY_SENSOR_TELENANO:
					printf ("state %1d accel %5d pres %5d ",
						telem.sensor.state, telem.sensor.accel, telem.sensor.pres);
					printf ("accel %5d speed %5d height %5d ",
						telem.sensor.acceleration,
						telem.sensor.speed,
						telem.sensor.height);
					printf ("ground_pres %5d ground_accel %5d accel_plus %5d accel_minus %5d\n",
						telem.sensor.ground_pres,
						telem.sensor.ground_accel,
						telem.sensor.accel_plus_g,
						telem.sensor.accel_minus_g);
					break;
				case AO_TELEMETRY_CONFIGURATION:
					memcpy(call, telem.configuration.callsign, AO_MAX_CALLSIGN);
					memcpy(version, telem.configuration.version, AO_MAX_VERSION);
					call[AO_MAX_CALLSIGN] = '\0';
					version[AO_MAX_CALLSIGN] = '\0';
					printf ("device %3d flight %5d config %3d.%03d delay %2d main %4d",
						telem.configuration.device,
						telem.configuration.flight,
						telem.configuration.config_major,
						telem.configuration.config_minor,
						telem.configuration.apogee_delay,
						telem.configuration.main_deploy,
						telem.configuration.flight_log_max);
					printf (" call %8s version %8s\n", call, version);
					break;
				default:
					printf("\n");
				}
			}
		}
		fclose (file);

	}
	return ret;
}
