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
		"\t{flight.mega} ...\n", program);
	exit(1);
}

#define bool(b)	((b) ? "true" : "false")

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


#define NUM_STATE	(sizeof state_names/sizeof state_names[0])

int
main (int argc, char **argv)
{
	char	line[256];
	int c, i, ret, j;
	char *s;
	FILE *file;
	int serial;
	const char *state;
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
			struct ao_log_mega	log;

			if (cc_mega_parse(line, &log)) {
				if (log.is_config) {
					printf ("kind %d\n", log.u.config_int.kind);
				} else {
					printf ("tick %5d ", log.tick);
					switch (log.type) {
					case AO_LOG_FLIGHT:
						printf ("flight %5u ground_accel %d ground_pres %u\n",
							log.u.flight.flight,
							log.u.flight.ground_accel,
							log.u.flight.ground_pres);
						break;
					case AO_LOG_STATE:
						if (log.u.state.state < NUM_STATE)
							state = state_names[log.u.state.state];
						else
							state = "invalid";
						printf ("state %d (%s)\n", log.u.state.state, state);
						break;
					case AO_LOG_SENSOR:
						printf ("p %9u t %9u ax %6d ay %6d az %6d gx %6d gy %6d gz %6d mx %6d my %6d mz %6d a %6d\n",
							log.u.sensor.pres,
							log.u.sensor.temp,
							log.u.sensor.accel_x,
							log.u.sensor.accel_y,
							log.u.sensor.accel_z,
							log.u.sensor.gyro_x,
							log.u.sensor.gyro_y,
							log.u.sensor.gyro_z,
							log.u.sensor.mag_x,
							log.u.sensor.mag_y,
							log.u.sensor.mag_z,
							log.u.sensor.accel);
						break;
					case AO_LOG_TEMP_VOLT:
						printf ("batt %6d pbatt %6d n_sense %d",
							log.u.volt.v_batt,
							log.u.volt.v_pbatt,
							log.u.volt.n_sense);
						for (j = 0; j < log.u.volt.n_sense; j++) {
							printf (" s%d %6d",
								j, log.u.volt.sense[j]);
						}
						printf ("pyro %04x\n", log.u.volt.pyro);
						printf ("\n");
						break;
					default:
						printf ("type %c\n", log.type, log.tick);
						break;
					}
				}
			}
		}
		fclose (file);

	}
	return ret;
}
