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

#define AO_HERTZ	100

#define AO_ADC_RING	64
#define ao_adc_ring_next(n)	(((n) + 1) & (AO_ADC_RING - 1))
#define ao_adc_ring_prev(n)	(((n) - 1) & (AO_ADC_RING - 1))

/*
 * One set of samples read from the A/D converter
 */
struct ao_adc {
	uint16_t	tick;		/* tick when the sample was read */
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
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

struct ao_adc ao_adc_ring[AO_ADC_RING];
uint8_t ao_adc_head;

#define ao_led_on(l)
#define ao_led_off(l)
#define ao_timer_set_adc_interval(i)
#define ao_wakeup(wchan) ao_dump_state()
#define ao_cmd_register(c)
#define ao_usb_disable()
#define ao_telemetry_set_interval(x)
#define ao_rdf_set(rdf)
#define ao_packet_slave_start()

enum ao_igniter {
	ao_igniter_drogue = 0,
	ao_igniter_main = 1
};

void
ao_ignite(enum ao_igniter igniter)
{
	printf ("ignite %s\n", igniter == ao_igniter_drogue ? "drogue" : "main");
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

struct ao_adc ao_adc_static;

FILE *emulator_in;

void
ao_dump_state(void);

void
ao_sleep(void *wchan);

const char const * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "fast",
	"coast", "drogue", "main", "landed", "invalid"
};

struct ao_cmds {
	char		cmd;
	void		(*func)(void);
	const char	*help;
};


static int16_t altitude_table[2048] = {
#include "altitude.h"
};

int16_t
ao_pres_to_altitude(int16_t pres) __reentrant
{
	pres = pres >> 4;
	if (pres < 0) pres = 0;
	if (pres > 2047) pres = 2047;
	return altitude_table[pres];
}

int16_t
ao_altitude_to_pres(int16_t alt) __reentrant
{
	int16_t pres;

	for (pres = 0; pres < 2047; pres++)
		if (altitude_table[pres] <= alt)
			break;
	return pres << 4;
}

struct ao_config {
	uint16_t	main_deploy;
	int16_t		accel_plus_g;
	int16_t		accel_minus_g;
};

#define ao_config_get()

struct ao_config ao_config = { 250, 15937, 16467 };

#define DATA_TO_XDATA(x) (x)

#include "ao_flight.c"

void
ao_insert(void)
{
	ao_adc_ring[ao_adc_head] = ao_adc_static;
	ao_adc_head = ao_adc_ring_next(ao_adc_head);
	if (ao_flight_state != ao_flight_startup) {
		printf("time %g accel %d pres %d\n",
		       (double) ao_adc_static.tick / 100,
		       ao_adc_static.accel,
		       ao_adc_static.pres);
	}
}

static int	ao_records_read = 0;
static int	ao_eof_read = 0;
static int	ao_flight_ground_accel;
static int	ao_flight_started = 0;

void
ao_sleep(void *wchan)
{
	ao_dump_state();
	if (wchan == &ao_adc_ring) {
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
					printf ("no more data, exiting simulation\n");
					exit(0);
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
				ao_flight_started = 1;
				break;
			case 'S':
				break;
			case 'A':
				ao_adc_static.tick = tick;
				ao_adc_static.accel = a;
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
	if (ao_flight_state == ao_flight_startup)
		return;
	printf ("\t\t\t\t\t%s accel %g vel %g alt %d main %d\n",
		ao_state_names[ao_flight_state],
		(ao_ground_accel - ao_flight_accel) / COUNTS_PER_G * GRAVITY,
		(double) ao_flight_vel / 100 / COUNTS_PER_G * GRAVITY,
		ao_pres_to_altitude(ao_flight_pres) - ao_pres_to_altitude(ao_ground_pres),
		ao_pres_to_altitude(ao_main_pres) - ao_pres_to_altitude(ao_ground_pres));
	if (ao_flight_state == ao_flight_landed)
		exit(0);
}

int
main (int argc, char **argv)
{
	emulator_in = fopen (argv[1], "r");
	if (!emulator_in) {
		perror(argv[1]);
		exit(1);
	}
	ao_flight_init();
	ao_flight();
}
