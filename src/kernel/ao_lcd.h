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

#ifndef _AO_LCD_H_
#define _AO_LCD_H_

/* ao_lcd.c */
  
void
ao_lcd_putchar(uint8_t d);

void
ao_lcd_putstring(char *string);

void
ao_lcd_contrast_set(uint8_t contrast);

void
ao_lcd_clear(void);

void
ao_lcd_cursor_on(void);

void
ao_lcd_cursor_off(void);

#define AO_LCD_ADDR(row,col)	((row << 6) | (col))

void
ao_lcd_goto(uint8_t addr);

void
ao_lcd_start(void);

void
ao_lcd_init(void);

/* ao_lcd_port.c */

void
ao_lcd_port_put_nibble(uint8_t rs, uint8_t d);

void
ao_lcd_port_init(void);

#endif /* _AO_LCD_H_ */
