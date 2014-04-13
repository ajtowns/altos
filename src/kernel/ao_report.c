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
#include <ao_flight.h>
#include <ao_sample.h>

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

/*
 * Farnsworth spacing
 *
 * From: http://www.arrl.org/files/file/Technology/x9004008.pdf
 *
 *	c:	character rate in wpm
 *	s:	overall rate in wpm
 *	u:	unit rate (dit speed)
 *
 *	dit:			u
 *	dah:			3u
 *	intra-character-time:	u
 *
 * 	u = 1.2/c
 *
 * Because our clock runs at 10ms, we'll round up to 70ms for u, which
 * is about 17wpm
 *
 *	Farnsworth adds space between characters and
 *	words:
 *           60 c - 37.2 s
 *	Ta = -------------
 *                sc
 *
 *           3 Ta
 *	Tc = ----
 *            19
 *
 *           7 Ta
 *	Tw = ----
 *            19
 *
 *	Ta = total delay to add to the characters (31 units)
 *           of a standard 50-unit "word", in seconds
 *
 *      Tc = period between characters, in seconds
 *
 *	Tw = period between words, in seconds
 *
 * We'll use Farnsworth spacing with c=18 and s=12:
 *
 *	u = 1.2/18 = 0.0667
 *
 *	Ta = (60 * 17 - 37.2 * 12) / (17 * 12) = 2.812
 *
 *	Tc = 3 * Ta / 19 = .444
 *
 *	Tw = 1.036
 *
 * Note that the values below are all reduced by 10ms; that's because
 * the timer always adds a tick to make sure the task actually sleeps
 * at least as long as the argument.
 */

static void
ao_report_beep(void) __reentrant
{
	uint8_t r = flight_reports[ao_flight_state];
	uint8_t l = r & 7;

	if (!r)
		return;
	while (l--) {
		if (r & 8)
			mid(AO_MS_TO_TICKS(200));
		else
			mid(AO_MS_TO_TICKS(60));
		pause(AO_MS_TO_TICKS(60));
		r >>= 1;
	}
	pause(AO_MS_TO_TICKS(360));
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
ao_report_number(int16_t n)
{
	__xdata uint8_t	digits[10];
	__pdata uint8_t ndigits, i;

	if (n < 0)
		n = 0;
	ndigits = 0;
	do {
		digits[ndigits++] = n % 10;
		n /= 10;
	} while (n);

	i = ndigits;
	do
		ao_report_digit(digits[--i]);
	while (i != 0);
}

static void
ao_report_altitude(void)
{
	ao_report_number(ao_max_height);
}

#if HAS_BATTERY_REPORT
static void
ao_report_battery(void)
{
	__xdata struct ao_data packet;
	for (;;) {
		ao_data_get(&packet);
		if (packet.adc.v_batt != 0)
			break;
		ao_sleep(DATA_TO_XDATA(&ao_sample_data));
	}
	ao_report_number(ao_battery_decivolt(packet.adc.v_batt));
}
#endif

#if HAS_IGNITE_REPORT
static uint8_t
ao_report_igniter_ready(enum ao_igniter igniter)
{
	return ao_igniter_status(igniter) == ao_igniter_ready ? 1 : 0;
}

uint8_t
ao_report_igniter(void)
{
	return (ao_report_igniter_ready(ao_igniter_drogue) |
		     (ao_report_igniter_ready(ao_igniter_main) << 1));
}

static void
ao_report_continuity(void) __reentrant
{
	uint8_t	c;

#if !HAS_IGNITE
	if (!ao_igniter_present)
		return;
#endif
	c = ao_report_igniter();
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
#if HAS_LOG
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
#endif
}
#endif

void
ao_report(void)
{
	ao_report_state = ao_flight_state;
	for(;;) {
#if HAS_BATTERY_REPORT
		if (ao_flight_state == ao_flight_startup)
			ao_report_battery();
		else
#endif
			ao_report_beep();
		if (ao_flight_state == ao_flight_landed) {
			ao_report_altitude();
#if HAS_FLIGHT
			ao_delay(AO_SEC_TO_TICKS(5));
			continue;
#endif
		}
#if HAS_IGNITE_REPORT
		if (ao_flight_state == ao_flight_idle)
			ao_report_continuity();
		while (ao_flight_state == ao_flight_pad) {
			uint8_t	c;
			ao_report_continuity();
			c = 50;
			while (c-- && ao_flight_state == ao_flight_pad)
				pause(AO_MS_TO_TICKS(100));
		}
#endif

		while (ao_report_state == ao_flight_state)
			ao_sleep(DATA_TO_XDATA(&ao_flight_state));
		ao_report_state = ao_flight_state;
	}
}

static __xdata struct ao_task ao_report_task;

void
ao_report_init(void)
{
	ao_add_task(&ao_report_task, ao_report, "report");
}
