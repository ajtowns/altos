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

#define AO_HERTZ	100

#define AO_ADC_RING	64
#define ao_adc_ring_next(n)	(((n) + 1) & (AO_ADC_RING - 1))
#define ao_adc_ring_prev(n)	(((n) - 1) & (AO_ADC_RING - 1))

#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))

/*
 * One set of samples read from the A/D converter
 */
struct ao_adc {
	uint16_t	tick;		/* tick when the sample was read */
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
	int16_t		pres_real;	/* unclipped */
	int16_t		temp;		/* temperature sensor */
	int16_t		v_batt;		/* battery voltage */
	int16_t		sense_d;	/* drogue continuity sense */
	int16_t		sense_m;	/* main continuity sense */
};

#define __pdata
#define __data
#define __xdata
#define __code
#define __reentrant

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

/*
 * Above this height, the baro sensor doesn't work
 */
#define AO_MAX_BARO_HEIGHT	12000
#define AO_BARO_SATURATE	13000
#define AO_MIN_BARO_VALUE	ao_altitude_to_pres(AO_BARO_SATURATE)

/*
 * Above this speed, baro measurements are unreliable
 */
#define AO_MAX_BARO_SPEED	200

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

enum ao_flight_state {
	ao_flight_startup = 0,
	ao_flight_idle = 1,
	ao_flight_pad = 2,
	ao_flight_boost = 3,
	ao_flight_fast = 4,
	ao_flight_coast = 5,
	ao_flight_drogue = 6,
	ao_flight_main = 7,
	ao_flight_landed = 8,
	ao_flight_invalid = 9
};

extern enum ao_flight_state ao_flight_state;

#define FALSE 0
#define TRUE 1

struct ao_adc ao_adc_ring[AO_ADC_RING];
uint8_t ao_adc_head;
int	ao_summary = 0;

#define ao_led_on(l)
#define ao_led_off(l)
#define ao_timer_set_adc_interval(i)
#define ao_wakeup(wchan) ao_dump_state()
#define ao_cmd_register(c)
#define ao_usb_disable()
#define ao_telemetry_set_interval(x)
#define ao_rdf_set(rdf)
#define ao_packet_slave_start()
#define ao_packet_slave_stop()

enum ao_igniter {
	ao_igniter_drogue = 0,
	ao_igniter_main = 1
};

struct ao_adc ao_adc_static;

int	drogue_height;
double	drogue_time;
int	main_height;
double	main_time;

int	tick_offset;

static int32_t	ao_k_height;

void
ao_ignite(enum ao_igniter igniter)
{
	double time = (double) (ao_adc_static.tick + tick_offset) / 100;

	if (igniter == ao_igniter_drogue) {
		drogue_time = time;
		drogue_height = ao_k_height >> 16;
	} else {
		main_time = time;
		main_height = ao_k_height >> 16;
	}
}

struct ao_task {
	int dummy;
};

#define ao_add_task(t,f,n)

#define ao_log_start()
#define ao_log_stop()

#define AO_MS_TO_TICKS(ms)	((ms) / 10)
#define AO_SEC_TO_TICKS(s)	((s) * 100)

#define AO_FLIGHT_TEST

int	ao_flight_debug;

FILE *emulator_in;
char *emulator_app;
char *emulator_name;
double emulator_error_max = 4;

void
ao_dump_state(void);

void
ao_sleep(void *wchan);

const char const * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "fast",
	"coast", "drogue", "main", "landed", "invalid"
};

struct ao_cmds {
	void		(*func)(void);
	const char	*help;
};

#include "ao_convert.c"

struct ao_config {
	uint16_t	main_deploy;
	int16_t		accel_plus_g;
	int16_t		accel_minus_g;
};

#define ao_config_get()

struct ao_config ao_config;

#define DATA_TO_XDATA(x) (x)

