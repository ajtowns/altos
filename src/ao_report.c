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

static const char * __xdata flight_reports[] = {
	"...",		/* startup, 'S' */
	"..",		/* idle 'I' */
	".--.",		/* pad 'P' */
	"-...",		/* boost 'B' */
	"..-.",		/* fast 'F' */
	"-.-.",		/* coast 'C' */
	"-..",		/* drogue 'D' */
	"--",		/* main 'M' */
	".-..",		/* landed 'L' */
	".-.-.-",	/* invalid */
};

#if 1
#define signal(time)	ao_beep_for(AO_BEEP_MID, time)
#else
#define signal(time)	ao_led_for(AO_LED_RED, time)
#endif
#define pause(time)	ao_delay(time)

static __xdata enum ao_flight_state ao_report_state;

static void
ao_report_beep(void) __reentrant
{
	char *r = flight_reports[ao_flight_state];
	char c;

	if (!r)
		return;
	while (c = *r++) {
		if (c == '.')
			signal(AO_MS_TO_TICKS(200));
		else
			signal(AO_MS_TO_TICKS(600));
		pause(AO_MS_TO_TICKS(200));
	}
	pause(AO_MS_TO_TICKS(400));
}

static void
ao_report_digit(uint8_t digit) __reentrant
{
	if (!digit) {
		signal(AO_MS_TO_TICKS(500));
		pause(AO_MS_TO_TICKS(200));
	} else {
		while (digit--) {
			signal(AO_MS_TO_TICKS(200));
			pause(AO_MS_TO_TICKS(200));
		}
	}
	pause(AO_MS_TO_TICKS(300));
}

static void
ao_report_altitude(void)
{
	__xdata int16_t	agl = ao_pres_to_altitude(ao_min_pres) - ao_pres_to_altitude(ao_ground_pres);
	__xdata uint8_t	digits[10];
	__xdata uint8_t ndigits, i;

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
			ao_beep_for(AO_BEEP_HIGH, AO_MS_TO_TICKS(25));
			pause(AO_MS_TO_TICKS(100));
		}
	} else {
		c = 10;
		while (c--) {
			ao_beep_for(AO_BEEP_HIGH, AO_MS_TO_TICKS(20));
			ao_beep_for(AO_BEEP_LOW, AO_MS_TO_TICKS(20));
		}
	}
	c = 50;
	while (c-- && ao_flight_state == ao_flight_pad)
		pause(AO_MS_TO_TICKS(100));
}

void
ao_report(void)
{
	ao_report_state = ao_flight_state;
	for(;;) {
		if (ao_flight_state == ao_flight_landed)
			ao_report_altitude();
		ao_report_beep();
		if (ao_flight_state == ao_flight_idle)
			ao_report_continuity();
		while (ao_flight_state == ao_flight_pad)
			ao_report_continuity();
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
