/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
	".--.",		/* launchpad 'P' */
	"-...",		/* boost 'B' */
	"-.-.",		/* coast 'C' */
	".-",		/* apogee 'A' */
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

static void
ao_report_state(void)
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
}

static __xdata ao_report_wait;

void
ao_report_notify(void)
{
	ao_wakeup(&ao_report_wait);
}

void
ao_report(void)
{
	for(;;) {
		ao_report_state();
		ao_sleep(&ao_report_wait);
	}
}

static __xdata struct ao_task ao_report_task;

void
ao_report_init(void)
{
	ao_add_task(&ao_report_task, ao_report);
}
