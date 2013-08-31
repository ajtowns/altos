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
#include <ao_flash.h>

#define IAP_LOCATION 0x1fff1ff1

typedef void (*iap_func)(uint32_t *in, uint32_t *out);

static void
iap(uint32_t *in, uint32_t *out)
{
	ao_arch_block_interrupts();
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_FLASHREG);
	((iap_func) IAP_LOCATION)(in, out);
	ao_arch_release_interrupts();
}

#define LPC_IAP_PREPARE_WRITE		50
#define LPC_IAP_COPY_RAM_TO_FLASH	51
#define LPC_IAP_ERASE_SECTOR		52
#define LPC_IAP_BLANK_CHECK		53
#define LPC_IAP_READ_PART_ID		54
#define LPC_IAP_READ_BOOT_CODE_VERSION	55
#define LPC_IAP_COMPARE			56
#define LPC_IAP_REINVOKE_ISP		57
#define LPC_IAP_READ_UID		58
#define LPC_IAP_ERASE_PAGE		59
#define LPC_IAP_EEPROM_WRITE		61
#define LPC_IAP_EEPROM_READ		62

#define LPC_IAP_CMD_SUCCESS		0
#define LPC_IAP_INVALID_COMMAND		1
#define LPC_IAP_SRC_ADDR_ERROR		2
#define LPC_IAP_DST_ADDR_ERROR		3
#define LPC_IAP_SRC_ADDR_NOT_MAPPED	4
#define LPC_IAP_DST_ADDR_NOT_MAPPED	5
#define LPC_IAP_COUNT_ERROR		6
#define LPC_IAP_INVALID_SECTOR		7
#define LPC_IAP_SECTOR_NOT_BLANK	8
#define LPC_IAP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION	9
#define LPC_IAP_COMPARE_ERROR		10
#define LPC_IAP_BUSY			11
#define LPC_IAP_PARAM_ERROR		12
#define LPC_IAP_ADDR_ERROR		13
#define LPC_IAP_ADDR_NOT_MAPPED		14
#define LPC_IAP_CMD_LOCKED		15
#define LPC_IAP_INVALID_CODE		16
#define LPC_IAP_INVALID_BAUD_RATE	17
#define LPC_IAP_INVALID_STOP_BIT	18
#define LPC_IAP_CODE_READ_PROTECTION_ENABLED	19

#define LPC_FLASH_BASE			((uint8_t *) 0x0)
#define LPC_FLASH_SECTOR		4096
#define LPC_FLASH_SECTOR_MASK	        (LPC_FLASH_SECTOR - 1)
#define LPC_FLASH_SECTOR_SHIFT		12

static uint32_t	iap_in[5], iap_out[5];

static uint32_t
ao_lpc_addr_to_sector(uint8_t *addr)
{
	uint32_t	off = addr - LPC_FLASH_BASE;

	return off >> LPC_FLASH_SECTOR_SHIFT;
}

static uint8_t
ao_lpc_addr_is_sector_aligned(uint8_t *addr)
{
	uint32_t	off = addr - LPC_FLASH_BASE;
	return		(off & LPC_FLASH_SECTOR_MASK) == 0;
}

static uint32_t
ao_lpc_prepare_write(uint32_t start_sector, uint32_t end_sector)
{
	iap_in[0] = LPC_IAP_PREPARE_WRITE;
	iap_in[1] = start_sector;
	iap_in[2] = end_sector;
	iap(iap_in,iap_out);
	return iap_out[0];
}

static uint32_t
ao_lpc_copy_ram_to_flash(uint8_t *dst, uint8_t *src, uint32_t len, uint32_t freq)
{
	iap_in[0] = LPC_IAP_COPY_RAM_TO_FLASH;
	iap_in[1] = (uint32_t) dst;
	iap_in[2] = (uint32_t) src;
	iap_in[3] = len;
	iap_in[4] = freq;
	iap(iap_in,iap_out);
	return iap_out[0];
}

static uint32_t
ao_lpc_erase_sector(uint32_t start_sector, uint32_t end_sector, uint32_t freq)
{
	iap_in[0] = LPC_IAP_ERASE_SECTOR;
	iap_in[1] = start_sector;
	iap_in[2] = end_sector;
	iap_in[3] = freq;
	iap(iap_in,iap_out);
	return iap_out[0];
}

uint32_t
ao_lpc_read_part_id(void)
{
	iap_in[0] = LPC_IAP_READ_PART_ID;
	iap(iap_in,iap_out);
	return iap_out[1];
}

uint32_t
ao_flash_erase_page(uint8_t *page)
{
	uint32_t	ret = LPC_IAP_CMD_SUCCESS;
	if (ao_lpc_addr_is_sector_aligned(page)) {
		uint32_t	sector = ao_lpc_addr_to_sector(page);
		ret = ao_lpc_prepare_write(sector, sector);
		if (ret == LPC_IAP_CMD_SUCCESS)
			ret = ao_lpc_erase_sector(sector, sector, AO_LPC_SYSCLK / 1000);
	}
	return ret;
}

uint32_t
ao_flash_page(uint8_t *page, uint8_t *src)
{
	uint32_t	sector = ao_lpc_addr_to_sector(page);
	uint32_t	ret;

	ret = ao_flash_erase_page(page);
	if (ret != LPC_IAP_CMD_SUCCESS)
		return ret;
	ret = ao_lpc_prepare_write(sector, sector);
	if (ret != LPC_IAP_CMD_SUCCESS)
		return ret;
	ret = ao_lpc_copy_ram_to_flash(page, src, 256, AO_LPC_SYSCLK / 1000);
	return ret;
}
