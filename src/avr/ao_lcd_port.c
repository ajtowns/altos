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

#define LCD_PORT	PORTB
#define LCD_DDR		DDRB

#define PIN_RS		4
#define PIN_E		5
#define PIN_RW		6

static void
ao_lcd_port_set_bits(uint8_t bits)
{
#if 0
	printf("\tLCD data %x RS %d R/W %d E %d\n",
	       bits & 0xf,
	       (bits & (1 << PIN_RS)) ? 1 : 0,
	       (bits & (1 << PIN_RW)) ? 1 : 0,
	       (bits & (1 << PIN_E)) ? 1 : 0);
#endif
	LCD_PORT = bits;
#if 0
	ao_delay(1);
	if (bits & (1 << PIN_RW))
		printf("\tLCD input %x\n", PINB);
#endif
}

uint8_t
ao_lcd_port_get_nibble(uint8_t rs)
{
	uint8_t	data = (rs ? (1 << PIN_RS) : 0) | (1 << PIN_RW);
	uint8_t n;

	DDRB = (1 << PIN_RS) | (1 << PIN_E) | (1 << PIN_RW);
	ao_lcd_port_set_bits(data);
	ao_lcd_port_set_bits(data | (1 << PIN_E));
	n = PINB & 0xf;
	ao_lcd_port_set_bits(data);
	return n;
}

void
ao_lcd_port_put_nibble(uint8_t rs, uint8_t data)
{
	data = (data & 0xf) | (rs ? (1 << PIN_RS) : 0);
	DDRB = (0xf) | (1 << PIN_RS) | (1 << PIN_E) | (1 << PIN_RW);
	ao_lcd_port_set_bits(data);
	ao_lcd_port_set_bits(data | (1 << PIN_E));
	ao_lcd_port_set_bits(data);
}

void
ao_lcd_port_init(void)
{
	DDRB = (1 << PIN_RS) | (1 << PIN_E) | (1 << PIN_RW);
	PORTB = 0;
}
