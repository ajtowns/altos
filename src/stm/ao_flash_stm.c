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

#include <ao.h>
#include <ao_flash_stm.h>

static uint8_t
ao_flash_pecr_is_locked(void)
{
	return (stm_flash.pecr & (1 << STM_FLASH_PECR_PELOCK)) != 0;
}

static uint8_t
ao_flash_pgr_is_locked(void)
{
	return (stm_flash.pecr & (1 << STM_FLASH_PECR_PRGLOCK)) != 0;
}

static void
ao_flash_pecr_unlock(void)
{
	if (!ao_flash_pecr_is_locked())
		return;

	/* Unlock Data EEPROM and FLASH_PECR register */
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY1;
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY2;
}

static void
ao_flash_pgr_unlock(void)
{
	if (!ao_flash_pgr_is_locked())
		return;

	/* Unlock program memory */
	stm_flash.prgkeyr = STM_FLASH_PRGKEYR_PRGKEY1;
	stm_flash.prgkeyr = STM_FLASH_PRGKEYR_PRGKEY2;
}

static void
ao_flash_wait_bsy(void)
{
	while (stm_flash.sr & (1 << STM_FLASH_SR_BSY))
		;
}

static void
ao_flash_erase_page(uint32_t *page)
{
	ao_flash_pecr_unlock();
	ao_flash_pgr_unlock();

	stm_flash.pecr |= (1 << STM_FLASH_PECR_ERASE);
	stm_flash.pecr |= (1 << STM_FLASH_PECR_PROG);
	
	ao_flash_wait_bsy();

	*page = 0x00000000;
}

static void __attribute__ ((section(".text.ram"), noinline))
	_ao_flash_half_page(uint32_t *dst, uint32_t *src)
{
	uint8_t		i;

	stm_flash.pecr |= (1 << STM_FLASH_PECR_FPRG);
	stm_flash.pecr |= (1 << STM_FLASH_PECR_PROG);
	while (stm_flash.sr & (1 << STM_FLASH_SR_BSY))
		;
	
	for (i = 0; i < 32; i++)
		*dst++ = *src++;
}

void
ao_flash_page(uint32_t *page, uint32_t *src)
{
	uint8_t		h;

	ao_flash_erase_page(page);
	for (h = 0; h < 2; h++) {
		ao_flash_pecr_unlock();
		ao_flash_pgr_unlock();
		_ao_flash_half_page(page, src);
		page += 32;
		src += 32;
	}
}
