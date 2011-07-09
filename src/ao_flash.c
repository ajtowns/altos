/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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
#include "at45db161d.h"

/* Total bytes of available storage */
__pdata uint32_t	ao_storage_total;

/* Block size - device is erased in these units. At least 256 bytes */
__pdata uint32_t	ao_storage_block;

/* Byte offset of config block. Will be ao_storage_block bytes long */
__pdata uint32_t	ao_storage_config;

/* Storage unit size - device reads and writes must be within blocks of this size. Usually 256 bytes. */
__pdata uint16_t	ao_storage_unit;

#define FLASH_CS		P1_1
#define FLASH_CS_INDEX		1

#define FLASH_BLOCK_SIZE_MAX	512

__xdata uint8_t ao_flash_mutex;

#define ao_flash_delay() do { \
	_asm nop _endasm; \
	_asm nop _endasm; \
	_asm nop _endasm; \
} while(0)

#define ao_flash_cs_low()	ao_spi_get_bit(FLASH_CS)

#define ao_flash_cs_high()	ao_spi_put_bit(FLASH_CS)

struct ao_flash_instruction {
	uint8_t	instruction;
	uint8_t	address[3];
} __xdata ao_flash_instruction;

static void
ao_flash_set_pagesize_512(void)
{
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_SET_CONFIG;
	ao_flash_instruction.address[0] = FLASH_SET_512_BYTE_0;
	ao_flash_instruction.address[1] = FLASH_SET_512_BYTE_1;
	ao_flash_instruction.address[2] = FLASH_SET_512_BYTE_2;
	ao_spi_send(&ao_flash_instruction, 4);
	ao_flash_cs_high();
}


static uint8_t
ao_flash_read_status(void)
{
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_READ_STATUS;
	ao_spi_send(&ao_flash_instruction, 1);
	ao_spi_recv(&ao_flash_instruction, 1);
	ao_flash_cs_high();
	return ao_flash_instruction.instruction;
}

#define FLASH_BLOCK_NONE	0xffff

static __xdata uint8_t ao_flash_data[FLASH_BLOCK_SIZE_MAX];
static __pdata uint16_t ao_flash_block = FLASH_BLOCK_NONE;
static __pdata uint8_t	ao_flash_block_dirty;
static __pdata uint8_t  ao_flash_write_pending;
static __pdata uint8_t	ao_flash_setup_done;
static __pdata uint8_t	ao_flash_block_shift;
static __pdata uint16_t	ao_flash_block_size;
static __pdata uint16_t	ao_flash_block_mask;

