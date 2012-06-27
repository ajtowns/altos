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
#include <ao_exti.h>
#include "ao_ms5607.h"

static struct ao_ms5607_prom	ms5607_prom;
static uint8_t	  		ms5607_configured;

static void
ao_ms5607_start(void) {
	ao_spi_get(AO_MS5607_SPI_INDEX);
	stm_gpio_set(AO_MS5607_CS_GPIO, AO_MS5607_CS, 0);
}

static void
ao_ms5607_stop(void) {
	stm_gpio_set(AO_MS5607_CS_GPIO, AO_MS5607_CS, 1);
	ao_spi_put(AO_MS5607_SPI_INDEX);
}

static void
ao_ms5607_reset(void) {
	uint8_t	cmd;

	cmd = AO_MS5607_RESET;
	ao_ms5607_start();
	ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
	ao_delay(AO_MS_TO_TICKS(10));
	ao_ms5607_stop();
}

static uint8_t
ao_ms5607_crc(uint8_t *prom)
{
	uint8_t		crc_byte = prom[15];
	uint8_t 	cnt;
	uint16_t	n_rem = 0;
	uint16_t	crc_read;
	uint8_t		n_bit;

	prom[15] = 0;
	for (cnt = 0; cnt < 16; cnt++) {
		n_rem ^= prom[cnt];
		for (n_bit = 8; n_bit > 0; n_bit--) {
			if (n_rem & 0x8000)
				n_rem = (n_rem << 1) ^ 0x3000;
			else
				n_rem = (n_rem << 1);
		}
	}
	n_rem = (n_rem >> 12) & 0xf;
	prom[15] = crc_byte;
	return n_rem;
}

static void
ao_ms5607_prom_read(struct ao_ms5607_prom *prom)
{
	uint8_t		addr;
	uint8_t		crc;
	uint16_t	*r;

	r = (uint16_t *) prom;
	for (addr = 0; addr < 8; addr++) {
		uint8_t	cmd = AO_MS5607_PROM_READ(addr);
		ao_ms5607_start();
		ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
		ao_spi_recv(r, 2, AO_MS5607_SPI_INDEX);
		ao_ms5607_stop();
		r++;
	}
	crc = ao_ms5607_crc((uint8_t *) prom);
	if (crc != (((uint8_t *) prom)[15] & 0xf)) {
		printf ("MS5607 PROM CRC error (computed %x actual %x)\n",
			crc, (((uint8_t *) prom)[15] & 0xf));
		flush();
		ao_panic(AO_PANIC_SELF_TEST);
	}

#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* Byte swap */
	r = (uint16_t *) prom;
	for (addr = 0; addr < 8; addr++) {
		uint16_t	t = *r;
		*r++ = (t << 8) | (t >> 8);
	}
#endif
}

static void
ao_ms5607_setup(void)
{
	if (ms5607_configured)
		return;
	ms5607_configured = 1;
	ao_ms5607_reset();
	ao_ms5607_prom_read(&ms5607_prom);
}

static uint8_t	ao_ms5607_done;

static void
ao_ms5607_isr(void)
{
	ao_ms5607_done = 1;
	ao_wakeup(&ao_ms5607_done);
}

