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
#include <ao_log_micro.h>
#include <ao_async.h>

#if HAS_EEPROM

ao_pos_t	ao_log_micro_pos;

void
ao_log_micro_data(uint32_t data)
{
	ao_storage_write(ao_log_micro_pos, &data, sizeof (data));
	ao_log_micro_pos += sizeof (data);
}

uint32_t	ao_log_last_ground;
uint32_t	ao_log_last_done;

uint8_t
ao_log_micro_scan(void)
{
	uint32_t	data;
	ao_pos_t	pos;

	ao_storage_read(0, &data, sizeof (data));
	if ((data & AO_LOG_MICRO_MASK) != AO_LOG_MICRO_GROUND)
		return 0;

	ao_log_last_ground = data & ~(AO_LOG_MICRO_MASK);
	for (pos = 4; pos < ao_storage_total; pos += 4) {
		ao_storage_read(pos, &data, sizeof (data));
		if ((data & AO_LOG_MICRO_MASK) == AO_LOG_MICRO_GROUND) {
			ao_log_last_done = data & ~(AO_LOG_MICRO_MASK);
			return 1;
		}
	}
	return 0;
}

void
ao_log_micro_dump(void)
{
	ao_pos_t	pos;
	uint8_t		data[4];
	uint8_t		i;

	for (pos = 0; pos < ao_storage_total; pos += 4) {
		ao_storage_read(pos, data, 4);
		for (i = 0; i < 4; i++)
			ao_async_byte(data[i]);
		if (data[3] == (uint8_t) (AO_LOG_MICRO_GROUND >> 24))
			break;
	}
}

#endif
