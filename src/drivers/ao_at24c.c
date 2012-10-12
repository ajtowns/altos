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

#if HAS_EEPROM
#define AO_AT24C_ADDR		0xa0
#define AO_AT24C_ADDR_WRITE	(AO_AT24C_ADDR|0)
#define AO_AT24C_ADDR_READ	(AO_AT24C_ADDR|1)
#define AO_AT24C_PAGE_LEN	128

/* Total bytes of available storage */
__pdata ao_pos_t	ao_storage_total = 64l * 1024l;

/* Storage unit size - device reads and writes must be within blocks of this size. */
__pdata uint16_t	ao_storage_unit = 128;

static void
ao_at24c_set_address(uint8_t addr, ao_pos_t pos)
{
	uint8_t	a[2];

	a[0] = pos >> 8;
	a[1] = pos;
	ao_i2c_start_bus(addr);
	ao_i2c_send_bus(a, 2, 0);
}

/*
 * Erase the specified sector
 */
uint8_t
ao_storage_erase(ao_pos_t pos) __reentrant
{
	if (pos >= ao_storage_total || pos + AO_AT24C_PAGE_LEN > ao_storage_total)
		return 0;

	ao_mutex_get(&ao_at24c_mutex);
	ao_at24c_set_address(AO_AT24C_ADDR_WRITE, pos);
	ao_i2c_send_fixed_bus(0xff, AO_AT24C_PAGE_LEN, 1);
	ao_mutex_put(&ao_at24c_mutex);
	return 1;
}

/*
 * Write to flash
 */
uint8_t
ao_storage_device_write(ao_pos_t pos, __xdata void *d, uint16_t len) __reentrant
{
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;

	ao_mutex_get(&ao_m25_mutex);
	ao_at24c_set_address(AO_AT24C_ADDR_WRITE, pos);
	ao_i2c_send_bus(d, len, 1);
	ao_mutex_put(&ao_m25_mutex);
	return 1;
}

/*
 * Read from flash
 */
uint8_t
ao_storage_device_read(ao_pos_t pos, __xdata void *d, uint16_t len) __reentrant
{
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	ao_mutex_get(&ao_m25_mutex);
	ao_at24c_set_address(AO_AT24C_ADDR_READ, pos);
	ao_i2c_recv_bus(d, len, 1);
	ao_mutex_put(&ao_m25_mutex);
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
ao_storage_device_init(void)
{
}
#endif
