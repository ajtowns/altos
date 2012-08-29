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
#include <ao_storage.h>

/* Total bytes of available storage */
ao_pos_t	ao_storage_total = 4096;

/* Block size - device is erased in these units. */
ao_pos_t	ao_storage_block = 1024;

/* Byte offset of config block. Will be ao_storage_block bytes long */
ao_pos_t	ao_storage_config = 0;

/* Storage unit size - device reads and writes must be within blocks of this size. */
uint16_t	ao_storage_unit = 1024;

/* Location of eeprom in address space */
#define stm_eeprom	((uint8_t *) 0x08080000)

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

static void
ao_intflash_unlock(void)
{
	/* Unlock Data EEPROM and FLASH_PECR register */
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY1;
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY2;

	/* Configure the FTDW bit (FLASH_PECR[8]) to execute
	 * word write, whatever the previous value of the word
	 * being written to
	 */
	stm_flash.pecr = ((0 << STM_FLASH_PECR_OBL_LAUNCH) |
			  (0 << STM_FLASH_PECR_ERRIE) |
			  (0 << STM_FLASH_PECR_EOPIE) |
			  (0 << STM_FLASH_PECR_FPRG) |
			  (0 << STM_FLASH_PECR_ERASE) |
			  (0 << STM_FLASH_PECR_FTDW) |
			  (1 << STM_FLASH_PECR_DATA) |
			  (0 << STM_FLASH_PECR_PROG) |
			  (0 << STM_FLASH_PECR_OPTLOCK) |
			  (0 << STM_FLASH_PECR_PRGLOCK) |
			  (0 << STM_FLASH_PECR_PELOCK));
}

static void
ao_intflash_lock(void)
{
	stm_flash.pecr |= (1 << STM_FLASH_PECR_PELOCK);
}

static void
ao_intflash_wait(void)
{
	/* Wait for the flash unit to go idle */
	while (stm_flash.sr & (1 << STM_FLASH_SR_BSY))
		;
}

static void
ao_intflash_write32(uint16_t pos, uint32_t w)
{
	volatile uint32_t	*addr;

	addr = (uint32_t *) (stm_eeprom + pos);

	/* Erase previous word */
	*addr = 0;
	ao_intflash_wait();

	if (w) {
		/* Write a word to a valid address in the data EEPROM */
		*addr = w;
		ao_intflash_wait();
	}
}

static void
ao_intflash_write8(uint16_t pos, uint8_t d)
{
	uint32_t	w, *addr, mask;
	uint8_t		shift;
	
	addr = (uint32_t *) (stm_eeprom + (pos & ~3));

	/* Compute word to be written */
	shift = (pos & 3) << 3;
	mask = 0xff << shift;
	w = (*addr & ~mask) | (d << shift);

	ao_intflash_write32(pos & ~3, w);
}

static uint8_t
ao_intflash_read(uint16_t pos)
{
	return stm_eeprom[pos];
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

	ao_intflash_unlock();
	while (len) {
		if ((pos & 3) == 0 && len >= 4) {
			uint32_t	w;

			w = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
			ao_intflash_write32(pos, w);
			pos += 4;
			d += 4;
			len -= 4;
		} else {
			ao_intflash_write8(pos, *d);
			pos += 1;
			d += 1;
			len -= 1;
		}
	}
	ao_intflash_lock();

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
	uint8_t	i;
	printf ("Using internal flash\n");
}

void
ao_storage_device_init(void)
{
}
