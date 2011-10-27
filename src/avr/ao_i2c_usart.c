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
 * Atmega32u4 TWI master mode (I2C)
 */

__xdata uint8_t	ao_i2c_mutex;

/* Send bytes over I2C.
 *
 * This just polls; the I2C is set to go as fast as possible,
 * so using interrupts would take way too long
 */
void
ao_i2c_send(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	ao_mutex_get(&ao_i2c_mutex);
	while (len--) {
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = *d++;
		while (!(UCSR1A & (1 << RXC1)));
		(void) UDR1;
	}
	ao_mutex_put(&ao_i2c_mutex);
}

/* Receive bytes over I2C.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the I2C transmitter as that is what
 * clocks the data coming in.
 */
void
ao_i2c_recv(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	ao_mutex_get(&ao_i2c_mutex);
	while (len--) {
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = 0;
		while (!(UCSR1A & (1 << RXC1)));
		*d++ = UDR1;
	}
	ao_mutex_put(&ao_i2c_mutex);
}

#define XCK1_DDR	DDRD
#define XCK1_PORT	PORTD
#define XCK1		PORTD5
#define XMS1_DDR	DDRE
#define XMS1_PORT	PORTE
#define XMS1		PORTE6

void
ao_i2c_init(void)
{
	/* Ensure the TWI is powered */

	/*
	 * Set pin directions
	 */
	XCK1_DDR |= (1 << XCK1);

	/* Clear chip select (which is negated) */
	XMS1_PORT |= (1 < XMS1);
	XMS1_DDR |= (1 << XMS1);

	/* Set baud register to zero (required before turning transmitter on) */
	UBRR1 = 0;

	UCSR1C = ((0x3 << UMSEL10) |	/* Master I2C mode */
		  (0 << UCSZ10) |	/* I2C mode 0 */
		  (0 << UCPOL1));	/* I2C mode 0 */

	/* Enable transmitter and receiver */
	UCSR1B = ((1 << RXEN1) |
		  (1 << TXEN1));

	/* It says that 0 is a legal value; we'll see... */
	UBRR1 = 0;
}