#define HAS_FLIGHT 1
#define HAS_ADC 1
#define HAS_USB 1
#define HAS_GPS 1
#ifndef HAS_ACCEL
#define HAS_ACCEL 1
#define HAS_ACCEL_REF 0
#endif

#define GRAVITY 9.80665
extern int16_t ao_ground_accel, ao_flight_accel;
extern int16_t ao_accel_2g;

extern uint16_t	ao_sample_tick;

extern int16_t	ao_sample_height;
extern int16_t	ao_sample_accel;
extern int32_t	ao_accel_scale;
extern int16_t	ao_ground_height;
extern int16_t	ao_sample_alt;

int ao_sample_prev_tick;
uint16_t	prev_tick;

#include "ao_kalman.c"
#include "ao_sample.c"
#include "ao_flight.c"

#define to_double(f)	((f) / 65536.0)

static int	ao_records_read = 0;
static int	ao_eof_read = 0;
static int	ao_flight_ground_accel;
static int	ao_flight_started = 0;
static int	ao_test_max_height;
static double	ao_test_max_height_time;
static int	ao_test_main_height;
static double	ao_test_main_height_time;

void
ao_test_exit(void)
{
	double	drogue_error;
	double	main_error;

	if (!ao_test_main_height_time) {
		ao_test_main_height_time = ao_test_max_height_time;
		ao_test_main_height = ao_test_max_height;
	}
	drogue_error = fabs(ao_test_max_height_time - drogue_time);
	main_error = fabs(ao_test_main_height_time - main_time);
	if (drogue_error > emulator_error_max || main_error > emulator_error_max) {
		printf ("%s %s\n",
			emulator_app, emulator_name);
		printf ("\tApogee error %g\n", drogue_error);
		printf ("\tMain error %g\n", main_error);
		printf ("\tActual: apogee: %d at %7.2f main: %d at %7.2f\n",
			ao_test_max_height, ao_test_max_height_time,
			ao_test_main_height, ao_test_main_height_time);
		printf ("\tComputed: apogee: %d at %7.2f main: %d at %7.2f\n",
			drogue_height, drogue_time, main_height, main_time);
		exit (1);
	}
	exit(0);
}

void
ao_insert(void)
{
	double	time;

	ao_adc_ring[ao_adc_head] = ao_adc_static;
	ao_adc_head = ao_adc_ring_next(ao_adc_head);
	if (ao_flight_state != ao_flight_startup) {
		double	height = ao_pres_to_altitude(ao_adc_static.pres_real) - ao_ground_height;
		double  accel = ((ao_flight_ground_accel - ao_adc_static.accel) * GRAVITY * 2.0) /
			(ao_config.accel_minus_g - ao_config.accel_plus_g);

		if (!tick_offset)
			tick_offset = -ao_adc_static.tick;
		if ((prev_tick - ao_adc_static.tick) > 0x400)
			tick_offset += 65536;
		prev_tick = ao_adc_static.tick;
		time = (double) (ao_adc_static.tick + tick_offset) / 100;

		if (ao_test_max_height < height) {
			ao_test_max_height = height;
			ao_test_max_height_time = time;
		}
		if (height > ao_config.main_deploy) {
			ao_test_main_height_time = time;
			ao_test_main_height = height;
		}

		if (!ao_summary) {
			printf("%7.2f height %g accel %g state %s k_height %g k_speed %g k_accel %g drogue %d main %d error %d\n",
			       time,
			       height,
			       accel,
			       ao_state_names[ao_flight_state],
			       ao_k_height / 65536.0,
			       ao_k_speed / 65536.0 / 16.0,
			       ao_k_accel / 65536.0 / 16.0,
			       drogue_height,
			       main_height,
			       ao_error_h_sq_avg);
			if (ao_flight_state == ao_flight_landed)
				ao_test_exit();
		}
	}
}

