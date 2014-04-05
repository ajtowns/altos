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

#include <ao.h>
#include <ao_flight.h>
#include <ao_sample.h>

__code uint8_t ao_log_format = AO_LOG_FORMAT_TELEMETRY;

static __data uint8_t			ao_log_monitor_pos;
__pdata enum ao_flight_state		ao_flight_state;
__xdata int16_t				ao_max_height;	/* max of ao_height */
__pdata int16_t				sense_d, sense_m;
__pdata uint8_t				ao_igniter_present;

static void
ao_log_telem_track() {
	if (ao_monitoring == sizeof (union ao_telemetry_all)) {
		switch (ao_log_single_write_data.telemetry.generic.type) {
		case AO_TELEMETRY_SENSOR_TELEMETRUM:
		case AO_TELEMETRY_SENSOR_TELEMINI:
			/* fall through ... */
		case AO_TELEMETRY_SENSOR_TELENANO:
			if (ao_log_single_write_data.telemetry.generic.type == AO_TELEMETRY_SENSOR_TELENANO) {
				ao_igniter_present = 0;
			} else {
				sense_d = ao_log_single_write_data.telemetry.sensor.sense_d;
				sense_m = ao_log_single_write_data.telemetry.sensor.sense_m;
				ao_igniter_present = 1;
			}
			if (ao_log_single_write_data.telemetry.sensor.height > ao_max_height) {
				ao_max_height = ao_log_single_write_data.telemetry.sensor.height;
			}
			if (ao_log_single_write_data.telemetry.sensor.state != ao_flight_state) {
				ao_flight_state = ao_log_single_write_data.telemetry.sensor.state;
				if (ao_flight_state == ao_flight_pad)
					ao_max_height = 0;
				ao_wakeup(DATA_TO_XDATA(&ao_flight_state));
			}
		}
	}
}

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter)
{
	int16_t	value;

	switch (igniter) {
	case ao_igniter_drogue:
		value = sense_d;
		break;
	case ao_igniter_main:
		value = sense_m;
		break;
	default:
		value = 0;
		break;
	}
	if (value < AO_IGNITER_OPEN)
		return ao_igniter_open;
	else if (value > AO_IGNITER_CLOSED)
		return ao_igniter_ready;
	else
		return ao_igniter_unknown;
}

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
	ao_flight_state = ao_flight_startup;
	ao_monitor_set(sizeof(struct ao_telemetry_generic));

	for (;;) {
		while (!ao_log_running)
			ao_sleep(&ao_log_running);

		ao_log_monitor_pos = ao_monitor_head;
		while (ao_log_running) {
			/* Write samples to EEPROM */
			while (ao_log_monitor_pos != ao_monitor_head) {
				ao_xmemcpy(&ao_log_single_write_data.telemetry,
					   &ao_monitor_ring[ao_log_monitor_pos],
					   AO_LOG_SINGLE_SIZE);
				ao_log_single_write();
				ao_log_monitor_pos = ao_monitor_ring_next(ao_log_monitor_pos);
				ao_log_telem_track();
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