void
ao_storage_setup(void) __reentrant
{
	uint8_t	status;

	if (ao_flash_setup_done)
		return;

	ao_mutex_get(&ao_flash_mutex);
	if (ao_flash_setup_done) {
		ao_mutex_put(&ao_flash_mutex);
		return;
	}

	/* On first use, check to see if the flash chip has
	 * been programmed to use 512 byte pages. If not, do so.
	 * And then, because the flash part must be power cycled
	 * for that change to take effect, panic.
	 */
	status = ao_flash_read_status();

	if (!(status & FLASH_STATUS_PAGESIZE_512)) {
		ao_flash_set_pagesize_512();
		ao_panic(AO_PANIC_FLASH);
	}

	switch (status & 0x3c) {

	/* AT45DB321D */
	case 0x34:
		ao_flash_block_shift = 9;
		ao_storage_total = ((uint32_t) 4 * (uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB161D */
	case 0x2c:
		ao_flash_block_shift = 9;
		ao_storage_total = ((uint32_t) 2 * (uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB081D */
	case 0x24:
		ao_flash_block_shift = 8;
		ao_storage_total = ((uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB041D */
	case 0x1c:
		ao_flash_block_shift = 8;
		ao_storage_total = ((uint32_t) 512 * (uint32_t) 1024);
		break;

	/* AT45DB021D */
	case 0x14:
		ao_flash_block_shift = 8;
		ao_storage_total = ((uint32_t) 256 * (uint32_t) 1024);
		break;

	/* AT45DB011D */
	case 0x0c:
		ao_flash_block_shift = 8;
		ao_storage_total = ((uint32_t) 128 * (uint32_t) 1024);
		break;

	default:
		ao_panic(AO_PANIC_FLASH);
	}
	ao_flash_block_size = 1 << ao_flash_block_shift;
	ao_flash_block_mask = ao_flash_block_size - 1;

	ao_storage_block = ao_flash_block_size;
	ao_storage_config = ao_storage_total - ao_storage_block;
	ao_storage_unit = ao_flash_block_size;

	ao_flash_setup_done = 1;
	ao_mutex_put(&ao_flash_mutex);
}

static void
ao_flash_wait_write(void)
{
	if (ao_flash_write_pending) {
		for (;;) {
			uint8_t status = ao_flash_read_status();
			if ((status & FLASH_STATUS_RDY))
				break;
		}
		ao_flash_write_pending = 0;
	}
}

/* Write the current block to the FLASHPROM */
static void
ao_flash_write_block(void)
{
	ao_flash_wait_write();
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_WRITE;

	/* 13/14 block bits + 9/8 byte bits (always 0) */
	ao_flash_instruction.address[0] = ao_flash_block >> (16 - ao_flash_block_shift);
	ao_flash_instruction.address[1] = ao_flash_block << (ao_flash_block_shift - 8);
	ao_flash_instruction.address[2] = 0;
	ao_spi_send(&ao_flash_instruction, 4);
	ao_spi_send(ao_flash_data, ao_storage_block);
	ao_flash_cs_high();
	ao_flash_write_pending = 1;
}

/* Read the current block from the FLASHPROM */
static void
ao_flash_read_block(void)
{
	ao_flash_wait_write();
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_READ;

	/* 13/14 block bits + 9/8 byte bits (always 0) */
	ao_flash_instruction.address[0] = ao_flash_block >> (16 - ao_flash_block_shift);
	ao_flash_instruction.address[1] = ao_flash_block << (ao_flash_block_shift - 8);
	ao_flash_instruction.address[2] = 0;
	ao_spi_send(&ao_flash_instruction, 4);
	ao_spi_recv(ao_flash_data, ao_flash_block_size);
	ao_flash_cs_high();
}

static void
ao_flash_flush_internal(void)
{
	if (ao_flash_block_dirty) {
		ao_flash_write_block();
		ao_flash_block_dirty = 0;
	}
}

static void
ao_flash_fill(uint16_t block)
{
	if (block != ao_flash_block) {
		ao_flash_flush_internal();
		ao_flash_block = block;
		ao_flash_read_block();
	}
}

uint8_t
ao_storage_device_write(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant
{
	uint16_t block = (uint16_t) (pos >> ao_flash_block_shift);

	/* Transfer the data */
	ao_mutex_get(&ao_flash_mutex); {
		if (len != ao_flash_block_size)
			ao_flash_fill(block);
		else {
			ao_flash_flush_internal();
			ao_flash_block = block;
		}
		memcpy(ao_flash_data + (uint16_t) (pos & ao_flash_block_mask),
		       buf,
		       len);
		ao_flash_block_dirty = 1;
	} ao_mutex_put(&ao_flash_mutex);
	return 1;
}

uint8_t
ao_storage_device_read(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant
{
	uint16_t block = (uint16_t) (pos >> ao_flash_block_shift);

	/* Transfer the data */
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_fill(block);
		memcpy(buf,
		       ao_flash_data + (uint16_t) (pos & ao_flash_block_mask),
		       len);
	} ao_mutex_put(&ao_flash_mutex);
	return 1;
}

void
ao_storage_flush(void) __reentrant
{
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_flush_internal();
	} ao_mutex_put(&ao_flash_mutex);
}

uint8_t
ao_storage_erase(uint32_t pos) __reentrant
{
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_flush_internal();
		ao_flash_block = (uint16_t) (pos >> ao_flash_block_shift);
		memset(ao_flash_data, 0xff, ao_flash_block_size);
		ao_flash_block_dirty = 1;
	} ao_mutex_put(&ao_flash_mutex);
	return 1;
}

void
ao_storage_device_info(void) __reentrant
{
	uint8_t	status;

	ao_storage_setup();
	ao_mutex_get(&ao_flash_mutex); {
		status = ao_flash_read_status();
		printf ("Flash status: 0x%02x\n", status);
		printf ("Flash block shift: %d\n", ao_flash_block_shift);
		printf ("Flash block size: %d\n", ao_flash_block_size);
		printf ("Flash block mask: %d\n", ao_flash_block_mask);
		printf ("Flash device size: %ld\n", ao_storage_total);
	} ao_mutex_put(&ao_flash_mutex);
}

/*
 * To initialize the chip, set up the CS line and
 * the SPI interface
 */
void
ao_storage_device_init(void)
{
	/* set up CS */
	FLASH_CS = 1;
	P1DIR |= (1 << FLASH_CS_INDEX);
	P1SEL &= ~(1 << FLASH_CS_INDEX);
}
