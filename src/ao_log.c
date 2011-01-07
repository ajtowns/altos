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

static __pdata uint32_t	ao_log_current_pos;
static __pdata uint32_t ao_log_end_pos;
static __pdata uint32_t	ao_log_start_pos;
static __xdata uint8_t	ao_log_running;
static __xdata uint8_t	ao_log_mutex;

static uint8_t
ao_log_csum(__xdata uint8_t *b) __reentrant
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_record); i++)
		sum += *b++;
	return -sum;
}

uint8_t
ao_log_data(__xdata struct ao_log_record *log) __reentrant
{
	uint8_t wrote = 0;
	/* set checksum */
	log->csum = 0;
	log->csum = ao_log_csum((__xdata uint8_t *) log);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos)
			ao_log_running = 0;
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 log,
					 sizeof (struct ao_log_record));
			ao_log_current_pos += sizeof (struct ao_log_record);
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

void
ao_log_flush(void)
{
	ao_storage_flush();
}

static void ao_log_scan(void);

__xdata struct ao_log_record log;
__xdata uint16_t ao_flight_number;

static uint8_t
ao_log_dump_check_data(void)
{
	if (ao_log_csum((uint8_t *) &log) != 0)
		return 0;
	return 1;
}

__xdata uint8_t	ao_log_adc_pos;
__xdata enum flight_state ao_log_state;

/* a hack to make sure that ao_log_records fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_record))] ;

void
ao_log(void)
{
	ao_storage_setup();

	/* For now, use all of the available space */
	ao_log_current_pos = 0;
	ao_log_end_pos = ao_storage_config;

	ao_log_scan();

	while (!ao_log_running)
		ao_sleep(&ao_log_running);

	log.type = AO_LOG_FLIGHT;
	log.tick = ao_flight_tick;
	log.u.flight.ground_accel = ao_ground_accel;
	log.u.flight.flight = ao_flight_number;
	ao_log_data(&log);

	/* Write the whole contents of the ring to the log
	 * when starting up.
	 */
	ao_log_adc_pos = ao_adc_ring_next(ao_adc_head);
	for (;;) {
		/* Write samples to EEPROM */
		while (ao_log_adc_pos != ao_adc_head) {
			log.type = AO_LOG_SENSOR;
			log.tick = ao_adc_ring[ao_log_adc_pos].tick;
			log.u.sensor.accel = ao_adc_ring[ao_log_adc_pos].accel;
			log.u.sensor.pres = ao_adc_ring[ao_log_adc_pos].pres;
			ao_log_data(&log);
			if ((ao_log_adc_pos & 0x1f) == 0) {
				log.type = AO_LOG_TEMP_VOLT;
				log.tick = ao_adc_ring[ao_log_adc_pos].tick;
				log.u.temp_volt.temp = ao_adc_ring[ao_log_adc_pos].temp;
				log.u.temp_volt.v_batt = ao_adc_ring[ao_log_adc_pos].v_batt;
				ao_log_data(&log);
				log.type = AO_LOG_DEPLOY;
				log.tick = ao_adc_ring[ao_log_adc_pos].tick;
				log.u.deploy.drogue = ao_adc_ring[ao_log_adc_pos].sense_d;
				log.u.deploy.main = ao_adc_ring[ao_log_adc_pos].sense_m;
				ao_log_data(&log);
			}
			ao_log_adc_pos = ao_adc_ring_next(ao_log_adc_pos);
		}
		/* Write state change to EEPROM */
		if (ao_flight_state != ao_log_state) {
			ao_log_state = ao_flight_state;
			log.type = AO_LOG_STATE;
			log.tick = ao_flight_tick;
			log.u.state.state = ao_log_state;
			log.u.state.reason = 0;
			ao_log_data(&log);

			if (ao_log_state == ao_flight_landed)
				ao_log_stop();
		}

		/* Wait for a while */
		ao_delay(AO_MS_TO_TICKS(100));

		/* Stop logging when told to */
		while (!ao_log_running)
			ao_sleep(&ao_log_running);
	}
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

