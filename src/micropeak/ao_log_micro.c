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
#include <ao_micropeak.h>
#include <ao_log_micro.h>
#include <ao_async.h>

static uint16_t ao_log_offset = STARTING_LOG_OFFSET;

void
ao_log_micro_save(void)
{
	uint16_t	n_samples = (ao_log_offset - STARTING_LOG_OFFSET) / sizeof (uint16_t);
	ao_eeprom_write(PA_GROUND_OFFSET, &pa_ground, sizeof (pa_ground));
	ao_eeprom_write(PA_MIN_OFFSET, &pa_min, sizeof (pa_min));
	ao_eeprom_write(N_SAMPLES_OFFSET, &n_samples, sizeof (n_samples));
}

void
ao_log_micro_restore(void)
{
	ao_eeprom_read(PA_GROUND_OFFSET, &pa_ground, sizeof (pa_ground));
	ao_eeprom_read(PA_MIN_OFFSET, &pa_min, sizeof (pa_min));
}

void
ao_log_micro_data(void)
{
	uint16_t	low_bits = pa;

	if (ao_log_offset < MAX_LOG_OFFSET) {
		ao_eeprom_write(ao_log_offset, &low_bits, sizeof (low_bits));
		ao_log_offset += sizeof (low_bits);
	}
}

#define POLY 0x8408

static uint16_t
ao_log_micro_crc(uint16_t crc, uint8_t byte)
{
	uint8_t	i;

	for (i = 0; i < 8; i++) {
		if ((crc & 0x0001) ^ (byte & 0x0001))
			crc = (crc >> 1) ^ POLY;
		else
			crc = crc >> 1;
		byte >>= 1;
	}
	return crc;
}

static void
ao_log_hex_nibble(uint8_t b)
{
	if (b < 10)
		ao_async_byte('0' + b);
	else
		ao_async_byte('a' - 10 + b);
}

static void
ao_log_hex(uint8_t b)
{
	ao_log_hex_nibble(b>>4);
	ao_log_hex_nibble(b&0xf);
}

static void
ao_log_newline(void)
{
	ao_async_byte('\r');
	ao_async_byte('\n');
}

void
ao_log_micro_dump(void)
{
	uint16_t	n_samples;
	uint16_t	nbytes;
	uint8_t		byte;
	uint16_t	b;
	uint16_t	crc = 0xffff;

	ao_eeprom_read(N_SAMPLES_OFFSET, &n_samples, sizeof (n_samples));
	if (n_samples == 0xffff)
		n_samples = 0;
	nbytes = STARTING_LOG_OFFSET + sizeof (uint16_t) * n_samples;
	ao_async_start();
	ao_async_byte('M');
	ao_async_byte('P');
	for (b = 0; b < nbytes; b++) {
		if ((b & 0xf) == 0)
			ao_log_newline();
		ao_eeprom_read(b, &byte, 1);
		ao_log_hex(byte);
		crc = ao_log_micro_crc(crc, byte);
	}
	ao_log_newline();
	crc = ~crc;
	ao_log_hex(crc >> 8);
	ao_log_hex(crc);
	ao_log_newline();
	ao_async_stop();
}
