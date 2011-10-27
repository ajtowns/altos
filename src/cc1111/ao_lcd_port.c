/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

static void
ao_lcd_port_delay(void)
{
	uint8_t	i;

	for (i = 0; i < 100; i++)
		ao_arch_nop();
}

void
ao_lcd_port_put_nibble(uint8_t rs, uint8_t nibble)
{
	P0 = (P0 & 0xf0) | (nibble & 0x0f);
	P1_1 = rs;
	P1_0 = 1;
	ao_lcd_port_delay();
	P1_0 = 0;
	ao_lcd_port_delay();
}

void
ao_lcd_port_init(void)
{
	/* LCD_E and LCD_RS are GPIO outputs */
	P1DIR |= 0x03;
	P1SEL &= ~0x03;

	/* LCD D4-D7 are GPIO outputs */
	P0DIR |= 0x0f;
	P0SEL &= ~0x0f;
}