static void
ao_log_write_erase(uint8_t pos)
{
	erase.unused = 0x00;
	erase.flight = ao_flight_number;
	ao_storage_write(ao_log_erase_pos(pos),  &erase, sizeof (erase));
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

static void
ao_log_erase(uint8_t pos)
{
	ao_config_get();
	(void) pos;
//	ao_log_current_pos = pos * ao_config.flight_log_max;
//	ao_log_end_pos = ao_log_current_pos + ao_config.flight_log_max;
//	if (ao_log_end_pos > ao_storage_config)
//		return;

	ao_log_current_pos = 0;
	ao_log_end_pos = ao_storage_config;

	while (ao_log_current_pos < ao_log_end_pos) {
		ao_storage_erase(ao_log_current_pos);
		ao_log_current_pos += ao_storage_block;
	}
}

static uint16_t
ao_log_flight(uint8_t slot)
{
	(void) slot;
	if (!ao_storage_read(0,
			     &log,
			     sizeof (struct ao_log_record)))
		ao_panic(AO_PANIC_LOG);

	if (ao_log_dump_check_data() && log.type == AO_LOG_FLIGHT)
		return log.u.flight.flight;
	return 0;
}

static void
ao_log_scan(void)
{
	uint8_t		log_slot;
	uint8_t		log_avail = 0;
	uint16_t	log_flight;

	ao_config_get();

	ao_flight_number = 0;

	/* Scan the log space looking for an empty one, and find the biggest flight number */
	log_slot = 0;
	{
		log_flight = ao_log_flight(log_slot);
		if (log_flight) {
			if (++log_flight == 0)
				log_flight = 1;
			if (ao_flight_number == 0 ||
			    (int16_t) (log_flight - ao_flight_number) > 0) {
				ao_flight_number = log_flight;
			}
		} else
			log_avail |= 1 << log_slot;
	}

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

	/* If there are no log slots available, then
	 * do not log the next flight
	 */
	if (!log_avail) {
		ao_log_current_pos = 0;
		ao_log_end_pos = 0;
	} else {
		log_slot = ao_flight_number % log_slot;
		while (!((log_avail & (1 << log_slot)))) {
			if ((1 << log_slot) > log_avail)
				log_slot = 0;
			else
				log_slot++;
		}
//		ao_log_current_pos = log_slot * ao_config.flight_log_max;
//		ao_log_end_pos = ao_log_current_pos + ao_config.flight_log_max;
		ao_log_current_pos = 0;
		ao_log_end_pos = ao_storage_config;
	}

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

static __xdata struct ao_task ao_log_task;

void
ao_log_list(void) __reentrant
{
	uint8_t	slot;
	uint16_t flight;

	slot = 0;
	{
		flight = ao_log_flight(slot);
		if (flight)
			printf ("Flight %d\n", flight);
	}
}

void
ao_log_delete(void) __reentrant
{
	uint8_t slot;
	ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	slot = 0;
	/* Look for the flight log matching the requested flight */
	{
		if (ao_log_flight(slot) == ao_cmd_lex_i) {
			ao_log_current_pos = 0;
			ao_log_end_pos = ao_storage_config;
			while (ao_log_current_pos < ao_log_end_pos) {
				/*
				 * Check to see if we've reached the end of
				 * the used memory to avoid re-erasing the same
				 * memory over and over again
				 */
				if (ao_storage_read(ao_log_current_pos,
						    &log,
						    sizeof (struct ao_log_record))) {
					for (slot = 0; slot < sizeof (struct ao_log_record); slot++)
						if (((uint8_t *) &log)[slot] != 0xff)
							break;
					if (slot == sizeof (struct ao_log_record))
						break;
				}
				ao_storage_erase(ao_log_current_pos);
				ao_log_current_pos += ao_storage_block;
			}
			puts("Erased\n");
			return;
		}
	}
	printf("No such flight: %d\n", ao_cmd_lex_i);
}

__code struct ao_cmds ao_log_cmds[] = {
	{ 'l',	ao_log_list,	"l                                  List stored flight logs" },
	{ 'd',	ao_log_delete,	"d <flight-number>                  Delete stored flight" },
	{ 0,	ao_log_delete,	NULL },
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
