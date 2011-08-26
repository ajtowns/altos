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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define ao_wakeup(wchan) ao_dump_state(wchan)
#define ao_cmd_register(c)
#define ao_usb_disable()
#define ao_telemetry_set_interval(x)
#define ao_delay(x)

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
ao_dump_state(void *wchan);

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


struct ao_config {
	uint16_t	main_deploy;
	int16_t		accel_zero_g;
};

#define ao_config_get()

struct ao_config ao_config = { 250, 16000 };
