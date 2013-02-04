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

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

FILE *emulator_in;
char *emulator_app;
char *emulator_name;
char *emulator_info;
uint8_t ao_flight_debug;

#define AO_FLIGHT_TEST

typedef int32_t alt_t;

#define AO_MS_TO_TICKS(ms)	((ms) / 10)

#define AO_LED_REPORT 0

static void ao_led_on(uint8_t led) {
}

static void ao_led_off(uint8_t led) {
}

static void ao_delay_until(uint16_t target) {
}

static uint16_t ao_time(void) {
	return 0;
}

#include "ao_microflight.c"
#include "ao_microkalman.c"
#include "ao_convert_pa.c"

uint16_t	now;
uint8_t		running;

void ao_log_micro_data() {
	running = 1;
}

void
ao_micro_report(void)
{
	if (running) {
		alt_t	ground = ao_pa_to_altitude(pa_ground);
		printf ("%6.3f %10d %10d %10d %10d %10d\n", now / 100.0,
			ao_pa_to_altitude(pa) - ground,
			ao_pa_to_altitude(ao_pa) - ground,
			ao_pa_to_altitude(pa_min) - ground,
			ao_pa_speed, ao_pa_accel);
	}
}

void
ao_micro_finish(void)
{
	ao_micro_report();
}

void
ao_pa_get(void)
{
	char	line[4096];
	char	*toks[128];
	char	*saveptr;
	int	t, ntok;
	static int	time_id;
	static int	pa_id;
	double		time;
	double		pressure;
	static double	last_time;
	static double	last_pressure;
	static int	been_here;
	static int	start_samples;
	static int	is_mp;
	static int	use_saved;

	if (been_here && start_samples < 100) {
		start_samples++;
		return;
	}
	ao_micro_report();
	if (use_saved) {
		pa = last_pressure;
		now = last_time;
		use_saved = 0;
//		printf ("use saved %d %d\n", now, pa);
		return;
	}
	for (;;) {
		if (!fgets(line, sizeof (line), emulator_in))
			exit(0);
		for (t = 0; t < 128; t++) {
			toks[t] = strtok_r(t ? NULL : line, ", ", &saveptr);
			if (!toks[t])
				break;
		}
		ntok = t;
		if (toks[0][0] == '#') {
			if (strcmp(toks[0],"#version") == 0) {
				for (t = 1; t < ntok; t++) {
					if (!strcmp(toks[t], "time"))
						time_id = t;
					if (!strcmp(toks[t],"pressure"))
						pa_id = t;
				}
			}
			continue;
		} else if (!strcmp(toks[0], "Time")) {
			time_id = 0;
			pa_id = 1;
			is_mp = 1;
			continue;
		}
		time = strtod(toks[time_id],NULL);
		pressure = strtod(toks[pa_id],NULL);
		time *= 100;
		if (been_here && time - last_time < 0.096 * 100)
			continue;
		if (is_mp && been_here) {
			double	avg_pressure = (pressure + last_pressure) / 2.0;
			double	avg_time = (time + last_time) / 2.0;

			now = avg_time;
			pa = avg_pressure;
//			printf ("new %d %d\n", now, pa);
			use_saved = 1;
		} else {
			now = floor (time + 0.5);
			pa = pressure;
		}
		last_pressure = pressure;
		last_time = time;
		been_here = 1;
		break;
	}
}

void
ao_dump_state(void)
{
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 0, .val = 's' },
	{ .name = "debug", .has_arg = 0, .val = 'd' },
	{ .name = "info", .has_arg = 1, .val = 'i' },
	{ 0, 0, 0, 0},
};

void run_flight_fixed(char *name, FILE *f, int summary, char *info)
{
	emulator_name = name;
	emulator_in = f;
	emulator_info = info;
	ao_microflight();
	ao_micro_finish();
}

int
main (int argc, char **argv)
{
	int	summary = 0;
	int	c;
	int	i;
	char	*info = NULL;

	emulator_app="baro";
	while ((c = getopt_long(argc, argv, "sdi:", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary = 1;
			break;
		case 'd':
			ao_flight_debug = 1;
			break;
		case 'i':
			info = optarg;
			break;
		}
	}

	if (optind == argc)
		run_flight_fixed("<stdin>", stdin, summary, info);
	else
		for (i = optind; i < argc; i++) {
			FILE	*f = fopen(argv[i], "r");
			if (!f) {
				perror(argv[i]);
				continue;
			}
			run_flight_fixed(argv[i], f, summary, info);
			fclose(f);
		}
	exit(0);
}
