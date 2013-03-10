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

uint8_t
ao_spi_slave_recv(void *buf, uint16_t len)
{
	uint8_t *b = buf;
	while (len--) {
		while (!(SPSR & (1 << SPIF)))
			if ((PINB & (1 << PINB0)))
				return 0;
		*b++ = SPDR;
	}
	return 1;
}

void
ao_spi_slave_send(void *buf, uint16_t len)
{
	uint8_t *b = buf;
	while (len--) {
		SPDR = *b++;
		while (!(SPSR & (1 << SPIF)))
			if ((PINB & (1 << PINB0)))
				return;
	}
	/* Clear pending SPIF bit by reading */
	(void) SPDR;
}

static uint8_t ao_spi_slave_running;

ISR(PCINT0_vect, ISR_BLOCK)
{
#if SPI_SLAVE_PIN_0_3
	if ((PINB & (1 << PORTB0)) == 0)
#endif
#if SPI_SLAVE_PIN_2_5
	if ((PINB & (1 << PORTB2)) == 0)
#endif
	{
		if (!ao_spi_slave_running) {
			ao_spi_slave_running = 1;
			ao_spi_slave();
		}
	} else {
		ao_spi_slave_running = 0;
	}
}

void
ao_spi_slave_init(void)
{
	/* We'd like to have a pull-up on SS so that disconnecting the
	 * TM would cause any SPI transaction to abort. However, when
	 * I tried that, SPI transactions would spontaneously abort,
	 * making me assume that we needed a less aggressive pull-up
	 * than is offered inside the AVR
	 */
#if SPI_SLAVE_PIN_0_3
	PCMSK0 |= (1 << PCINT0);	/* Enable PCINT0 pin change */
	PCICR |= (1 << PCIE0);		/* Enable pin change interrupt */

	DDRB = ((DDRB & 0xf0) |
		(1 << 3) |		/* MISO, output */
		(0 << 2) |		/* MOSI, input */
		(0 << 1) |		/* SCK, input */
		(0 << 0));		/* SS, input */

	PORTB = ((PORTB & 0xf0) |
		 (1 << 3) |		/* MISO, output */
		 (0 << 2) |		/* MOSI, no pull-up */
		 (0 << 1) |		/* SCK, no pull-up */
		 (1 << 0));		/* SS, pull-up */
#endif
#if SPI_SLAVE_PIN_2_5
	PCMSK0 |= (1 << PCINT2);	/* Enable PCINT2 pin change */
	PCICR |= (1 << PCIE0);		/* Enable pin change interrupt */

	DDRB = ((DDRB & 0xf0) |
		(0 << 5) |		/* SCK, input */
		(1 << 4) |		/* MISO, output */
		(0 << 3) |		/* MOSI, input */
		(0 << 2));		/* SS, input */

	PORTB = ((PORTB & 0xf0) |
		 (0 << 5) |		/* SCK, no pull-up */
		 (1 << 4) |		/* MISO, output */
		 (0 << 3) |		/* MOSI, no pull-up */
		 (1 << 2));		/* SS, pull-up */
#endif	

	SPCR = (0 << SPIE) |		/* Disable SPI interrupts */
		(1 << SPE) |		/* Enable SPI */
		(0 << DORD) |		/* MSB first */
		(0 << MSTR) |		/* Slave mode */
		(0 << CPOL) |		/* Clock low when idle */
		(0 << CPHA);		/* Sample at leading clock edge */
}
