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
#include <ao_async.h>

#define AO_ASYNC_BAUD	38400l
#define AO_ASYNC_DELAY	(uint8_t) (1000000l / AO_ASYNC_BAUD)

void
ao_async_byte(uint8_t byte)
{
	uint8_t		b;
	uint16_t	w;

	/* start bit */

	/* start     data         stop */
	w = 0x001 | (byte << 1) | 0x000;

	for (b = 0; b < 10; b++) {
		ao_led_set((w & 1) << AO_LED_SERIAL);
		w >>= 1;
		ao_delay_us(26);
	}
}
