/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
 * 74HC165 driver.
 * Reads a single byte from the shift register
 */

#include <ao.h>
#include <ao_74hc165.h>

uint8_t
ao_74hc165_read(void)
{
	static __xdata state;
	ao_spi_get(AO_74HC165_SPI_BUS);
	ao_spi_set_speed(AO_74HC165_SPI_BUS, AO_SPI_SPEED_FAST);
	AO_74HC165_CS = 1;
	ao_spi_recv(&state, 1, AO_74HC165_SPI_BUS);
	AO_74HC165_CS = 0;
	ao_spi_put(AO_74HC165_SPI_BUS);
	return state;
}

static void
ao_74hc165_cmd(void)
{
	uint8_t	v;

	v = ao_74hc165_read();
	printf ("Switches: 0x%02x\n", v);
}

static const struct ao_cmds ao_74hc165_cmds[] = {
	{ ao_74hc165_cmd, "L\0Show 74hc165" },
	{ 0, NULL }
};

void
ao_74hc165_init(void)
{
	ao_enable_output(AO_74HC165_CS_PORT, AO_74HC165_CS_PIN, AO_74HC165_CS, 0);
	ao_cmd_register(&ao_74hc165_cmds[0]);
}