void
ao_sleep(void *wchan)
{
	if (wchan == &ao_adc_head) {
		char		type;
		uint16_t	tick;
		uint16_t	a, b;
		int		ret;
		char		line[1024];
		char		*saveptr;
		char		*l;
		char		*words[64];
		int		nword;

		for (;;) {
			if (ao_records_read > 2 && ao_flight_state == ao_flight_startup)
			{
				ao_adc_static.accel = ao_flight_ground_accel;
				ao_insert();
				return;
			}

			if (!fgets(line, sizeof (line), emulator_in)) {
				if (++ao_eof_read >= 1000) {
					if (!ao_summary)
						printf ("no more data, exiting simulation\n");
					ao_test_exit();
				}
				ao_adc_static.tick += 10;
				ao_insert();
				return;
			}
			l = line;
			for (nword = 0; nword < 64; nword++) {
				words[nword] = strtok_r(l, " \t\n", &saveptr);
				l = NULL;
				if (words[nword] == NULL)
					break;
			}
			if (nword == 4) {
				type = words[0][0];
				tick = strtoul(words[1], NULL, 16);
				a = strtoul(words[2], NULL, 16);
				b = strtoul(words[3], NULL, 16);
			} else if (nword >= 6 && strcmp(words[0], "Accel") == 0) {
				ao_config.accel_plus_g = atoi(words[3]);
				ao_config.accel_minus_g = atoi(words[5]);
			} else if (nword >= 4 && strcmp(words[0], "Main") == 0) {
				ao_config.main_deploy = atoi(words[2]);
			} else if (nword >= 36 && strcmp(words[0], "CALL") == 0) {
				tick = atoi(words[10]);
				if (!ao_flight_started) {
					type = 'F';
					a = atoi(words[26]);
					ao_flight_started = 1;
				} else {
					type = 'A';
					a = atoi(words[12]);
					b = atoi(words[14]);
				}
			}
			if (type != 'F' && !ao_flight_started)
				continue;

			switch (type) {
			case 'F':
				ao_flight_ground_accel = a;
				if (ao_config.accel_plus_g == 0) {
					ao_config.accel_plus_g = a;
					ao_config.accel_minus_g = a + 530;
				}
				if (ao_config.main_deploy == 0)
					ao_config.main_deploy = 250;
				ao_flight_started = 1;
				break;
			case 'S':
				break;
			case 'A':
				ao_adc_static.tick = tick;
				ao_adc_static.accel = a;
				ao_adc_static.pres_real = b;
				if (b < AO_MIN_BARO_VALUE)
					b = AO_MIN_BARO_VALUE;
				ao_adc_static.pres = b;
				ao_records_read++;
				ao_insert();
				return;
			case 'T':
				ao_adc_static.tick = tick;
				ao_adc_static.temp = a;
				ao_adc_static.v_batt = b;
				break;
			case 'D':
			case 'G':
			case 'N':
			case 'W':
			case 'H':
				break;
			}
		}

	}
}
#define COUNTS_PER_G 264.8

void
ao_dump_state(void)
{
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 0, .val = 's' },
	{ .name = "debug", .has_arg = 0, .val = 'd' },
	{ 0, 0, 0, 0},
};

void run_flight_fixed(char *name, FILE *f, int summary)
{
	emulator_name = name;
	emulator_in = f;
	ao_summary = summary;
	ao_flight_init();
	ao_flight();
}

int
main (int argc, char **argv)
{
	int	summary = 0;
	int	c;
	int	i;

#if HAS_ACCEL
	emulator_app="full";
#else
	emulator_app="baro";
#endif
	while ((c = getopt_long(argc, argv, "sd", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary = 1;
			break;
		case 'd':
			ao_flight_debug = 1;
			break;
		}
	}

	if (optind == argc)
		run_flight_fixed("<stdin>", stdin, summary);
	else
		for (i = optind; i < argc; i++) {
			FILE	*f = fopen(argv[i], "r");
			if (!f) {
				perror(argv[i]);
				continue;
			}
			run_flight_fixed(argv[i], f, summary);
			fclose(f);
		}
}
