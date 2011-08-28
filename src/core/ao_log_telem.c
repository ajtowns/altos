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

__code uint8_t ao_log_format = AO_LOG_FORMAT_TELEMETRY;

static __data uint8_t	ao_log_monitor_pos;

void
ao_log_single(void)
{
	ao_storage_setup();

	/* This can take a while, so let the rest
	 * of the system finish booting before we start
	 */
	ao_delay(AO_SEC_TO_TICKS(2));

	ao_log_running = 1;
	ao_log_single_restart();
	for (;;) {
		while (!ao_log_running)
			ao_sleep(&ao_log_running);

		ao_log_monitor_pos = ao_monitor_head;
		while (ao_log_running) {
			/* Write samples to EEPROM */
			while (ao_log_monitor_pos != ao_monitor_head) {
				memcpy(&ao_log_single_write_data.telemetry,
				       &ao_monitor_ring[ao_log_monitor_pos],
				       AO_LOG_SINGLE_SIZE);
				ao_log_single_write();
				ao_log_monitor_pos = ao_monitor_ring_next(ao_log_monitor_pos);
			}
			/* Wait for more telemetry data to arrive */
			ao_sleep(DATA_TO_XDATA(&ao_monitor_head));
		}
	}
}

void
ao_log_single_list(void)
{
	if (ao_log_current_pos != 0)
		printf("flight 1 start %x end %x\n",
		       0,
		       (uint16_t) ((ao_log_current_pos + 0xff) >> 8));
	printf ("done\n");
}

void
ao_log_single_extra_query(void)
{
}
