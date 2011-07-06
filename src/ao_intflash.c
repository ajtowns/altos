/*
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

#include "ao.h"
#include "cc1111.h"

#define ENDOFCODE  (CODESIZE)
#define AO_INTFLASH_BLOCK	1024
#define AO_INTFLASH_BLOCKS 	((0x8000 - ENDOFCODE)/AO_INTFLASH_BLOCK)
#define AO_INTFLASH_SIZE      	(AO_INTFLASH_BLOCK * AO_INTFLASH_BLOCKS)
#define AO_INTFLASH_LOCATION	(0x8000 - AO_INTFLASH_SIZE)

/*
 *       21000 * 24e6
 * FWT = ------------
 *           16e9
 *
 *     = 31.5
 *
 * Round up and use 32
 */

#define FLASH_TIMING	0x20

#if AO_INTFLASH_BLOCKS < 2
#error "Too few pages"
#endif

#if AO_INFTLASH_LOCATION % 1024 != 0
#error "Pages aren't aligned properly"
#endif

__xdata __at(AO_INTFLASH_LOCATION) uint8_t ao_intflash[AO_INTFLASH_SIZE];

/* Total bytes of available storage */
__pdata uint32_t	ao_storage_total = sizeof(ao_intflash);

/* Block size - device is erased in these units. */
__pdata uint32_t	ao_storage_block = AO_INTFLASH_BLOCK;

/* Byte offset of config block. Will be ao_storage_block bytes long */
__pdata uint32_t	ao_storage_config = sizeof(ao_intflash) - AO_INTFLASH_BLOCK;

/* Storage unit size - device reads and writes must be within blocks of this size. */
__pdata uint16_t	ao_storage_unit = AO_INTFLASH_BLOCK;

__xdata static uint8_t  ao_intflash_dma_done;
static uint8_t ao_intflash_dma;

/*
 * The internal flash chip is arranged in 1kB sectors; the
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
ao_storage_erase(uint32_t pos) __reentrant
{
	uint16_t addr;

	if (pos >= ao_storage_total || pos + ao_storage_block > ao_storage_total)
		return 0;

	addr = ((uint16_t)(ao_intflash + pos) >> 1);

	FADDRH = addr >> 8;
	FADDRL = addr;

	__critical {
		_asm
		.even
		orl _FCTL, #FCTL_ERASE;		; FCTL |=  FCTL_ERASE
		nop				; Required, see datasheet.
		_endasm;
	}

	return 1;
}

/*
 * Write to flash
 */

static void
ao_intflash_write_aligned(uint16_t pos, __xdata void *d, uint16_t len) __reentrant
{
	pos = ((uint16_t) ao_intflash + pos) >> 1;

	ao_dma_set_transfer(ao_intflash_dma,
			    d,
			    &FWDATAXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_FLASH,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_HIGH);

	FADDRH = pos >> 8;
	FADDRL = pos;

	ao_dma_start(ao_intflash_dma);

	__critical {
		_asm
		.even
		orl _FCTL, #FCTL_WRITE;		; FCTL |=  FCTL_WRITE
		nop
		_endasm;
	}
}

static void
ao_intflash_write_byte(uint16_t pos, uint8_t byte) __reentrant
{
	static __xdata uint8_t b[2];

	if (pos & 1) {
		b[0] = 0xff;
		b[1] = byte;
	} else {
		b[0] = byte;
		b[1] = 0xff;
	}
	ao_intflash_write_aligned(pos, b, 2);
}

uint8_t
ao_storage_device_write(uint32_t pos32, __xdata void *v, uint16_t len) __reentrant
{
	uint16_t pos = pos32;
	__xdata uint8_t *d = v;
	uint8_t oddlen;

	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	if (len == 0)
		return 1;

	if (pos & 1) {
		ao_intflash_write_byte(pos++, *d++);
		len--;
	}
	oddlen = len & 1;
	len -= oddlen;
	if (len)
		ao_intflash_write_aligned(pos, d, len);
	if (oddlen)
		ao_intflash_write_byte(pos + len, d[len]);

	return 1;
}

/*
 * Read from flash
 */
uint8_t
ao_storage_device_read(uint32_t pos, __xdata void *d, uint16_t len) __reentrant
{
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	memcpy(d, ao_intflash+pos, len);
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
	printf ("Using internal flash, starting at 0x%04x\n", AO_INTFLASH_LOCATION);
}

void
ao_storage_device_init(void)
{
	ao_intflash_dma = ao_dma_alloc(&ao_intflash_dma_done);

	FWT = FLASH_TIMING;
}
