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

#include <ao.h>

/*
 * ATtiny USI as a SPI interface
 */

/*
 * Transfer one byte in/out of the USI interface
 */

/* Three wire mode */
#define SPI_USICR_WM	((0 << USIWM1) | (1 << USIWM0)) 
#define SPI_USICR_CS	((1 << USICS1) | (0 << USICS0) | (1 << USICLK))

#define SPI_USICR_FAST_2 ((1 << USIWM0) | (1 << USITC))
#define SPI_USICR_FAST_1 ((1 << USIWM0) | (1 << USITC) | (1 << USICLK))


static uint8_t
ao_spi_transfer(uint8_t i)
{
	/* Load data register */
	USIDR = i;

#if 1
	USISR = (1 << USIOIF);
	do {
		USICR = SPI_USICR_WM | SPI_USICR_CS | (1 << USITC);
	} while ((USISR & (1 << USIOIF)) == 0);
#else
	/* 8 clocks */
	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;

	USICR = SPI_USICR_FAST_1;
	USICR = SPI_USICR_FAST_2;
#endif

	/* Pull data from the port */
	return USIDR;
}

/* Send bytes over SPI.
 *
 * This just polls; the SPI is set to go as fast as possible,
 * so using interrupts would take way too long
 */
void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	while (len--)
		ao_spi_transfer (*d++);
}

/* Receive bytes over SPI.
 *
 * Poll, sending zeros and reading data back
 */
void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant
{
	uint8_t	*d = block;

	while (len--)
		*d++ = ao_spi_transfer (0xff);
}

/*
 * Initialize USI
 *
 * Chip select is the responsibility of the caller
 */

void
ao_spi_init(void)
{
#if 1
	USICR = (1 << USIWM0) | (1 << USICS1) | (1 << USICLK);
#else
	USICR = SPI_USICR_FAST_2;
#endif
	SPI_DIR &= ~(1 << DDB0);	/* DI */
	SPI_DIR |= (1 << DDB1);		/* DO */
	SPI_DIR |= (1 << DDB2);		/* SCLK */
}
