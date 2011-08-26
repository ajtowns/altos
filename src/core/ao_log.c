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

__pdata uint32_t ao_log_current_pos;
__pdata uint32_t ao_log_end_pos;
__pdata uint32_t ao_log_start_pos;
__xdata uint8_t	ao_log_running;
__pdata enum flight_state ao_log_state;
__xdata uint16_t ao_flight_number;

__code uint8_t ao_log_format = AO_LOG_FORMAT_FULL;

void
ao_log_flush(void)
{
	ao_storage_flush();
}

/*
 * When erasing a flight log, make sure the config block
 * has an up-to-date version of the current flight number
 */

struct ao_log_erase {
	uint8_t	unused;
	uint16_t flight;
};

static __xdata struct ao_log_erase erase;

#define LOG_MAX_ERASE	16

static uint32_t
ao_log_erase_pos(uint8_t i)
{
	return i * sizeof (struct ao_log_erase) + AO_STORAGE_ERASE_LOG;
}

void
ao_log_write_erase(uint8_t pos)
{
	erase.unused = 0x00;
	erase.flight = ao_flight_number;
	ao_storage_write(ao_log_erase_pos(pos),  &erase, sizeof (erase));
	ao_storage_flush();
}

static void
ao_log_read_erase(uint8_t pos)
{
	ao_storage_read(ao_log_erase_pos(pos), &erase, sizeof (erase));
}


static void
ao_log_erase_mark(void)
{
	uint8_t				i;

	for (i = 0; i < LOG_MAX_ERASE; i++) {
		ao_log_read_erase(i);
		if (erase.unused == 0 && erase.flight == ao_flight_number)
			return;
		if (erase.unused == 0xff) {
			ao_log_write_erase(i);
			return;
		}
	}
	ao_config_put();
}

static uint8_t
ao_log_slots()
{
	return (uint8_t) (ao_storage_config / ao_config.flight_log_max);
}

uint32_t
ao_log_pos(uint8_t slot)
{
	return ((slot) * ao_config.flight_log_max);
}

static uint16_t
ao_log_max_flight(void)
{
	uint8_t		log_slot;
	uint8_t		log_slots;
	uint16_t	log_flight;
	uint16_t	max_flight = 0;

	/* Scan the log space looking for the biggest flight number */
	log_slots = ao_log_slots();
	for (log_slot = 0; log_slot < log_slots; log_slot++) {
		log_flight = ao_log_flight(log_slot);
		if (!log_flight)
			continue;
		if (max_flight == 0 || (int16_t) (log_flight - max_flight) > 0)
			max_flight = log_flight;
	}
	return max_flight;
}

void
ao_log_scan(void) __reentrant
{
	uint8_t		log_slot;
	uint8_t		log_slots;
	uint8_t		log_want;

	ao_config_get();

	ao_flight_number = ao_log_max_flight();
	if (ao_flight_number)
		if (++ao_flight_number == 0)
			ao_flight_number = 1;

	/* Now look through the log of flight numbers from erase operations and
	 * see if the last one is bigger than what we found above
	 */
	for (log_slot = LOG_MAX_ERASE; log_slot-- > 0;) {
		ao_log_read_erase(log_slot);
		if (erase.unused == 0) {
			if (ao_flight_number == 0 ||
			    (int16_t) (erase.flight - ao_flight_number) > 0)
				ao_flight_number = erase.flight;
			break;
		}
	}
	if (ao_flight_number == 0)
		ao_flight_number = 1;

	/* With a flight number in hand, find a place to write a new log,
	 * use the target flight number to index the available log slots so
	 * that we write logs to each spot about the same number of times.
	 */

	/* Find a log slot for the next flight, if available */
	ao_log_current_pos = ao_log_end_pos = 0;
	log_slots = ao_log_slots();
	log_want = (ao_flight_number - 1) % log_slots;
	log_slot = log_want;
	do {
		if (ao_log_flight(log_slot) == 0) {
			ao_log_current_pos = ao_log_pos(log_slot);
			ao_log_end_pos = ao_log_current_pos + ao_config.flight_log_max;
			break;
		}
		if (++log_slot >= log_slots)
			log_slot = 0;
	} while (log_slot != log_want);

	ao_wakeup(&ao_flight_number);
}

void
ao_log_start(void)
{
	/* start logging */
	ao_log_running = 1;
	ao_wakeup(&ao_log_running);
}

void
ao_log_stop(void)
{
	ao_log_running = 0;
	ao_log_flush();
}

uint8_t
ao_log_present(void)
{
	return ao_log_max_flight() != 0;
}

uint8_t
ao_log_full(void)
{
	return ao_log_current_pos == ao_log_end_pos;
}

static __xdata struct ao_task ao_log_task;

void
ao_log_list(void) __reentrant
{
	uint8_t	slot;
	uint8_t slots;
	uint16_t flight;

	slots = ao_log_slots();
	for (slot = 0; slot < slots; slot++)
	{
		flight = ao_log_flight(slot);
		if (flight)
			printf ("flight %d start %x end %x\n",
				flight,
				(uint16_t) (ao_log_pos(slot) >> 8),
				(uint16_t) (ao_log_pos(slot+1) >> 8));
	}
	printf ("done\n");
}

void
ao_log_delete(void) __reentrant
{
	uint8_t slot;
	uint8_t slots;

	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;

	slots = ao_log_slots();
	/* Look for the flight log matching the requested flight */
	if (ao_cmd_lex_i) {
		for (slot = 0; slot < slots; slot++) {
			if (ao_log_flight(slot) == ao_cmd_lex_i) {
				ao_log_erase_mark();
				ao_log_current_pos = ao_log_pos(slot);
				ao_log_end_pos = ao_log_current_pos + ao_config.flight_log_max;
				while (ao_log_current_pos < ao_log_end_pos) {
					uint8_t	i;
					static __xdata uint8_t b;

					/*
					 * Check to see if we've reached the end of
					 * the used memory to avoid re-erasing the same
					 * memory over and over again
					 */
					for (i = 0; i < 16; i++) {
						if (ao_storage_read(ao_log_current_pos + i, &b, 1))
							if (b != 0xff)
								break;
					}
					if (i == 16)
						break;
					ao_storage_erase(ao_log_current_pos);
					ao_log_current_pos += ao_storage_block;
				}
				puts("Erased");
				return;
			}
		}
	}
	printf("No such flight: %d\n", ao_cmd_lex_i);
}

__code struct ao_cmds ao_log_cmds[] = {
	{ ao_log_list,	"l\0List flight logs" },
	{ ao_log_delete,	"d <flight-number>\0Delete flight" },
	{ 0,	NULL },
};

void
ao_log_init(void)
{
	ao_log_running = 0;

	/* For now, just log the flight starting at the begining of eeprom */
	ao_log_state = ao_flight_invalid;

	ao_cmd_register(&ao_log_cmds[0]);

	/* Create a task to log events to eeprom */
	ao_add_task(&ao_log_task, ao_log, "log");
}
