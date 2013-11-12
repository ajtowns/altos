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

#ifndef _AO_STORAGE_H_
#define _AO_STORAGE_H_

/*
 * Storage interface, provided by one of the eeprom or flash
 * drivers
 */

#ifndef ao_storage_pos_t
#define ao_storage_pos_t uint32_t
#endif

typedef ao_storage_pos_t ao_pos_t;

/* Total bytes of available storage */
extern __pdata ao_pos_t	ao_storage_total;

/* Block size - device is erased in these units. At least 256 bytes */
extern __pdata ao_pos_t	ao_storage_block;

#ifndef USE_STORAGE_CONFIG
#define USE_STORAGE_CONFIG 1
#endif

#if USE_STORAGE_CONFIG
/* Byte offset of config block. Will be ao_storage_block bytes long */
extern __pdata ao_pos_t	ao_storage_config;

#define ao_storage_log_max	ao_storage_config
#else
#define ao_storage_log_max	ao_storage_total
#endif

/* Storage unit size - device reads and writes must be within blocks of this size. Usually 256 bytes. */
extern __pdata uint16_t ao_storage_unit;

/* Initialize above values. Can only be called once the OS is running */
void
ao_storage_setup(void) __reentrant;

/* Write data. Returns 0 on failure, 1 on success */
uint8_t
ao_storage_write(ao_pos_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Read data. Returns 0 on failure, 1 on success */
uint8_t
ao_storage_read(ao_pos_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Erase a block of storage. This always clears ao_storage_block bytes */
uint8_t
ao_storage_erase(ao_pos_t pos) __reentrant;

/* Flush any pending writes to stable storage */
void
ao_storage_flush(void) __reentrant;

/* Initialize the storage code */
void
ao_storage_init(void);

/*
 * Low-level functions wrapped by ao_storage.c
 */

/* Read data within a storage unit */
uint8_t
ao_storage_device_read(ao_pos_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Write data within a storage unit */
uint8_t
ao_storage_device_write(ao_pos_t pos, __xdata void *buf, uint16_t len) __reentrant;

/* Initialize low-level device bits */
void
ao_storage_device_init(void);

/* Print out information about flash chips */
void
ao_storage_device_info(void) __reentrant;

#endif /* _AO_STORAGE_H_ */
