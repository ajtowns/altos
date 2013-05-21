/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

__pdata uint16_t ao_led_enable;

void
ao_led_on(uint16_t colors)
{
	lpc_gpio.pin[LED_PORT] |= colors;
}

void
ao_led_off(uint16_t colors)
{
	lpc_gpio.pin[LED_PORT] &= ~colors;
}

void
ao_led_set(uint16_t colors)
{
	uint16_t	on = colors & ao_led_enable;
	uint16_t	off = ~colors & ao_led_enable;

	ao_led_off(off);
	ao_led_on(on);
}

void
ao_led_toggle(uint16_t colors)
{
	lpc_gpio.pin[LED_PORT] ^= colors;
}

void
ao_led_for(uint16_t colors, uint16_t ticks) __reentrant
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(uint16_t enable)
{
	int	bit;

	ao_led_enable = enable;
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_GPIO);
	lpc_gpio.dir[LED_PORT] |= enable;
}
