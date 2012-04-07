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
#include "ao_ms5607.h"

#define AO_MS5607_CS_GPIO	stm_gpioc
#define AO_MS5607_CS		4
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS)
#define AO_MS5607_SPI_INDEX	(STM_SPI_INDEX(1))

struct ms5607_prom {
	uint16_t	reserved;
	uint16_t	sens;
	uint16_t	off;
	uint16_t	tcs;
	uint16_t	tco;
	uint16_t	tref;
	uint16_t	tempsens;
	uint16_t	crc;
};

static struct ms5607_prom ms5607_prom;

static void
ao_ms5607_start(void) {
	ao_spi_get(AO_MS5607_SPI_INDEX);
	stm_gpio_set(&AO_MS5607_CS_GPIO, AO_MS5607_CS, 0);
}

static void
ao_ms5607_stop(void) {
	stm_gpio_set(&AO_MS5607_CS_GPIO, AO_MS5607_CS, 1);
	ao_spi_put(AO_MS5607_SPI_INDEX);
}

static void
ao_ms5607_reset(void) {
	uint8_t	cmd;

	cmd = AO_MS5607_RESET;
	ao_ms5607_start();
	ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();
}

static uint16_t
ao_ms5607_prom_read(uint8_t addr)
{
	uint8_t	cmd = AO_MS5607_PROM_READ(addr);
	uint8_t d[2];

	ao_ms5607_start();
	ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
	ao_spi_recv(d, 2, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();
	return ((uint16_t) d[0] << 8) | (uint16_t) d[1];
}

static void
ao_ms5607_init_chip(void) {
	uint8_t		addr;
	uint16_t	*prom;
	ao_ms5607_reset();
	prom = &ms5607_prom.reserved;

	for (addr = 0; addr <= 7; addr++)
		prom[addr] = ao_ms5607_prom_read(addr);
}

static uint32_t
ao_ms5607_convert(uint8_t cmd) {
	uint8_t	reply[3];
	uint8_t read;

	ao_ms5607_start();
	ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();

	ao_delay(AO_MS_TO_TICKS(200));

	ao_ms5607_start();
	read = AO_MS5607_ADC_READ;
	ao_spi_send(&read, 1, AO_MS5607_SPI_INDEX);
	ao_spi_recv(&reply, 3, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();

	return ((uint32_t) reply[0] << 16) | ((uint32_t) reply[1] << 8) | (uint32_t) reply[2];
}

static void
ao_ms5607_dump(void)
{
	uint8_t	addr;
	uint32_t d1, d2;
	int32_t	dT;
	int32_t TEMP;
	int64_t OFF;
	int64_t SENS;
	int32_t P;

	ao_ms5607_init_chip();
	printf ("reserved: %d\n", ms5607_prom.reserved);
	printf ("sens:     %d\n", ms5607_prom.sens);
	printf ("off:      %d\n", ms5607_prom.off);
	printf ("tcs:      %d\n", ms5607_prom.tcs);
	printf ("tco:      %d\n", ms5607_prom.tco);
	printf ("tref:     %d\n", ms5607_prom.tref);
	printf ("tempsens: %d\n", ms5607_prom.tempsens);
	printf ("crc:      %d\n", ms5607_prom.crc);
	d1 =  ao_ms5607_convert(AO_MS5607_CONVERT_D1_4096);
	printf ("Conversion D1: %d\n", d1);
	d2 =  ao_ms5607_convert(AO_MS5607_CONVERT_D2_4096);
	printf ("Conversion D2: %d\n", d2);

	dT = d2 - ((int32_t) ms5607_prom.tref << 8);
	
	TEMP = 2000 + (((int64_t) dT * ms5607_prom.tempsens) >> 23);

	OFF = ((int64_t) ms5607_prom.off << 17) + (((int64_t) ms5607_prom.tco * dT) >> 6);

	SENS = ((int64_t) ms5607_prom.sens << 16) + (((int64_t) ms5607_prom.tcs * dT) >> 7);

	if (TEMP < 2000) {
		int32_t	T2 = ((int64_t) dT * (int64_t) dT) >> 31;
		int32_t TEMPM = TEMP - 2000;
		int64_t OFF2 = (61 * (int64_t) TEMPM * (int64_t) TEMPM) >> 4;
		int64_t SENS2 = 2 * (int64_t) TEMPM * (int64_t) TEMPM;
	}

	P = ((((int64_t) d1 * SENS) >> 21) - OFF) >> 15;
	
	printf ("Temperature: %d", TEMP);
	printf ("Pressure %d\n", P);
}

__code struct ao_cmds ao_ms5607_cmds[] = {
	{ ao_ms5607_dump,	"p\0Display MS5607 data" },
	{ 0, NULL },
};

void
ao_ms5607_init(void)
{
	ao_cmd_register(&ao_ms5607_cmds[0]);

	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOCEN);
	stm_gpio_set(&AO_MS5607_CS_GPIO, AO_MS5607_CS, 1);
	stm_moder_set(&AO_MS5607_CS_GPIO, AO_MS5607_CS, STM_MODER_OUTPUT);
}
