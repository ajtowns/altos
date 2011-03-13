/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 * Copyright © 2011 Anthony Towns <aj@erisian.com.au>
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

__xdata struct ao_log_record log; // XXX: used by ao_flight_log, not us
__xdata uint16_t ao_flight_number;

static __pdata uint32_t	ao_log_current_pos;
static __pdata uint32_t ao_log_end_pos;
static __xdata uint8_t	ao_log_running;
static __xdata uint8_t	ao_log_mutex;

#if 0
static uint8_t
ao_log_csum(__xdata uint8_t *b) __reentrant
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_record); i++)
		sum += *b++;
	return -sum;
}
#endif

static __xdata uint16_t log_data[16] = {0};
const static uint8_t incrementer = ~0;

static uint8_t
ao_log_bytes(uint16_t d) __reentrant
{
	uint8_t wrote = 0;
	uint8_t log_code = d & 0x000F;
	ao_mutex_get(&ao_log_mutex); do {
		if (!ao_log_running)
			break;
		if (log_code != incrementer && log_data[log_code] == d) {
			wrote = 1;
			break;
		}
		if (ao_log_current_pos >= ao_log_end_pos) {
			ao_log_stop();
			break;
		}

		wrote = 1;
		log_data[log_code] = d;
		ao_storage_write(ao_log_current_pos,
				 &log_data[log_code],
				 sizeof (log_data[0]));
		ao_log_current_pos += sizeof (log_data[0]);
	} while(0); ao_mutex_put(&ao_log_mutex);
	return wrote;
}

#define LOG_8b(data, subcode, code) \
    LOG_2B( (data), ((((subcode) & 0xF) << 4) | (code & (0xF))) )

#define LOG_2B(hi, lo) \
    ao_log_data(((uint16_t)((hi) & 0xFF) << 8) | (uint16_t)((lo) & 0xFF))

#define LOG_HI_12b(data, code) \
    ao_log_data((((uint16_t)(data) & 0xFFF0) << 8) | (uint16_t)(code) & 0x0F)

uint8_t
ao_log_data(__xdata struct ao_log_record *log) __reentrant
{
	uint8_t wrote = 0;

	if (!LOG_8b((log->tick & 0x00FF), 0, 0))
		return 0;
	if (!LOG_8b((log->tick & 0xFF00) >> 8, 2, 0))
		return 0;

	switch (log->type) {
	  case AO_LOG_FLIGHT:
		wrote |= LOG_8b(log->u.flight.flight, 3, 0);
		break;
	  case AO_LOG_SENSOR:
		wrote |= LOG_HI_12b(log->u.sensor.pres, 2);
		wrote |= LOG_HI_12b(log->u.sensor.accel, 3);
		break;
	  case AO_LOG_TEMP_VOLT:
		wrote |= LOG_HI_12b(log->u.temp_volt.temp, 4);
		wrote |= LOG_HI_12b(log->u.temp_volt.v_batt, 5);
		break;
	  case AO_LOG_DEPLOY:
		wrote |= LOG_HI_12b(log->u.deploy.drogue, 6);
		wrote |= LOG_HI_12b(log->u.deploy.main, 7);
		break;
	  case AO_LOG_STATE:
		wrote |= LOG_8b(log->u.state.reason, 2, 0);
		wrote |= LOG_8b(log->u.state.state, 1, 0);
		break;
#if 0
	  case AO_LOG_GPS_TIME:
		// gps_time.hour, minute, second, flags;
	  case AO_LOG_GPS_DATE:
		// gps_date.year, month, day;
	  case AO_LOG_GPS_LAT:
		// gps_latitude;
	  case AO_LOG_GPS_LON:
		// gps_longitude;
	  case AO_LOG_GPS_ALT:
		// gps_altitude.altitude;
	  case AO_LOG_GPS_SAT:
		// gps_sat.svid, c_n;
#endif
	  default:
		break;
	}
	return wrote;
}

static void
ao_log_flush(void)
{
	ao_storage_flush();
}

void
ao_log_write_erase(uint8_t pos)
{
	(void)pos;
}

static uint16_t
ao_log_flight(void)
{
	if (!ao_storage_read(0,
			     &log_data[0],
			     sizeof (struct ao_log_record)))
		return 0;

	if ((log_data[0] & 0x00FF) == 0x0030)
		return (log_data[0] >> 8);
	return 0;
}

static uint16_t
ao_log_max_flight(void)
{
	return ao_log_flight();
}

static void
ao_log_scan(void) __reentrant
{
	ao_config_get();

	ao_flight_number = ao_log_flight();
	if (ao_flight_number) {
		ao_log_current_pos = ao_log_end_pos = 0;
		ao_flight_number++;
	} else {
		ao_flight_number = 4;
		ao_log_current_pos = 0;
		ao_log_end_pos = ao_storage_config;
	}

	ao_wakeup(&ao_flight_number);
}

void
ao_log_waitforlogging(void) __reentrant __critical
{
	while (!ao_log_running)
		ao_sleep(&ao_log_running);
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

static void
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

static void
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
					/*
					 * Check to see if we've reached the end of
					 * the used memory to avoid re-erasing the same
					 * memory over and over again
					 */
					if (ao_storage_read(ao_log_current_pos,
							    &log_data[0],
							    sizeof (log_data[0]))) {
						for (slot = 0; slot < sizeof (log_data[0]); slot++)
							if (((uint8_t *) &log_data)[slot] != 0xff)
								break;
						if (slot == sizeof (log_data[0]))
							break;
					}
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
	{ ao_log_list,	"l\0List stored flight logs" },
	{ ao_log_delete,	"d <flight-number>\0Delete stored flight" },
	{ 0,	NULL },
};

void ao_log_flight_init(void);

void
ao_log_init(void)
{
	ao_log_running = 0;

	ao_cmd_register(&ao_log_cmds[0]);

	/* these probably should be in a thread */
	ao_storage_setup();
	ao_log_scan();

	/* ...and this starts a thread, so... */
	ao_log_flight_init();
}
