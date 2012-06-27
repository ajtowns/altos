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

#include "ao.h"

__pdata uint16_t ao_led_enable;

void
ao_led_on(uint16_t colors)
{
	LED_PORT->bsrr = (colors & ao_led_enable);
}

void
ao_led_off(uint16_t colors)
{
	LED_PORT->bsrr = (uint32_t) (colors & ao_led_enable) << 16;
}

void
ao_led_set(uint16_t colors)
{
	uint16_t	on = colors & ao_led_enable;
	uint16_t	off = ~colors & ao_led_enable;

	LED_PORT->bsrr = off << 16 | on;
}

void
ao_led_toggle(uint16_t colors)
{
	LED_PORT->odr ^= (colors & ao_led_enable);
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

	stm_rcc.ahbenr |= (1 << LED_PORT_ENABLE);
	ao_led_enable = enable;
	LED_PORT->odr &= ~enable;
	for (bit = 0; bit < 16; bit++) {
		if (enable & (1 << bit)) {
			stm_moder_set(LED_PORT, bit, STM_MODER_OUTPUT);
			stm_otyper_set(LED_PORT, bit, STM_OTYPER_PUSH_PULL);
		}
	}
}