static uint32_t
ao_ms5607_get_sample(uint8_t cmd) {
	uint8_t	reply[3];
	uint8_t read;
	uint16_t now;

	ao_ms5607_done = 0;

	ao_ms5607_start();
	ao_spi_send(&cmd, 1, AO_MS5607_SPI_INDEX);
	ao_exti_enable(AO_MS5607_MISO_GPIO, AO_MS5607_MISO);
	cli();
	while (!ao_ms5607_done)
		ao_sleep(&ao_ms5607_done);
	sei();
	ao_exti_disable(AO_MS5607_MISO_GPIO, AO_MS5607_MISO);
	ao_ms5607_stop();

	ao_ms5607_start();
	read = AO_MS5607_ADC_READ;
	ao_spi_send(&read, 1, AO_MS5607_SPI_INDEX);
	ao_spi_recv(&reply, 3, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();

	return ((uint32_t) reply[0] << 16) | ((uint32_t) reply[1] << 8) | (uint32_t) reply[2];
}

void
ao_ms5607_sample(struct ao_ms5607_sample *sample)
{
	sample->pres = ao_ms5607_get_sample(AO_MS5607_CONVERT_D1_2048);
	sample->temp = ao_ms5607_get_sample(AO_MS5607_CONVERT_D2_2048);
}

void
ao_ms5607_convert(struct ao_ms5607_sample *sample, struct ao_ms5607_value *value)
{
	uint8_t	addr;
	int32_t	dT;
	int32_t TEMP;
	int64_t OFF;
	int64_t SENS;
	int32_t P;

	dT = sample->temp - ((int32_t) ms5607_prom.tref << 8);
	
	TEMP = 2000 + (((int64_t) dT * ms5607_prom.tempsens) >> 23);

	OFF = ((int64_t) ms5607_prom.off << 17) + (((int64_t) ms5607_prom.tco * dT) >> 6);

	SENS = ((int64_t) ms5607_prom.sens << 16) + (((int64_t) ms5607_prom.tcs * dT) >> 7);

	if (TEMP < 2000) {
		int32_t	T2 = ((int64_t) dT * (int64_t) dT) >> 31;
		int32_t TEMPM = TEMP - 2000;
		int64_t OFF2 = (61 * (int64_t) TEMPM * (int64_t) TEMPM) >> 4;
		int64_t SENS2 = 2 * (int64_t) TEMPM * (int64_t) TEMPM;
		if (TEMP < 1500) {
			int32_t TEMPP = TEMP + 1500;
			int64_t TEMPP2 = TEMPP * TEMPP;
			OFF2 = OFF2 + 15 * TEMPP2;
			SENS2 = SENS2 + 8 * TEMPP2;
		}
		TEMP -= T2;
		OFF -= OFF2;
		SENS -= SENS2;
	}

	value->pres = ((((int64_t) sample->pres * SENS) >> 21) - OFF) >> 15;
	value->temp = TEMP;
}

struct ao_ms5607_sample	ao_ms5607_current;
uint8_t ao_ms5607_valid;

static void
ao_ms5607(void)
{
	ao_ms5607_setup();
	for (;;)
	{
		static struct ao_ms5607_sample	ao_ms5607_next;
		ao_ms5607_sample(&ao_ms5607_next);
		ao_arch_critical(
			ao_ms5607_current = ao_ms5607_next;
			ao_ms5607_valid = 1;
			);
		ao_delay(0);
	}
}

__xdata struct ao_task ao_ms5607_task;

void
ao_ms5607_info(void)
{
	printf ("ms5607 reserved: %u\n", ms5607_prom.reserved);
	printf ("ms5607 sens: %u\n", ms5607_prom.sens);
	printf ("ms5607 off: %u\n", ms5607_prom.off);
	printf ("ms5607 tcs: %u\n", ms5607_prom.tcs);
	printf ("ms5607 tco: %u\n", ms5607_prom.tco);
	printf ("ms5607 tref: %u\n", ms5607_prom.tref);
	printf ("ms5607 tempsens: %u\n", ms5607_prom.tempsens);
	printf ("ms5607 crc: %u\n", ms5607_prom.crc);
}

static void
ao_ms5607_dump(void)
{
	struct ao_ms5607_sample	sample;
	struct ao_ms5607_value value;

	sample = ao_ms5607_current;
	ao_ms5607_convert(&sample, &value);
	printf ("Pressure:    %8u %8d\n", sample.pres, value.pres);
	printf ("Temperature: %8u %8d\n", sample.temp, value.temp);
	printf ("Altitude: %ld\n", ao_pa_to_altitude(value.pres));
}

__code struct ao_cmds ao_ms5607_cmds[] = {
	{ ao_ms5607_dump,	"B\0Display MS5607 data" },
	{ 0, NULL },
};

void
ao_ms5607_init(void)
{
	ms5607_configured = 0;
	ao_ms5607_valid = 0;
	ao_cmd_register(&ao_ms5607_cmds[0]);
	ao_spi_init_cs(AO_MS5607_CS_GPIO, (1 << AO_MS5607_CS));

	ao_add_task(&ao_ms5607_task, ao_ms5607, "ms5607");

	/* Configure the MISO pin as an interrupt; when the
	 * conversion is complete, the MS5607 will raise this
	 * pin as a signal
	 */
	ao_exti_setup(AO_MS5607_MISO_GPIO,
		      AO_MS5607_MISO,
		      AO_EXTI_MODE_RISING,
		      ao_ms5607_isr);

	/* Reset the pin from INPUT to ALTERNATE so that SPI works
	 * This needs an abstraction at some point...
	 */
	stm_moder_set(AO_MS5607_MISO_GPIO,
		      AO_MS5607_MISO,
		      STM_MODER_ALTERNATE);
}
