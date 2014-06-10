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
	{ .name = "crc", .has_arg = 0, .val = 'c' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s\n"
		"\t{flight-log} ...\n", program);
	exit(1);
}

#define bool(b)	((b) ? "true" : "false")

int
main (int argc, char **argv)
{
	char	line[80];
	int c, i, ret;
	char *s;
	FILE *file;
	int serial;
	int ignore_crc = 0;
	while ((c = getopt_long(argc, argv, "c", options, NULL)) != -1) {
		switch (c) {
		case 'c':
			ignore_crc = 1;
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
 			char call[AO_MAX_CALLSIGN+1];
			char version[AO_MAX_VERSION+1];

			if (cc_telemetry_parse(line, &telem)) {
				int rssi = (int8_t) telem.generic.rssi / 2 - 74;

				printf ("serial %5d rssi %d status %02x tick %5d type %3d ",
					telem.generic.serial, rssi, telem.generic.status,
					telem.generic.tick, telem.generic.type);
				if (!ignore_crc && (telem.generic.status & (1 << 7)) == 0) {
					printf ("CRC error\n");
					continue;
				}
				switch (telem.generic.type) {
				case AO_TELEMETRY_SENSOR_TELEMETRUM:
				case AO_TELEMETRY_SENSOR_TELEMINI:
				case AO_TELEMETRY_SENSOR_TELENANO:
					printf ("state %1d accel %5d pres %5d ",
						telem.sensor.state, telem.sensor.accel, telem.sensor.pres);
					printf ("accel %6.2f speed %6.2f height %5d ",
						telem.sensor.acceleration / 16.0,
						telem.sensor.speed / 16.0,
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
				case AO_TELEMETRY_LOCATION:
					printf ("sats %d flags %s%s%s%s",
						telem.location.flags & 0xf,
						(telem.location.flags & (1 << 4)) ? "valid" : "invalid",
						(telem.location.flags & (1 << 5)) ? ",running" : "",
						(telem.location.flags & (1 << 6)) ? ",date" : "",
						(telem.location.flags & (1 << 7)) ? ",course" : "");
					printf (" alt %5d lat %12.7f lon %12.7f",
						telem.location.altitude,
						telem.location.latitude / 1e7,
						telem.location.longitude / 1e7);
					if ((telem.location.flags & (1 << 6)) != 0) {
						printf (" year %2d month %2d day %2d",
							telem.location.year,
							telem.location.month,
							telem.location.day);
						printf (" hour %2d minute %2d second %2d",
							telem.location.hour,
							telem.location.minute,
							telem.location.second);
					}
					printf (" pdop %3.1f hdop %3.1f vdop %3.1f mode %d",
						telem.location.pdop / 5.0,
						telem.location.hdop / 5.0,
						telem.location.vdop / 5.0,
						telem.location.mode);
					if ((telem.location.flags & (1 << 7)) != 0)
						printf (" ground_speed %6.2f climb_rate %6.2f course %d",
							telem.location.ground_speed / 100.0,
							telem.location.climb_rate / 100.0,
							telem.location.course * 2);
					printf ("\n");
					break;
				case AO_TELEMETRY_SATELLITE:
					printf ("sats %d", telem.satellite.channels);
					for (c = 0; c < 12 && c < telem.satellite.channels; c++) {
						printf (" sat %d svid %d c_n_1 %d",
							c,
							telem.satellite.sats[c].svid,
							telem.satellite.sats[c].c_n_1);
					}
					printf ("\n");
					break;
				case AO_TELEMETRY_MEGA_SENSOR:
					printf ("orient %3d accel %5d pres %9d temp %5d accel_x %5d accel_y %5d accel_z %5d gyro_x %5d gyro_y %5d gyro_z %5d mag_x %5d mag_y %5d mag_z %5d\n",
						telem.mega_sensor.orient,
						telem.mega_sensor.accel,
						telem.mega_sensor.pres,
						telem.mega_sensor.temp,
						telem.mega_sensor.accel_x,
						telem.mega_sensor.accel_y,
						telem.mega_sensor.accel_z,
						telem.mega_sensor.gyro_x,
						telem.mega_sensor.gyro_y,
						telem.mega_sensor.gyro_z,
						telem.mega_sensor.mag_x,
						telem.mega_sensor.mag_y,
						telem.mega_sensor.mag_z);
					break;
				case AO_TELEMETRY_MEGA_DATA:
					printf ("state %1d v_batt %5d v_pyro %5d ",
						telem.mega_data.state,
						telem.mega_data.v_batt,
						telem.mega_data.v_pyro);
					for (c = 0; c < 6; c++)
						printf ("s%1d %5d ", c,
							telem.mega_data.sense[c] |
							(telem.mega_data.sense[c] << 8));
					
					printf ("ground_pres %5d ground_accel %5d accel_plus %5d accel_minus %5d ",
						telem.mega_data.ground_pres,
						telem.mega_data.ground_accel,
						telem.mega_data.accel_plus_g,
						telem.mega_data.accel_minus_g);

					printf ("accel %6.2f speed %6.2f height %5d\n",
						telem.mega_data.acceleration / 16.0,
						telem.mega_data.speed / 16.0,
						telem.mega_data.height);

					break;
				case AO_TELEMETRY_METRUM_SENSOR:
					printf ("state %1d accel %5d pres %9d temp %6.2f acceleration %6.2f speed %6.2f height %5d v_batt %5d sense_a %5d sense_m %5d\n",
						telem.metrum_sensor.state,
						telem.metrum_sensor.accel,
						telem.metrum_sensor.pres,
						telem.metrum_sensor.temp / 100.0,
						telem.metrum_sensor.acceleration / 16.0,
						telem.metrum_sensor.speed / 16.0,
						telem.metrum_sensor.height,
						telem.metrum_sensor.v_batt,
						telem.metrum_sensor.sense_a,
						telem.metrum_sensor.sense_m);
					break;
				case AO_TELEMETRY_METRUM_DATA:
					printf ("ground_pres %9d ground_accel %5d accel_plus %5d accel_minus %5d\n",
						telem.metrum_data.ground_pres,
						telem.metrum_data.ground_accel,
						telem.metrum_data.accel_plus_g,
						telem.metrum_data.accel_minus_g);
					break;
				case AO_TELEMETRY_MINI:
					printf ("state %1d v_batt %5d sense_a %5d sense_m %5d pres %9d temp %6.2f acceleration %6.2f speed %6.2f height %5d ground_pres %9d\n",
						telem.mini.state,
						telem.mini.v_batt,
						telem.mini.sense_a,
						telem.mini.sense_m,
						telem.mini.pres,
						telem.mini.temp / 100.0,
						telem.mini.acceleration / 16.0,
						telem.mini.speed / 16.0,
						telem.mini.height,
						telem.mini.ground_pres);
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
