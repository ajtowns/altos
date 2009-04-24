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

const char const * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "coast",
	"apogee", "drogue", "main", "landed", "invalid"
};

__xdata uint8_t ao_monitoring;

void
ao_monitor(void)
{
	__xdata struct ao_radio_recv recv;
	uint8_t state;

	for (;;) {
		__critical while (!ao_monitoring)
			ao_sleep(&ao_monitoring);
		ao_radio_recv(&recv);
		state = recv.telemetry.flight_state;
		if (state > ao_flight_invalid)
			state = ao_flight_invalid;
		printf ("SERIAL %3d RSSI %3d STATUS %02x STATE %s ",
			recv.telemetry.addr, recv.rssi, recv.status,
			ao_state_names[state]);
		if (!(recv.status & PKT_APPEND_STATUS_1_CRC_OK))
			printf("CRC INVALID ");
		printf("%5u a: %d p: %d t: %d v: %d d: %d m: %d ",
		       recv.telemetry.adc.tick,
		       recv.telemetry.adc.accel,
		       recv.telemetry.adc.pres,
		       recv.telemetry.adc.temp,
		       recv.telemetry.adc.v_batt,
		       recv.telemetry.adc.sense_d,
		       recv.telemetry.adc.sense_m);
		ao_gps_print(&recv.telemetry.gps);
		ao_usb_flush();
		ao_led_for(AO_LED_GREEN, AO_MS_TO_TICKS(10));
	}
}

__xdata struct ao_task ao_monitor_task;

static void
ao_set_monitor(void)
{
	ao_cmd_hex();
	ao_monitoring = ao_cmd_lex_i != 0;
	ao_wakeup(&ao_monitoring);
}

__code struct ao_cmds ao_monitor_cmds[] = {
	{ 'M',	ao_set_monitor,	"M                                  Enable/disable radio monitoring" },
	{ 0,	ao_set_monitor,	NULL },
};

void
ao_monitor_init(void)
{
	ao_monitoring = 0;
	ao_cmd_register(&ao_monitor_cmds[0]);
	ao_add_task(&ao_monitor_task, ao_monitor, "monitor");
}
