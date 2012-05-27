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
#include "ao_product.h"
#include "ao_log.h"
#include "ao_companion.h"

static uint8_t	ao_log_data_pos;

__code uint8_t ao_log_format = AO_LOG_FORMAT_TELESCIENCE;

static void
ao_log_telescience_csum(void) __reentrant
{
	__xdata uint8_t *b = ao_log_single_write_data.bytes;
	uint8_t	sum = 0x5a;
	uint8_t	i;

	ao_log_single_write_data.telescience.csum = 0;
	for (i = 0; i < sizeof (struct ao_log_telescience); i++)
		sum += *b++;
	ao_log_single_write_data.telescience.csum = -sum;
}

void
ao_log_single(void)
{
	ao_storage_setup();

	/* This can take a while, so let the rest
	 * of the system finish booting before we start
	 */
	ao_delay(AO_SEC_TO_TICKS(10));

	ao_log_single_restart();
	for (;;) {
		while (!ao_log_running)
			ao_sleep(&ao_log_running);

		ao_log_start_pos = ao_log_current_pos;
		ao_log_single_write_data.telescience.type = AO_LOG_TELESCIENCE_START;
		ao_log_single_write_data.telescience.tick = ao_time();
		ao_log_single_write_data.telescience.adc[0] = ao_companion_command.serial;
		ao_log_single_write_data.telescience.adc[1] = ao_companion_command.flight;
		ao_log_telescience_csum();
		ao_log_single_write();
		/* Write the whole contents of the ring to the log
		 * when starting up.
		 */
		ao_log_data_pos = ao_data_ring_next(ao_data_head);
		ao_log_single_write_data.telescience.type = AO_LOG_TELESCIENCE_DATA;
		while (ao_log_running) {
			/* Write samples to EEPROM */
			while (ao_log_data_pos != ao_data_head) {
				ao_log_single_write_data.telescience.tick = ao_data_ring[ao_log_data_pos].tick;
				memcpy(&ao_log_single_write_data.telescience.adc, (void *) ao_data_ring[ao_log_data_pos].adc.adc,
				       AO_LOG_TELESCIENCE_NUM_ADC * sizeof (uint16_t));
				ao_log_telescience_csum();
				ao_log_single_write();
				ao_log_data_pos = ao_data_ring_next(ao_log_data_pos);
			}
			/* Wait for more ADC data to arrive */
			ao_sleep((void *) &ao_data_head);
		}
		memset(&ao_log_single_write_data.telescience.adc, '\0', sizeof (ao_log_single_write_data.telescience.adc));
	}
}

void
ao_log_single_list(void)
{
	uint32_t	pos;
	uint32_t	start = 0;
	uint8_t		flight = 0;

	for (pos = 0; ; pos += sizeof (struct ao_log_telescience)) {
		if (pos >= ao_storage_config ||
		    !ao_log_single_read(pos) ||
		    ao_log_single_read_data.telescience.type == AO_LOG_TELESCIENCE_START)
		{
			if (pos != start) {
				printf("flight %d start %x end %x\n",
				       flight,
				       (uint16_t) (start >> 8),
				       (uint16_t) ((pos + 0xff) >> 8)); flush();
			}
			if (ao_log_single_read_data.telescience.type != AO_LOG_TELESCIENCE_START)
				break;
			start = pos;
			flight++;
		}
	}
	printf ("done\n");
}

void
ao_log_single_extra_query(void)
{
	printf("log data tick: %04x\n", ao_log_single_write_data.telescience.tick);
	printf("TM data tick: %04x\n", ao_log_single_write_data.telescience.tm_tick);
	printf("TM state: %d\n", ao_log_single_write_data.telescience.tm_state);
	printf("TM serial: %d\n", ao_companion_command.serial);
	printf("TM flight: %d\n", ao_companion_command.flight);
}
