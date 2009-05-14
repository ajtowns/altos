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

__xdata uint8_t ao_monitoring;
__pdata uint8_t ao_monitor_led;

void
ao_monitor(void)
{
	__xdata struct ao_radio_recv recv;
	__xdata char callsign[AO_MAX_CALLSIGN+1];
	uint8_t state;

	for (;;) {
		__critical while (!ao_monitoring)
			ao_sleep(&ao_monitoring);
		ao_radio_recv(&recv);
		state = recv.telemetry.flight_state;
		memcpy(callsign, recv.telemetry.callsign, AO_MAX_CALLSIGN);
		if (state > ao_flight_invalid)
			state = ao_flight_invalid;
		if (recv.status & PKT_APPEND_STATUS_1_CRC_OK) {
			printf ("CALL %s SERIAL %3d RSSI %3d STATUS %02x STATE %7s ",
				callsign,
				recv.telemetry.addr,
				(int) recv.rssi - 74, recv.status,
				ao_state_names[state]);
			printf("%5u a: %5d p: %5d t: %5d v: %5d d: %5d m: %5d ",
			       recv.telemetry.adc.tick,
			       recv.telemetry.adc.accel,
			       recv.telemetry.adc.pres,
			       recv.telemetry.adc.temp,
			       recv.telemetry.adc.v_batt,
			       recv.telemetry.adc.sense_d,
			       recv.telemetry.adc.sense_m);
			ao_gps_print(&recv.telemetry.gps);
			ao_rssi_set((int) recv.rssi - 74);
		} else {
			printf("CRC INVALID RSSI %3d\n", (int) recv.rssi - 74);
		}
		ao_usb_flush();
		ao_led_toggle(ao_monitor_led);
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
	{ 'm',	ao_set_monitor,		"m <0 off, 1 on>                    Enable/disable radio monitoring" },
	{ 0,	ao_set_monitor,	NULL },
};

void
ao_monitor_init(uint8_t monitor_led)
{
	ao_monitor_led = monitor_led;
	ao_monitoring = 0;
	ao_cmd_register(&ao_monitor_cmds[0]);
	ao_add_task(&ao_monitor_task, ao_monitor, "monitor");
}
