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

__pdata AO_PORT_TYPE ao_led_enable;

void
ao_led_on(AO_PORT_TYPE colors)
{
	lpc_gpio.pin[LED_PORT] |= colors;
}

void
ao_led_off(AO_PORT_TYPE colors)
{
	lpc_gpio.pin[LED_PORT] &= ~colors;
}

void
ao_led_set(AO_PORT_TYPE colors)
{
	AO_PORT_TYPE	on = colors & ao_led_enable;
	AO_PORT_TYPE	off = ~colors & ao_led_enable;

	ao_led_off(off);
	ao_led_on(on);
}

void
ao_led_toggle(AO_PORT_TYPE colors)
{
	lpc_gpio.pin[LED_PORT] ^= colors;
}

void
ao_led_for(AO_PORT_TYPE colors, uint16_t ticks) __reentrant
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(AO_PORT_TYPE enable)
{
	ao_led_enable = enable;
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_GPIO);
	lpc_gpio.dir[LED_PORT] |= enable;
}
