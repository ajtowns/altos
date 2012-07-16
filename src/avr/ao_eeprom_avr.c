/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 * Copyright © 2011  Anthony Towns <aj@erisian.com.au>
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
#include <ao_storage.h>

/* Total bytes of available storage */
__pdata ao_pos_t	ao_storage_total = 1024;

/* Block size - device is erased in these units. */
__pdata ao_pos_t	ao_storage_block = 1024;

/* Byte offset of config block. Will be ao_storage_block bytes long */
__pdata ao_pos_t	ao_storage_config = 0;

/* Storage unit size - device reads and writes must be within blocks of this size. */
__pdata uint16_t	ao_storage_unit = 1024;

/*
 * The internal flash chip is arranged in 8 byte sectors; the
 * chip cannot erase in units smaller than that.
 *
 * Writing happens in units of 2 bytes and
 * can only change bits from 1 to 0. So, you can rewrite
 * the same contents, or append to an existing page easily enough
 */

/*
 * Erase the specified sector
 */
uint8_t
ao_storage_erase(ao_pos_t pos) __reentrant
{
	/* Not necessary */
	return 1;
}

#define ao_intflash_wait_idle() do {					\
		/* Wait for any outstanding writes to complete */	\
		while (EECR & (1 << EEPE))				\
			;						\
	} while (0)							\

static void
ao_intflash_write(uint16_t pos, uint8_t d)
{
	ao_intflash_wait_idle();
	EEAR = pos;
	EEDR = d;
	ao_arch_critical(
		EECR |= (1 << EEMPE);
		EECR |= (1 << EEPE);
		);
}

static uint8_t
ao_intflash_read(uint16_t pos)
{
	ao_intflash_wait_idle();
	EEAR = pos;

	EECR |= (1 << EERE);
	return EEDR;
}
/*
 * Write to flash
 */

uint8_t
ao_storage_device_write(ao_pos_t pos32, __xdata void *v, uint16_t len) __reentrant
{
	uint16_t pos = pos32;
	__xdata uint8_t *d = v;

	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;

	while (len--)
		ao_intflash_write(pos++, *d++);

	return 1;
}

/*
 * Read from flash
 */
uint8_t
ao_storage_device_read(ao_pos_t pos, __xdata void *v, uint16_t len) __reentrant
{
	uint8_t	*d = v;
	
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	while (len--)
		*d++ = ao_intflash_read(pos++);
	return 1;
}

void
ao_storage_flush(void) __reentrant
{
}

void
ao_storage_setup(void)
{
}

void
ao_storage_device_info(void) __reentrant
{
	printf ("Using internal flash\n");
}

void
ao_storage_device_init(void)
{
}
