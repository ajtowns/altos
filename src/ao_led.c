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

__pdata uint8_t ao_led_enable;

void
ao_led_on(uint8_t colors)
{
	P1 |= (colors & ao_led_enable);
}

void
ao_led_off(uint8_t colors)
{
	P1 &= ~(colors & ao_led_enable);
}

void
ao_led_set(uint8_t colors)
{
	P1 = (P1 & ~(ao_led_enable)) | (colors & ao_led_enable);
}

void
ao_led_toggle(uint8_t colors)
{
	P1 ^= (colors & ao_led_enable);
}

void
ao_led_for(uint8_t colors, uint16_t ticks) __reentrant
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(uint8_t enable)
{
	ao_led_enable = enable;
	P1SEL &= ~enable;
	P1 &= ~enable;
	P1DIR |= enable;
}
