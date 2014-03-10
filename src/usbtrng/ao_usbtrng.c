/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#define AO_TRNG_SPI_BUS		1
#define AO_TRNG_SPI_SPEED	AO_SPI_SPEED_250kHz

static void
ao_trng_test(void)
{
	static uint8_t	random[32];
	uint8_t		i;

	ao_spi_get(AO_TRNG_SPI_BUS, AO_TRNG_SPI_SPEED);
	ao_spi_recv(random, sizeof (random), AO_TRNG_SPI_BUS);
	ao_spi_put(AO_TRNG_SPI_BUS);
	for (i = 0; i < sizeof (random); i++)
		printf (" %02x", random[i]);
	printf ("\n");
}

static const struct ao_cmds ao_trng_cmds[] = {
	{ ao_trng_test, "R\0Dump some random numbers" },
	{ 0, NULL }
};

void
main(void)
{
	ao_clock_init();
	ao_task_init();
	ao_timer_init();

	ao_spi_init();
	ao_usb_init();

	ao_serial_init();

	ao_led_init(LEDS_AVAILABLE);

	ao_led_on(AO_LED_GREEN);

	ao_cmd_init();

	ao_cmd_register(ao_trng_cmds);

	ao_start_scheduler();
}
