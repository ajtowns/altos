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

#ifndef HAS_BEEP
#error Please define HAS_BEEP
#endif

#if !HAS_BEEP
#define ao_beep(x)
#endif

static void
ao_panic_delay(uint8_t n)
{
	uint8_t	i = 0, j = 0;

	while (n--)
		while (--j)
			while (--i)
				_asm nop _endasm;
}

void
ao_panic(uint8_t reason)
{
	uint8_t	n;

	__critical for (;;) {
		ao_panic_delay(20);
		for (n = 0; n < 5; n++) {
			ao_led_on(AO_LED_RED);
			ao_beep(AO_BEEP_HIGH);
			ao_panic_delay(1);
			ao_led_off(AO_LED_RED);
			ao_beep(AO_BEEP_LOW);
			ao_panic_delay(1);
		}
		ao_beep(AO_BEEP_OFF);
		ao_panic_delay(2);
#pragma disable_warning 126
		for (n = 0; n < reason; n++) {
			ao_led_on(AO_LED_RED);
			ao_beep(AO_BEEP_MID);
			ao_panic_delay(10);
			ao_led_off(AO_LED_RED);
			ao_beep(AO_BEEP_OFF);
			ao_panic_delay(10);
		}
	}
}
