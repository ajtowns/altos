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

static uint16_t	ao_lcd_time = 3;

static __xdata uint8_t	ao_lcd_mutex;

static void
ao_lcd_delay(void)
{
	volatile uint16_t	count;

	for (count = 0; count < ao_lcd_time; count++)
		;
}

static void
ao_lcd_send_ins(uint8_t ins)
{
//	printf("send ins %02x\n", ins);
//	ao_lcd_wait_idle();
//	ao_delay(1);
	ao_lcd_delay();
	ao_lcd_port_put_nibble(0, ins >> 4);
	ao_lcd_port_put_nibble(0, ins & 0xf);
}

static void
ao_lcd_put_byte(uint8_t c)
{
//	printf ("send data %02x\n", c);
//	ao_lcd_wait_idle();
	ao_lcd_delay();
	ao_lcd_port_put_nibble(1, c >> 4);
	ao_lcd_port_put_nibble(1, c & 0x0f);
}

void
ao_lcd_putstring(char *string)
{
	char	c;

	ao_mutex_get(&ao_lcd_mutex);
	while ((c = (uint8_t) *string++))
		ao_lcd_put_byte((uint8_t) c);
	ao_mutex_put(&ao_lcd_mutex);
}

#define AO_LCD_POWER_CONTROL	0x54

void
ao_lcd_contrast_set(uint8_t contrast)
{
	ao_mutex_get(&ao_lcd_mutex);
	ao_lcd_send_ins(AO_LCD_POWER_CONTROL | ((contrast >> 4) & 0x3));
	ao_lcd_send_ins(0x70 | (contrast & 0xf));
	ao_mutex_put(&ao_lcd_mutex);
}

void
ao_lcd_cursor_on(void)
{
	ao_mutex_get(&ao_lcd_mutex);
	ao_lcd_send_ins(0x08 | 0x04 | 0x02 | 0x01);
	ao_mutex_put(&ao_lcd_mutex);
}

void
ao_lcd_cursor_off(void)
{
	ao_mutex_get(&ao_lcd_mutex);
	ao_lcd_send_ins(0x08 | 0x04);
	ao_mutex_put(&ao_lcd_mutex);
}

void
ao_lcd_clear(void)
{
	ao_mutex_get(&ao_lcd_mutex);
	ao_lcd_send_ins(0x01);
	ao_delay(1);
	/* Entry mode */
	ao_lcd_send_ins(0x04 | 0x02);
	ao_mutex_put(&ao_lcd_mutex);
}

void
ao_lcd_goto(uint8_t addr)
{
	ao_mutex_get(&ao_lcd_mutex);
	ao_lcd_send_ins(0x80 | addr);
	ao_lcd_send_ins(0x04 | 0x02);
	ao_mutex_put(&ao_lcd_mutex);
}

void
ao_lcd_start(void)
{
	/* get to 4bit mode */
	ao_lcd_port_put_nibble(0, 0x3);
	ao_lcd_port_put_nibble(0, 0x3);
	ao_lcd_port_put_nibble(0, 0x3);
	ao_lcd_port_put_nibble(0, 0x2);

	/* function set */
	ao_lcd_send_ins(0x28);
	/* function set, instruction table 1 */
	ao_lcd_send_ins(0x29);

	/* freq set */
	ao_lcd_send_ins(0x14);

	/* Power/icon/contrast control*/
	ao_lcd_send_ins(AO_LCD_POWER_CONTROL);

	/* Follower control */
	ao_lcd_send_ins(0x6d);
	ao_delay(AO_MS_TO_TICKS(200));

	/* contrast set */
	ao_lcd_contrast_set(0x18);

	/* Display on */
	ao_lcd_send_ins(0x08 | 0x04);

	/* Clear */
	ao_lcd_clear();
}

void
ao_lcd_init(void)
{
	ao_lcd_port_init();
}
