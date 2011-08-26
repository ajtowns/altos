/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

uint8_t
ao_storage_read(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant
{
	uint16_t this_len;
	uint16_t this_off;

	ao_storage_setup();
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	while (len) {

		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) pos & (ao_storage_unit - 1);
		this_len = ao_storage_unit - this_off;
		if (this_len > len)
			this_len = len;

		if (!ao_storage_device_read(pos, buf, this_len))
			return 0;

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

uint8_t
ao_storage_write(uint32_t pos, __xdata void *buf, uint16_t len) __reentrant
{
	uint16_t this_len;
	uint16_t this_off;

	ao_storage_setup();
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	while (len) {

		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) pos & (ao_storage_unit - 1);
		this_len = ao_storage_unit - this_off;
		if (this_len > len)
			this_len = len;

		if (!ao_storage_device_write(pos, buf, this_len))
			return 0;

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

static __xdata uint8_t storage_data[8];

static void
ao_storage_dump(void) __reentrant
{
	uint8_t i, j;

	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	for (i = 0; ; i += 8) {
		if (ao_storage_read(((uint32_t) (ao_cmd_lex_i) << 8) + i,
				  storage_data,
				  8)) {
			ao_cmd_put16((uint16_t) i);
			for (j = 0; j < 8; j++) {
				putchar(' ');
				ao_cmd_put8(storage_data[j]);
			}
			putchar ('\n');
		}
		if (i == 248)
			break;
	}
}

#if 0

/* not enough space for this today
 */
static void
ao_storage_store(void) __reentrant
{
	uint16_t block;
	uint8_t i;
	uint16_t len;
	static __xdata uint8_t b;
	uint32_t addr;

	ao_cmd_hex();
	block = ao_cmd_lex_i;
	ao_cmd_hex();
	i = ao_cmd_lex_i;
	addr = ((uint32_t) block << 8) | i;
	ao_cmd_hex();
	len = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	while (len--) {
		ao_cmd_hex();
		if (ao_cmd_status != ao_cmd_success)
			return;
		b = ao_cmd_lex_i;
		ao_storage_write(addr, &b, 1);
		addr++;
	}
}
#endif

void
ao_storage_zap(void) __reentrant
{
	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_storage_erase((uint32_t) ao_cmd_lex_i << 8);
}

void
ao_storage_zapall(void) __reentrant
{
	uint32_t	pos;

	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	for (pos = 0; pos < ao_storage_config; pos += ao_storage_block)
		ao_storage_erase(pos);
}

void
ao_storage_info(void) __reentrant
{
	printf("Storage size: %ld\n", ao_storage_total);
	printf("Storage erase unit: %ld\n", ao_storage_block);
	ao_storage_device_info();
}

__code struct ao_cmds ao_storage_cmds[] = {
	{ ao_storage_info, "f\0Show storage" },
	{ ao_storage_dump, "e <block>\0Dump flash" },
#ifdef HAS_STORAGE_DBG
	{ ao_storage_store, "w <block> <start> <len> <data> ...\0Write data to flash" },
#endif
	{ ao_storage_zap, "z <block>\0Erase <block>" },
	{ ao_storage_zapall,"Z <key>\0Erase all. <key> is doit with D&I" },
	{ 0, NULL },
};

void
ao_storage_init(void)
{
	ao_storage_device_init();
	ao_cmd_register(&ao_storage_cmds[0]);
}
