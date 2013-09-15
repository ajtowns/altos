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
#include <ao_async.h>

#define AO_ASYNC_BAUD	38400l
#define AO_ASYNC_DELAY	(uint8_t) (1000000l / AO_ASYNC_BAUD)

#define LED_PORT	PORTB

void
ao_async_start(void)
{
	LED_PORT |= (1 << AO_LED_SERIAL);
}

void
ao_async_stop(void)
{
	LED_PORT &= ~(1 << AO_LED_SERIAL);
}

void
ao_async_byte(uint8_t byte)
{
	uint8_t		b;
	uint16_t	w;

	/*    start           data           stop */
	w = (0x000 << 0) | (byte << 1) | (0x001 << 9);

	ao_arch_block_interrupts();
	for (b = 0; b < 10; b++) {
		uint8_t	v = LED_PORT & ~(1 << AO_LED_SERIAL);
		v |= (w & 1) << AO_LED_SERIAL;
		LED_PORT = v;
		w >>= 1;

		/* Carefully timed to hit around 9600 baud */
		asm volatile ("nop");
		asm volatile ("nop");

		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");

		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
	}
	ao_arch_release_interrupts();
}
