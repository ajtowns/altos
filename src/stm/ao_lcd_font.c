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

static const uint16_t ao_lcd_font[] = {
#include "ao_lcd_font.h"
};

/*
	 ----- 		0
	|\ | /|		1 2 3 4 5
	| \|/ | @		14
	 -- -- 		6 7
	| /|\ |		8 9 10 11 12	
	|/ | \| @		14
	 ----- 		13
	        @		15
*/

static const uint8_t	ao_bit_src[] = {
	 8,  7,  5,  6,
	13, 12,  0,  1,
	10, 14,  4,  9,
	11, 15,  3,  2
};

static const uint8_t	ao_bit_dst[6][4] = {
	{  0,  1, 28, 29 },
	{  2,  7, 26, 27 },
	{  8,  9, 24, 25 },
	{ 10, 11, 20, 21 },
	{ 12, 13, 18, 19 },
	{ 14, 15, 17, 16 },
};

static void
ao_draw_glyph(uint8_t pos, uint16_t glyph) {
	uint8_t		col, seg, src, dst;
	uint32_t	ram;

	for (col = 0; col < 4; col++) {
		for (seg = 0; seg < 4; seg++) {
			src = ao_bit_src[(col << 2) | seg];
			dst = ao_bit_dst[pos][seg];
			ram = stm_lcd.ram[col * 2];
			ram &= ~(1 << dst);
			ram |= (uint32_t) ((glyph >> src) & 1) << dst;
			stm_lcd.ram[col*2] = ram;
		}
	}
}

#define AO_LCD_FONT_COLON	(1 << 14)
#define AO_LCD_FONT_DECIMAL	(1 << 15)

void
ao_lcd_font_char(uint8_t pos, char c, uint16_t flags) {
	if (pos > 5)
		pos = 5;
	if (c < ' ' || c > 127)
		c = 127;
	ao_draw_glyph(pos, ao_lcd_font[c - ' '] | flags);
}

void
ao_lcd_font_string(char *s) {
	uint8_t	pos = 0;
	uint16_t flags;
	char c;

	while (pos < 6 && (c = *s++)) {
		flags = 0;
		for (;;) {
			if (*s == ':')
				flags |= AO_LCD_FONT_COLON;
			else if (*s == '.')
				flags |= AO_LCD_FONT_DECIMAL;
			else
				break;
			s++;
		}
		ao_lcd_font_char(pos++, c, flags);
	}
	while (pos < 6)
		ao_lcd_font_char(pos++, ' ', 0);
	stm_lcd.sr = (1 << STM_LCD_SR_UDR);
}

static void
ao_lcd_font_text(void)
{
	char	string[20];
	uint8_t	c = 0;
	ao_cmd_white();
	while (ao_cmd_lex_c != '\n' && c < sizeof (string) - 1) {
		string[c++] = ao_cmd_lex_c;
		ao_cmd_lex();
	}
	string[c++] = '\0';
	ao_lcd_font_string(string);
}

const struct ao_cmds ao_lcd_font_cmds[] = {
	{ ao_lcd_font_text,	"t <string>\0Write <string> to LCD" },
	{ 0, NULL }
};

void
ao_lcd_font_init(void)
{
	ao_cmd_register(ao_lcd_font_cmds);
}
	
