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
#include <ao_ms5607.h>

#if HAS_MS5607 || HAS_MS5611

__xdata struct ao_ms5607_prom	ao_ms5607_prom;
static __xdata uint8_t	  	ms5607_configured;

static void
ao_ms5607_start(void) {
	ao_spi_get_bit(AO_MS5607_CS_PORT, AO_MS5607_CS_PIN, AO_MS5607_CS, AO_MS5607_SPI_INDEX, AO_SPI_SPEED_FAST);
}

static void
ao_ms5607_stop(void) {
	ao_spi_put_bit(AO_MS5607_CS_PORT, AO_MS5607_CS_PIN, AO_MS5607_CS, AO_MS5607_SPI_INDEX);
}

static void
ao_ms5607_reset(void) {
	uint8_t	cmd;

	cmd = AO_MS5607_RESET;
	ao_ms5607_start();
	ao_spi_send(DATA_TO_XDATA(&cmd), 1, AO_MS5607_SPI_INDEX);
	ao_delay(AO_MS_TO_TICKS(10));
	ao_ms5607_stop();
}

static uint8_t
ao_ms5607_crc(uint8_t *prom)
{
	uint8_t		crc_byte = prom[15];
	uint8_t 	cnt;
	uint16_t	n_rem = 0;
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
ao_ms5607_prom_read(__xdata struct ao_ms5607_prom *prom)
{
	uint8_t			addr;
	uint8_t			crc;
	__xdata uint16_t	*r;

	r = (__xdata uint16_t *) prom;
	for (addr = 0; addr < 8; addr++) {
		uint8_t	cmd = AO_MS5607_PROM_READ(addr);
		ao_ms5607_start();
		ao_spi_send(DATA_TO_XDATA(&cmd), 1, AO_MS5607_SPI_INDEX);
		ao_spi_recv(r, 2, AO_MS5607_SPI_INDEX);
		ao_ms5607_stop();
		r++;
	}
	crc = ao_ms5607_crc((uint8_t *) prom);
	if (crc != (((uint8_t *) prom)[15] & 0xf)) {
#if HAS_TASK
		printf ("MS5607 PROM CRC error (computed %x actual %x)\n",
			crc, (((uint8_t *) prom)[15] & 0xf));
		flush();
#endif
		ao_panic(AO_PANIC_SELF_TEST_MS5607);
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

void
ao_ms5607_setup(void)
{
	if (ms5607_configured)
		return;
	ms5607_configured = 1;
	ao_ms5607_reset();
	ao_ms5607_prom_read(&ao_ms5607_prom);
}

static __xdata volatile uint8_t	ao_ms5607_done;

static void
ao_ms5607_isr(void)
{
	ao_exti_disable(AO_MS5607_MISO_PORT, AO_MS5607_MISO_PIN);
	ao_ms5607_done = 1;
	ao_wakeup((__xdata void *) &ao_ms5607_done);
}

static uint32_t
ao_ms5607_get_sample(uint8_t cmd) {
	__xdata uint8_t	reply[3];
	__xdata uint8_t read;

	ao_ms5607_done = 0;

	ao_ms5607_start();
	ao_spi_send(DATA_TO_XDATA(&cmd), 1, AO_MS5607_SPI_INDEX);

	ao_exti_enable(AO_MS5607_MISO_PORT, AO_MS5607_MISO_PIN);

#if AO_MS5607_PRIVATE_PINS
	ao_spi_put(AO_MS5607_SPI_INDEX);
#endif
	ao_arch_block_interrupts();
	while (!ao_gpio_get(AO_MS5607_MISO_PORT, AO_MS5607_MISO_PIN, AO_MS5607_MISO) &&
	       !ao_ms5607_done)
		ao_sleep((void *) &ao_ms5607_done);
	ao_arch_release_interrupts();
#if AO_MS5607_PRIVATE_PINS
	stm_gpio_set(AO_MS5607_CS_PORT, AO_MS5607_CS_PIN, 1);
#else
	ao_ms5607_stop();
#endif

	ao_ms5607_start();
	read = AO_MS5607_ADC_READ;
	ao_spi_send(&read, 1, AO_MS5607_SPI_INDEX);
	ao_spi_recv(&reply, 3, AO_MS5607_SPI_INDEX);
	ao_ms5607_stop();

	return ((uint32_t) reply[0] << 16) | ((uint32_t) reply[1] << 8) | (uint32_t) reply[2];
}

#ifndef AO_MS5607_BARO_OVERSAMPLE
#define AO_MS5607_BARO_OVERSAMPLE	2048
#endif

#ifndef AO_MS5607_TEMP_OVERSAMPLE
#define AO_MS5607_TEMP_OVERSAMPLE	AO_MS5607_BARO_OVERSAMPLE
#endif

#define token_paster(x,y)	x ## y
#define token_evaluator(x,y)	token_paster(x,y)

#define AO_CONVERT_D1	token_evaluator(AO_MS5607_CONVERT_D1_, AO_MS5607_BARO_OVERSAMPLE)
#define AO_CONVERT_D2	token_evaluator(AO_MS5607_CONVERT_D2_, AO_MS5607_TEMP_OVERSAMPLE)

void
ao_ms5607_sample(__xdata struct ao_ms5607_sample *sample)
{
	sample->pres = ao_ms5607_get_sample(AO_CONVERT_D1);
	sample->temp = ao_ms5607_get_sample(AO_CONVERT_D2);
}

#ifdef _CC1111_H_
#include "ao_ms5607_convert_8051.c"
#else
#include "ao_ms5607_convert.c"
#endif

#if HAS_TASK
__xdata struct ao_ms5607_sample	ao_ms5607_current;

static void
ao_ms5607(void)
{
	ao_ms5607_setup();
	for (;;)
	{
		ao_ms5607_sample(&ao_ms5607_current);
		ao_arch_block_interrupts();
		AO_DATA_PRESENT(AO_DATA_MS5607);
		AO_DATA_WAIT();
		ao_arch_release_interrupts();
	}
}

__xdata struct ao_task ao_ms5607_task;

void
ao_ms5607_info(void)
{
	printf ("ms5607 reserved: %u\n", ao_ms5607_prom.reserved);
	printf ("ms5607 sens: %u\n", ao_ms5607_prom.sens);
	printf ("ms5607 off: %u\n", ao_ms5607_prom.off);
	printf ("ms5607 tcs: %u\n", ao_ms5607_prom.tcs);
	printf ("ms5607 tco: %u\n", ao_ms5607_prom.tco);
	printf ("ms5607 tref: %u\n", ao_ms5607_prom.tref);
	printf ("ms5607 tempsens: %u\n", ao_ms5607_prom.tempsens);
	printf ("ms5607 crc: %u\n", ao_ms5607_prom.crc);
}

static void
ao_ms5607_dump(void)
{
	__xdata struct ao_ms5607_value value;

	ao_ms5607_convert(&ao_ms5607_current, &value);
	printf ("Pressure:    %8lu %8ld\n", ao_ms5607_current.pres, value.pres);
	printf ("Temperature: %8lu %8ld\n", ao_ms5607_current.temp, value.temp);
	printf ("Altitude: %ld\n", ao_pa_to_altitude(value.pres));
}

__code struct ao_cmds ao_ms5607_cmds[] = {
	{ ao_ms5607_dump,	"B\0Display MS5607 data" },
	{ 0, NULL },
};
#endif /* HAS_TASK */

void
ao_ms5607_init(void)
{
	ms5607_configured = 0;
	ao_spi_init_cs(AO_MS5607_CS_PORT, (1 << AO_MS5607_CS_PIN));

#if HAS_TASK
	ao_cmd_register(&ao_ms5607_cmds[0]);
	ao_add_task(&ao_ms5607_task, ao_ms5607, "ms5607");
#endif

	/* Configure the MISO pin as an interrupt; when the
	 * conversion is complete, the MS5607 will raise this
	 * pin as a signal
	 */
	ao_exti_setup(AO_MS5607_MISO_PORT,
		      AO_MS5607_MISO_PIN,
		      AO_EXTI_MODE_RISING|
		      AO_EXTI_PIN_NOCONFIGURE,
		      ao_ms5607_isr);
}

#endif
