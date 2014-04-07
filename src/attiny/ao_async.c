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
	uint8_t		v;
	uint8_t		bit;
	uint8_t		w_hi, w_lo;

	/*    start           data           stop */
	w = (0x000 << 0) | (byte << 1) | (0x001 << 9);

	w_hi = w >> 8;
	w_lo = w;

	ao_arch_block_interrupts();

	/* Ok, this is a bit painful.
	 * We need this loop to be precisely timed, which
	 * means knowing exactly how many instructions will
	 * be executed for each bit. It's easy to do that by
	 * compiling the C code and looking at the output,
	 * but we need this code to work even if the compiler
	 * changes. So, just hand-code the whole thing
	 */

	asm volatile (
		"	ldi	%[b], 10\n"		// loop value
		"loop:\n"
		"	in	%[v], %[port]\n"	// read current value
		"	andi	%[v], %[led_mask]\n"	// mask to clear LED bit
		"	mov	%[bit], %[w_lo]\n"	// get current data byte
		"	andi	%[bit], 0x01\n"		// get current data bit
#if AO_LED_SERIAL >= 1
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 2
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 3
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 4
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 5
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 6
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
#if AO_LED_SERIAL >= 7
		"	add	%[bit],%[bit]\n"	// shift by one
#else
		"	nop\n"
#endif
		"	or	%[v], %[bit]\n"		// add to register
		"	out	%[port], %[v]\n"	// write current value
		"	lsr	%[w_hi]\n"		// shift data
		"	ror	%[w_lo]\n"		//  ...
		"	nop\n"
		"	nop\n"
		"	nop\n"
		"	nop\n"
		"	nop\n"

		"	nop\n"
		"	nop\n"
		"	nop\n"
		"	subi	%[b], 1\n"		// decrement bit count
		"	brne	loop\n"			// jump back to top
		: [v]        "=&r" (v),
		  [bit]	     "=&r" (bit),
		  [b]	     "=&r" (b),
		  [w_lo]     "+r" (w_lo),
		  [w_hi]     "+r" (w_hi)
		: [port]     "I"  (_SFR_IO_ADDR(LED_PORT)),
		  [led_mask] "M"  ((~(1 << AO_LED_SERIAL)) & 0xff)
		);

#if 0
	/*
	 * Here's the equivalent C code to document
	 * what the above assembly code does
	 */
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
		asm volatile ("nop");
	}
#endif
	ao_arch_release_interrupts();
}
