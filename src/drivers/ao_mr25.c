/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

/* Total bytes of available storage */
__pdata uint32_t	ao_storage_total;

/* Block size - device is erased in these units. At least 256 bytes */
__pdata uint32_t	ao_storage_block;

/* Byte offset of config block. Will be ao_storage_block bytes long */
__pdata uint32_t	ao_storage_config;

/* Storage unit size - device reads and writes must be within blocks of this size. Usually 256 bytes. */
__pdata uint16_t	ao_storage_unit;

/*
 * MRAM is entirely random access; no erase operations are required,
 * nor are reads or writes restricted to a particular alignment.
 */

#define MR25_WREN	0x06	/* Write Enable */
#define MR25_WRDI	0x04	/* Write Disable */
#define MR25_RDSR	0x05	/* Read Status Register */
#define MR25_WRSR	0x01	/* Write Status Register */
#define MR25_READ	0x03	/* Read Data Bytes */
#define MR25_WRITE	0x02	/* Write Data Bytes */

/*
 * Status register bits
 */

#define MR25_STATUS_SRWD	(1 << 7)	/* Status register write disable */
#define MR25_STATUS_BP_MASK	(3 << 2)	/* Block protect bits */
#define MR25_STATUS_BP_SHIFT	(2)
#define MR25_STATUS_WEL		(1 << 1)	/* Write enable latch */

static __xdata uint8_t ao_mr25_mutex;

/*
 * This little array is abused to send and receive data. A particular
 * caution -- the read and write addresses are written into the last
 * three bytes of the array by ao_mr25_set_page_address and then the
 * first byte is used by ao_mr25_write_enable, neither of which touch
 * those last three bytes.
 */

static __xdata uint8_t	ao_mr25_instruction[4];

#define MR25_SELECT()		ao_spi_get_mask(AO_MR25_SPI_CS_PORT,(1 << AO_MR25_SPI_CS_PIN),AO_MR25_SPI_BUS, AO_SPI_SPEED_FAST)
#define MR25_DESELECT()		ao_spi_put_mask(AO_MR25_SPI_CS_PORT,(1 << AO_MR25_SPI_CS_PIN),AO_MR25_SPI_BUS)

/*
 * Set the write enable latch so that page program and sector
 * erase commands will work. Also mark the chip as busy writing
 * so that future operations will block until the WIP bit goes off
 */
static void
ao_mr25_write_enable(void)
{
	MR25_SELECT();
	ao_mr25_instruction[0] = MR25_WREN;
	ao_spi_send(&ao_mr25_instruction, 1, AO_MR25_SPI_BUS);
	MR25_DESELECT();
}


static void
ao_mr25_set_address(uint32_t pos)
{
	ao_mr25_instruction[1] = pos >> 16;
	ao_mr25_instruction[2] = pos >> 8;
	ao_mr25_instruction[3] = pos;
}

/*
 * Erase the specified sector (no-op for MRAM)
 */
uint8_t
ao_storage_erase(uint32_t pos) __reentrant
{
	if (pos >= ao_storage_total || pos + ao_storage_block > ao_storage_total)
		return 0;
	return 1;
}

/*
 * Write to flash
 */
uint8_t
ao_storage_device_write(uint32_t pos, __xdata void *d, uint16_t len) __reentrant
{
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;

	ao_mutex_get(&ao_mr25_mutex);

	ao_mr25_set_address(pos);
	ao_mr25_write_enable();

	ao_mr25_instruction[0] = MR25_WRITE;
	MR25_SELECT();
	ao_spi_send(ao_mr25_instruction, 4, AO_MR25_SPI_BUS);
	ao_spi_send(d, len, AO_MR25_SPI_BUS);
	MR25_DESELECT();

	ao_mutex_put(&ao_mr25_mutex);
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
	ao_mutex_get(&ao_mr25_mutex);

	ao_mr25_set_address(pos);

	ao_mr25_instruction[0] = MR25_READ;
	MR25_SELECT();
	ao_spi_send(ao_mr25_instruction, 4, AO_MR25_SPI_BUS);
	ao_spi_recv(d, len, AO_MR25_SPI_BUS);
	MR25_DESELECT();

	ao_mutex_put(&ao_mr25_mutex);
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
	printf ("Detected chips 1 size %d\n", ao_storage_total >> 8);
}

void
ao_storage_device_init(void)
{
	ao_storage_total = 512 * 1024;	/* 4Mb */
	ao_storage_block = 256;
	ao_storage_config = ao_storage_total - ao_storage_block;
	ao_storage_unit = 256;
	ao_spi_init_cs (AO_MR25_SPI_CS_PORT, (1 << AO_MR25_SPI_CS_PIN));
}
