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

#define FCTL_BUSY		(1 << 7)
#define FCTL_SWBSY		(1 << 6)
#define FCTL_CONTRD_ENABLE	(1 << 4)
#define FCTL_WRITE		(1 << 1)
#define FCTL_ERASE		(1 << 0)

#define ENDOFCODE  (0x51f0 + 1500)
#define NUM_PAGES ((0x8000-ENDOFCODE)/1024)
#define SIZE      (1024*NUM_PAGES)
#define LOCN      (0x8000 - SIZE)

#if NUM_PAGES < 1
#error "Too few pages"
#endif

#if LOCN % 1024 != 0
#error "Pages aren't aligned properly"
#endif

__xdata __at(LOCN) uint8_t ao_intflash[SIZE];

/* Total bytes of available storage */
__xdata uint32_t	ao_storage_total = sizeof(ao_intflash);

/* Block size - device is erased in these units. */
__xdata uint32_t	ao_storage_block = 1024;

/* Byte offset of config block. Will be ao_storage_block bytes long */
__xdata uint32_t	ao_storage_config = sizeof(ao_intflash);

/* Storage unit size - device reads and writes must be within blocks of this size. */
__xdata uint16_t	ao_storage_unit = 1024;

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

	while (FCTL & FCTL_BUSY)
		;

	FWT = 0x1F; // 21000 * f / 16e9 ; f = system freq = 24MHz
	FADDRH = addr >> 8;
	FADDRL = addr & ~0xFF00;

	_asm
	.even
	orl _FCTL, #FCTL_ERASE;		; FCTL |=  FCTL_ERASE
	nop;				; Required, see datasheet.
	_endasm;

	while (FCTL & FCTL_BUSY)
		;

	return 1;
}

/*
 * Write to flash
 */

static void
word_aligned_write(uint32_t pos, __xdata void *d, uint16_t len) __reentrant
{
	uint16_t addr;

	addr = ((uint16_t)(ao_intflash + pos) >> 1);

	ao_dma_set_transfer(ao_intflash_dma, d, &X_FWDATA,
		DMA_LEN_HIGH_VLEN_LEN | len,
		DMA_CFG0_WORDSIZE_8 | DMA_CFG0_TMODE_SINGLE |
			DMA_CFG0_TRIGGER_FLASH,
		DMA_CFG1_SRCINC_1 | DMA_CFG1_DESTINC_0 |
			DMA_CFG1_IRQMASK | DMA_CFG1_PRIORITY_HIGH);

	while (FCTL & FCTL_BUSY)
		;

	FWT = 0x1F; // 21000 * f / 16e9 ; f = system freq = 24MHz

	FADDRH = addr >> 8;
	FADDRL = addr & ~0xFF00;

	ao_dma_start(ao_intflash_dma);

	_asm
	.even
	orl _FCTL, #FCTL_WRITE;		; FCTL |=  FCTL_WRITE
	_endasm;

	__critical while (!ao_intflash_dma_done)
		ao_sleep(&ao_intflash_dma_done);

	while (FCTL & (FCTL_BUSY | FCTL_SWBSY))
		;
}

uint8_t
ao_storage_device_write(uint32_t pos, __xdata void *v, uint16_t len) __reentrant
{
	static __xdata uint8_t b[2];
	__xdata uint8_t *d = v;
	uint8_t oddlen;

	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	if (len == 0)
		return 1;

	if (pos & 1) {
		b[0] = ~0;
		b[1] = d[0];
		word_aligned_write(pos-1, b, 2);
		pos++;
		len--;
		d++;
	}
	oddlen = len & 1;
	len &= ~1;
	if (len > 0) {
		word_aligned_write(pos, d, len);
	}
	if (oddlen) {
		b[0] = d[len];
		b[1] = ~0;
		word_aligned_write(pos+len, b, 2);
	}

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
	printf ("Using internal flash, starting at 0x%04x\n", LOCN);
}

void
ao_storage_device_init(void)
{
	ao_intflash_dma = ao_dma_alloc(&ao_intflash_dma_done);
}
