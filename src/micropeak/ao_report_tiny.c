/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <ao.h>

#define mid(time)	ao_led_for(AO_LED_REPORT, time)
#define pause(time)	ao_delay(time)

static void
ao_report_digit(uint8_t digit) __reentrant
{
	if (!digit) {
		mid(AO_MS_TO_TICKS(1000));
		pause(AO_MS_TO_TICKS(300));
	} else {
		while (digit--) {
			mid(AO_MS_TO_TICKS(300));
			pause(AO_MS_TO_TICKS(300));
		}
	}
	pause(AO_MS_TO_TICKS(1000));
}

void
ao_report_altitude(void)
{
	__pdata alt_t	agl = ao_max_height;
	static __xdata uint8_t	digits[11];
	__pdata uint8_t ndigits, i;

	if (agl < 0)
		agl = 0;
	ndigits = 0;
	do {
		digits[ndigits++] = agl % 10;
		agl /= 10;
	} while (agl);

	i = ndigits;
	do
		ao_report_digit(digits[--i]);
	while (i != 0);
}
