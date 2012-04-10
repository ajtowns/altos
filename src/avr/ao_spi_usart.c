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

/*
 * Atmega32u4 USART in MSPIM (master SPI mode)
 */

__xdata uint8_t	ao_spi_mutex;

/* Send bytes over SPI.
 *
 * This just polls; the SPI is set to go as fast as possible,
 * so using interrupts would take way too long
 */
void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	while (len--) {
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = *d++;
		while (!(UCSR1A & (1 << RXC1)));
		(void) UDR1;
	}
}

/* Receive bytes over SPI.
 *
 * Poll, sending zeros and reading data back
 */
void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	/* Clear any pending data */
	while (UCSR1A & (1 << RXC1))
		(void) UDR1;
	
	while (len--) {
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = 0;
		while (!(UCSR1A & (1 << RXC1)));
		*d++ = UDR1;
	}
}

/*
 * Initialize USART0 for SPI using config alt 2
 *
 * 	MO	P1_5
 * 	MI	P1_4
 * 	CLK	P1_3
 *
 * Chip select is the responsibility of the caller
 */

#define XCK1_DDR	DDRD
#define XCK1_PORT	PORTD
#define XCK1		PORTD5
#define XMS1_DDR	DDRE
#define XMS1_PORT	PORTE
#define XMS1		PORTE6

void
ao_spi_init(void)
{
	/* Ensure the USART is powered */
	PRR1 &= ~(1 << PRUSART1);

	/*
	 * Set pin directions
	 */
	XCK1_DDR |= (1 << XCK1);

	/* Clear chip select (which is negated) */
	XMS1_PORT |= (1 < XMS1);
	XMS1_DDR |= (1 << XMS1);

	/* Set baud register to zero (required before turning transmitter on) */
	UBRR1 = 0;

	UCSR1C = ((0x3 << UMSEL10) |	/* Master SPI mode */
		  (0 << UCSZ10) |	/* SPI mode 0 */
		  (0 << UCPOL1));	/* SPI mode 0 */

	/* Enable transmitter and receiver */
	UCSR1B = ((1 << RXEN1) |
		  (1 << TXEN1));

	/* It says that 0 is a legal value; we'll see... */
	UBRR1 = 0;
}
