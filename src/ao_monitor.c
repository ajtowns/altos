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
#include "ao_telem.h"

__xdata uint8_t ao_monitoring;
__pdata uint8_t ao_monitor_led;

void
ao_monitor(void)
{
	__xdata char callsign[AO_MAX_CALLSIGN+1];
	__xdata union {
		struct ao_telemetry_recv	full;
		struct ao_telemetry_tiny_recv	tiny;
	} u;

#define recv		(u.full)
#define recv_tiny	(u.tiny)

	uint8_t state;
	int16_t rssi;

	for (;;) {
		__critical while (!ao_monitoring)
			ao_sleep(&ao_monitoring);
		if (ao_monitoring == AO_MONITORING_FULL) {
			if (!ao_radio_recv(&recv, sizeof (struct ao_telemetry_recv)))
				continue;

			state = recv.telemetry.flight_state;

			/* Typical RSSI offset for 38.4kBaud at 433 MHz is 74 */
			rssi = (int16_t) (recv.rssi >> 1) - 74;
			memcpy(callsign, recv.telemetry.callsign, AO_MAX_CALLSIGN);
			if (state > ao_flight_invalid)
				state = ao_flight_invalid;
			if (recv.status & PKT_APPEND_STATUS_1_CRC_OK) {

				/* General header fields */
				printf(AO_TELEM_VERSION " %d "
				       AO_TELEM_CALL " %s "
				       AO_TELEM_SERIAL " %d "
				       AO_TELEM_FLIGHT " %d "
				       AO_TELEM_RSSI " %d "
				       AO_TELEM_STATE " %s "
				       AO_TELEM_TICK " %d ",
				       AO_TELEMETRY_VERSION,
				       callsign,
				       recv.telemetry.serial,
				       recv.telemetry.flight,
				       rssi,
				       ao_state_names[state],
				       recv.telemetry.adc.tick);

				/* Raw sensor values */
				printf(AO_TELEM_RAW_ACCEL " %d "
				       AO_TELEM_RAW_BARO " %d "
				       AO_TELEM_RAW_THERMO " %d "
				       AO_TELEM_RAW_BATT " %d "
				       AO_TELEM_RAW_DROGUE " %d "
				       AO_TELEM_RAW_MAIN " %d ",
				       recv.telemetry.adc.accel,
				       recv.telemetry.adc.pres,
				       recv.telemetry.adc.temp,
				       recv.telemetry.adc.v_batt,
				       recv.telemetry.adc.sense_d,
				       recv.telemetry.adc.sense_m);

				/* Sensor calibration values */
				printf(AO_TELEM_CAL_ACCEL_GROUND " %d "
				       AO_TELEM_CAL_BARO_GROUND " %d "
				       AO_TELEM_CAL_ACCEL_PLUS " %d "
				       AO_TELEM_CAL_ACCEL_MINUS " %d ",
				       recv.telemetry.ground_accel,
				       recv.telemetry.ground_pres,
				       recv.telemetry.accel_plus_g,
				       recv.telemetry.accel_minus_g);

				if (recv.telemetry.u.k.unused == 0x8000) {
					/* Kalman state values */
					printf(AO_TELEM_KALMAN_HEIGHT " %d "
					       AO_TELEM_KALMAN_SPEED " %d "
					       AO_TELEM_KALMAN_ACCEL " %d ",
					       recv.telemetry.height,
					       recv.telemetry.u.k.speed,
					       recv.telemetry.accel);
				} else {
					/* Ad-hoc flight values */
					printf(AO_TELEM_ADHOC_ACCEL " %d "
					       AO_TELEM_ADHOC_SPEED " %ld "
					       AO_TELEM_ADHOC_BARO " %d ",
					       recv.telemetry.accel,
					       recv.telemetry.u.flight_vel,
					       recv.telemetry.height);
				}
				ao_gps_print(&recv.telemetry.gps);
				ao_gps_tracking_print(&recv.telemetry.gps_tracking);
				putchar('\n');
				ao_rssi_set(rssi);
			} else {
				printf("CRC INVALID RSSI %3d\n", rssi);
			}
		} else {
			if (!ao_radio_recv(&recv_tiny, sizeof (struct ao_telemetry_tiny_recv)))
				continue;

			state = recv_tiny.telemetry_tiny.flight_state;

			/* Typical RSSI offset for 38.4kBaud at 433 MHz is 74 */
			rssi = (int16_t) (recv_tiny.rssi >> 1) - 74;
			memcpy(callsign, recv_tiny.telemetry_tiny.callsign, AO_MAX_CALLSIGN);
			if (state > ao_flight_invalid)
				state = ao_flight_invalid;
			if (recv_tiny.status & PKT_APPEND_STATUS_1_CRC_OK) {
				/* General header fields */
				printf(AO_TELEM_VERSION " %d "
				       AO_TELEM_CALL " %s "
				       AO_TELEM_SERIAL " %d "
				       AO_TELEM_FLIGHT " %d "
				       AO_TELEM_RSSI " %d "
				       AO_TELEM_STATE " %s "
				       AO_TELEM_TICK " %d ",
				       AO_TELEMETRY_VERSION,
				       callsign,
				       recv_tiny.telemetry_tiny.serial,
				       recv_tiny.telemetry_tiny.flight,
				       rssi,
				       ao_state_names[state],
				       recv_tiny.telemetry_tiny.adc.tick);

				/* Raw sensor values */
				printf(AO_TELEM_RAW_BARO " %d "
				       AO_TELEM_RAW_THERMO " %d "
				       AO_TELEM_RAW_BATT " %d "
				       AO_TELEM_RAW_DROGUE " %d "
				       AO_TELEM_RAW_MAIN " %d ",
				       recv_tiny.telemetry_tiny.adc.pres,
				       recv_tiny.telemetry_tiny.adc.temp,
				       recv_tiny.telemetry_tiny.adc.v_batt,
				       recv_tiny.telemetry_tiny.adc.sense_d,
				       recv_tiny.telemetry_tiny.adc.sense_m);

				/* Sensor calibration values */
				printf(AO_TELEM_CAL_BARO_GROUND " %d ",
				       recv_tiny.telemetry_tiny.ground_pres);

#if 1
				/* Kalman state values */
				printf(AO_TELEM_KALMAN_HEIGHT " %d "
				       AO_TELEM_KALMAN_SPEED " %d "
				       AO_TELEM_KALMAN_ACCEL " %d\n",
				       recv_tiny.telemetry_tiny.height,
				       recv_tiny.telemetry_tiny.speed,
				       recv_tiny.telemetry_tiny.accel);
#else
				/* Ad-hoc flight values */
				printf(AO_TELEM_ADHOC_ACCEL " %d "
				       AO_TELEM_ADHOC_SPEED " %ld "
				       AO_TELEM_ADHOC_BARO " %d\n",
				       recv_tiny.telemetry_tiny.flight_accel,
				       recv_tiny.telemetry_tiny.flight_vel,
				       recv_tiny.telemetry_tiny.flight_pres);
#endif
				ao_rssi_set(rssi);
			} else {
				printf("CRC INVALID RSSI %3d\n", rssi);
			}
		}
		ao_usb_flush();
		ao_led_toggle(ao_monitor_led);
	}
}

__xdata struct ao_task ao_monitor_task;

void
ao_set_monitor(uint8_t monitoring)
{
	if (ao_monitoring)
		ao_radio_recv_abort();
	ao_monitoring = monitoring;
	ao_wakeup(&ao_monitoring);
}

static void
set_monitor(void)
{
	ao_cmd_hex();
	ao_set_monitor(ao_cmd_lex_i);
}

__code struct ao_cmds ao_monitor_cmds[] = {
	{ set_monitor,	"m <0 off, 1 full, 2 tiny>\0Enable/disable radio monitoring" },
	{ 0,	NULL },
};

void
ao_monitor_init(uint8_t monitor_led, uint8_t monitoring) __reentrant
{
	ao_monitor_led = monitor_led;
	ao_monitoring = monitoring;
	ao_cmd_register(&ao_monitor_cmds[0]);
	ao_add_task(&ao_monitor_task, ao_monitor, "monitor");
}
