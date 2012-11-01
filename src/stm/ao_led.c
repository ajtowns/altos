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
#ifdef LED_PORT
	LED_PORT->bsrr = (colors & ao_led_enable);
#else
#ifdef LED_PORT_0
	LED_PORT_0->bsrr = ((colors & ao_led_enable) & LED_PORT_0_MASK) << LED_PORT_0_SHIFT;
#endif
#ifdef LED_PORT_1
	LED_PORT_1->bsrr = ((colors & ao_led_enable) & LED_PORT_1_MASK) << LED_PORT_1_SHIFT;
#endif
#endif
}

void
ao_led_off(uint16_t colors)
{
#ifdef LED_PORT
	LED_PORT->bsrr = (uint32_t) (colors & ao_led_enable) << 16;
#else
#ifdef LED_PORT_0
	LED_PORT_0->bsrr = ((uint32_t) (colors & ao_led_enable) & LED_PORT_0_MASK) << (LED_PORT_0_SHIFT + 16);
#endif
#ifdef LED_PORT_1
	LED_PORT_1->bsrr = ((uint32_t) (colors & ao_led_enable) & LED_PORT_1_MASK) << (LED_PORT_1_SHIFT + 16);
#endif
#endif
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
#ifdef LED_PORT
	LED_PORT->odr ^= (colors & ao_led_enable);
#else
#ifdef LED_PORT_0
	LED_PORT_0->odr ^= ((colors & ao_led_enable) & LED_PORT_0_MASK) << LED_PORT_0_SHIFT;
#endif
#ifdef LED_PORT_1
	LED_PORT_1->odr ^= ((colors & ao_led_enable) & LED_PORT_1_MASK) << LED_PORT_1_SHIFT;
#endif
#endif
}

void
ao_led_for(uint16_t colors, uint16_t ticks) __reentrant
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

#define init_led_pin(port, bit) do { \
		stm_moder_set(port, bit, STM_MODER_OUTPUT);		\
		stm_otyper_set(port, bit, STM_OTYPER_PUSH_PULL);	\
	} while (0)
	
void
ao_led_init(uint16_t enable)
{
	int	bit;

	ao_led_enable = enable;
#ifdef LED_PORT
	stm_rcc.ahbenr |= (1 << LED_PORT_ENABLE);
	LED_PORT->odr &= ~enable;
#else
#ifdef LED_PORT_0
	stm_rcc.ahbenr |= (1 << LED_PORT_0_ENABLE);
	LED_PORT_0->odr &= ~((enable & ao_led_enable) & LED_PORT_0_MASK) << LED_PORT_0_SHIFT;
#endif
#ifdef LED_PORT_1
	stm_rcc.ahbenr |= (1 << LED_PORT_1_ENABLE);
	LED_PORT_1->odr &= ~((enable & ao_led_enable) & LED_PORT_1_MASK) << LED_PORT_1_SHIFT;
#endif
#endif
	for (bit = 0; bit < 16; bit++) {
		if (enable & (1 << bit)) {
#ifdef LED_PORT
			init_led_pin(LED_PORT, bit);
#else
#ifdef LED_PORT_0
			if (LED_PORT_0_MASK & (1 << bit))
				init_led_pin(LED_PORT_0, bit + LED_PORT_0_SHIFT);
#endif
#ifdef LED_PORT_1
			if (LED_PORT_1_MASK & (1 << bit))
				init_led_pin(LED_PORT_1, bit + LED_PORT_1_SHIFT);
#endif
#endif
		}
	}
}
