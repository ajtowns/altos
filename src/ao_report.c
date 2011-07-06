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

#include "ao.h"

#define BIT(i,x)    	   ((x) ? (1 << (i)) : 0)
#define MORSE1(a)          (1 | BIT(3,a))
#define MORSE2(a,b)        (2 | BIT(3,a) | BIT(4,b))
#define MORSE3(a,b,c)      (3 | BIT(3,a) | BIT(4,b) | BIT(5,c))
#define MORSE4(a,b,c,d)    (4 | BIT(3,a) | BIT(4,b) | BIT(5,c) | BIT(6,d))
#define MORSE5(a,b,c,d,e)  (5 | BIT(3,a) | BIT(4,b) | BIT(5,c) | BIT(6,d) | BIT(7,e))

static const uint8_t flight_reports[] = {
	MORSE3(0,0,0),		/* startup, 'S' */
	MORSE2(0,0),		/* idle 'I' */
	MORSE4(0,1,1,0),	/* pad 'P' */
	MORSE4(1,0,0,0),	/* boost 'B' */
	MORSE4(0,0,1,0),	/* fast 'F' */
	MORSE4(1,0,1,0),	/* coast 'C' */
	MORSE3(1,0,0),		/* drogue 'D' */
	MORSE2(1,1),		/* main 'M' */
	MORSE4(0,1,0,0),	/* landed 'L' */
	MORSE4(1,0,0,1),	/* invalid 'X' */
};

#if HAS_BEEP
#define low(time)	ao_beep_for(AO_BEEP_LOW, time)
#define mid(time)	ao_beep_for(AO_BEEP_MID, time)
#define high(time)	ao_beep_for(AO_BEEP_HIGH, time)
#else
#define low(time)	ao_led_for(AO_LED_GREEN, time)
#define mid(time)	ao_led_for(AO_LED_RED, time)
#define high(time)	ao_led_for(AO_LED_GREEN|AO_LED_RED, time)
#endif
#define pause(time)	ao_delay(time)

static __pdata enum ao_flight_state ao_report_state;

static void
ao_report_beep(void) __reentrant
{
	uint8_t r = flight_reports[ao_flight_state];
	uint8_t l = r & 7;

	if (!r)
		return;
	while (l--) {
		if (r & 8)
			mid(AO_MS_TO_TICKS(600));
		else
			mid(AO_MS_TO_TICKS(200));
		pause(AO_MS_TO_TICKS(200));
		r >>= 1;
	}
	pause(AO_MS_TO_TICKS(400));
}

static void
ao_report_digit(uint8_t digit) __reentrant
{
	if (!digit) {
		mid(AO_MS_TO_TICKS(500));
		pause(AO_MS_TO_TICKS(200));
	} else {
		while (digit--) {
			mid(AO_MS_TO_TICKS(200));
			pause(AO_MS_TO_TICKS(200));
		}
	}
	pause(AO_MS_TO_TICKS(300));
}

static void
ao_report_altitude(void)
{
	__pdata int16_t	agl = ao_max_height;
	__xdata uint8_t	digits[10];
	__pdata uint8_t ndigits, i;

	if (agl < 0)
		agl = 0;
	ndigits = 0;
	do {
		digits[ndigits++] = agl % 10;
		agl /= 10;
	} while (agl);

	for (;;) {
		ao_report_beep();
		i = ndigits;
		do
			ao_report_digit(digits[--i]);
		while (i != 0);
		pause(AO_SEC_TO_TICKS(5));
	}
}

#if HAS_IGNITE
static uint8_t
ao_report_igniter_ready(enum ao_igniter igniter)
{
	return ao_igniter_status(igniter) == ao_igniter_ready ? 1 : 0;
}

static void
ao_report_continuity(void) __reentrant
{
	uint8_t	c = (ao_report_igniter_ready(ao_igniter_drogue) |
		     (ao_report_igniter_ready(ao_igniter_main) << 1));
	if (c) {
		while (c--) {
			high(AO_MS_TO_TICKS(25));
			pause(AO_MS_TO_TICKS(100));
		}
	} else {
		c = 10;
		while (c--) {
			high(AO_MS_TO_TICKS(20));
			low(AO_MS_TO_TICKS(20));
		}
	}
	if (ao_log_full()) {
		pause(AO_MS_TO_TICKS(100));
		c = 2;
		while (c--) {
			low(AO_MS_TO_TICKS(100));
			mid(AO_MS_TO_TICKS(100));
			high(AO_MS_TO_TICKS(100));
			mid(AO_MS_TO_TICKS(100));
		}
	}
	c = 50;
	while (c-- && ao_flight_state == ao_flight_pad)
		pause(AO_MS_TO_TICKS(100));
}
#endif

void
ao_report(void)
{
	ao_report_state = ao_flight_state;
	for(;;) {
		if (ao_flight_state == ao_flight_landed)
			ao_report_altitude();
		ao_report_beep();
#if HAS_IGNITE
		if (ao_flight_state == ao_flight_idle)
			ao_report_continuity();
		while (ao_flight_state == ao_flight_pad)
			ao_report_continuity();
#endif
		__critical {
			while (ao_report_state == ao_flight_state)
				ao_sleep(DATA_TO_XDATA(&ao_flight_state));
			ao_report_state = ao_flight_state;
		}
	}
}

static __xdata struct ao_task ao_report_task;

void
ao_report_init(void)
{
	ao_add_task(&ao_report_task, ao_report, "report");
}
