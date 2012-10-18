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

static void
ao_eeprom_wait(void)
{
	/* Wait for previous write to complete */
	while (EECR & (1 << EEPE))
		;
}

static uint8_t
ao_eeprom_read_byte(uint16_t addr)
{
	uint8_t	v;

	ao_eeprom_wait();
	EEAR = addr;
	cli();
	EECR |= (1 << EERE);
	v = EEDR;
	sei();
	return v;
}

static void
ao_eeprom_write_byte(uint16_t addr, uint8_t v)
{
	ao_eeprom_wait();
	EECR = (0 << EEPM1) | (0 << EEPM0);
	EEAR = addr;
	EEDR = v;
	cli();
	EECR |= (1 << EEMPE);
	EECR |= (1 << EEPE);
	sei();
}

void
ao_eeprom_read(uint16_t addr, void *buf, uint16_t len)
{
	uint8_t	*b = buf;

	while (len--)
		*b++ = ao_eeprom_read_byte(addr++);
}

void
ao_eeprom_write(uint16_t addr, void *buf, uint16_t len)
{
	uint8_t	*b = buf;

	while (len--)
		ao_eeprom_write_byte(addr++, *b++);
}
