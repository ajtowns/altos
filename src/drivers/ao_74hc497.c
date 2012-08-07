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

/*
 * 74HC597 driver.
 * Reads a single byte from the shift register
 */

#include <ao.h>
#include <ao_74hc497.h>

uint8_t
ao_74hc497_read(void)
{
	static __xdata state;
	ao_spi_get_bit(AO_74HC497_CS_PORT, AO_74HC497_CS_PIN, AO_74HC497_CS, AO_74HC497_SPI_BUS, AO_SPI_SPEED_FAST);
	ao_spi_send(&state, 1, AO_74HC497_SPI_BUS);
	ao_spi_put_bit(AO_74HC497_CS_PORT, AO_74HC497_CS_PIN, AO_74HC497_CS, AO_74HC497_SPI_BUS);
	return state;
}

void
ao_74hc497_init(void)
{
	ao_enable_output(AO_74HC497_CS_PORT, AO_74HC497_CS_PIN, AO_74HC497_CS, 1);
}
