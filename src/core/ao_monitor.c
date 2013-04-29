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
#include "ao_flight.h"

#if !HAS_MONITOR
#error Must define HAS_MONITOR to 1
#endif

#ifndef LEGACY_MONITOR
#error Must define LEGACY_MONITOR
#endif

#ifndef HAS_MONITOR_PUT
#define HAS_MONITOR_PUT 1
#endif

#ifndef AO_MONITOR_LED
#error Must define AO_MONITOR_LED
#endif

__data uint8_t ao_monitoring;
static __data uint8_t ao_monitor_disabled;
static __data uint8_t ao_internal_monitoring;
static __data uint8_t ao_external_monitoring;

__xdata union ao_monitor ao_monitor_ring[AO_MONITOR_RING];

__data uint8_t	ao_monitor_head;

static void
_ao_monitor_adjust(void)
{
	if (ao_monitoring)
		ao_radio_recv_abort();
	if (ao_monitor_disabled)
		ao_monitoring = 0;
	else {
		if (ao_external_monitoring)
			ao_monitoring = ao_external_monitoring;
		else
			ao_monitoring = ao_internal_monitoring;
	}
	ao_wakeup(DATA_TO_XDATA(&ao_monitoring));
}

void
ao_monitor_get(void)
{
	uint8_t	size;

	for (;;) {
		switch (ao_monitoring) {
		case 0:
			ao_sleep(DATA_TO_XDATA(&ao_monitoring));
			continue;
#if LEGACY_MONITOR
		case AO_MONITORING_ORIG:
			size = sizeof (struct ao_telemetry_orig_recv);
			break;
#endif
		default:
			if (ao_monitoring > AO_MAX_TELEMETRY)
				ao_monitoring = AO_MAX_TELEMETRY;
			size = ao_monitoring;
			break;
		}
		if (!ao_radio_recv(&ao_monitor_ring[ao_monitor_head], size + 2, 0))
			continue;
		ao_monitor_head = ao_monitor_ring_next(ao_monitor_head);
		ao_wakeup(DATA_TO_XDATA(&ao_monitor_head));
	}
}

#if AO_MONITOR_LED
__xdata struct ao_task ao_monitor_blink_task;

void
ao_monitor_blink(void)
{
	for (;;) {
		ao_sleep(DATA_TO_XDATA(&ao_monitor_head));
		ao_led_for(AO_MONITOR_LED, AO_MS_TO_TICKS(100));
	}
}
#endif

#if HAS_MONITOR_PUT

static const char xdigit[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

#define hex(c) do { putchar(xdigit[(c) >> 4]); putchar(xdigit[(c)&0xf]); } while (0)

void
ao_monitor_put(void)
{
#if LEGACY_MONITOR
	__xdata char callsign[AO_MAX_CALLSIGN+1];
	int16_t rssi;
#endif
	uint8_t ao_monitor_tail;
	uint8_t state;
	uint8_t sum, byte;
	__xdata union ao_monitor	*m;

#define recv_raw	((m->raw))
#define recv_orig	((m->orig))
#define recv_tiny	((m->tiny))

	ao_monitor_tail = ao_monitor_head;
	for (;;) {
		while (!ao_external_monitoring)
			ao_sleep(DATA_TO_XDATA(&ao_external_monitoring));
		while (ao_monitor_tail == ao_monitor_head && ao_external_monitoring)
			ao_sleep(DATA_TO_XDATA(&ao_monitor_head));
		if (!ao_external_monitoring)
			continue;
		m = &ao_monitor_ring[ao_monitor_tail];
		ao_monitor_tail = ao_monitor_ring_next(ao_monitor_tail);
		switch (ao_monitoring) {
		case 0:
			break;
#if LEGACY_MONITOR
		case AO_MONITORING_ORIG:
			state = recv_orig.telemetry_orig.flight_state;

			rssi = (int16_t) AO_RSSI_FROM_RADIO(recv_orig.rssi);
			ao_xmemcpy(callsign, recv_orig.telemetry_orig.callsign, AO_MAX_CALLSIGN);
			if (state > ao_flight_invalid)
				state = ao_flight_invalid;
			if (recv_orig.status & PKT_APPEND_STATUS_1_CRC_OK) {

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
				       recv_orig.telemetry_orig.serial,
				       recv_orig.telemetry_orig.flight,
				       rssi,
				       ao_state_names[state],
				       recv_orig.telemetry_orig.adc.tick);

				/* Raw sensor values */
				printf(AO_TELEM_RAW_ACCEL " %d "
				       AO_TELEM_RAW_BARO " %d "
				       AO_TELEM_RAW_THERMO " %d "
				       AO_TELEM_RAW_BATT " %d "
				       AO_TELEM_RAW_DROGUE " %d "
				       AO_TELEM_RAW_MAIN " %d ",
				       recv_orig.telemetry_orig.adc.accel,
				       recv_orig.telemetry_orig.adc.pres,
				       recv_orig.telemetry_orig.adc.temp,
				       recv_orig.telemetry_orig.adc.v_batt,
				       recv_orig.telemetry_orig.adc.sense_d,
				       recv_orig.telemetry_orig.adc.sense_m);

				/* Sensor calibration values */
				printf(AO_TELEM_CAL_ACCEL_GROUND " %d "
				       AO_TELEM_CAL_BARO_GROUND " %d "
				       AO_TELEM_CAL_ACCEL_PLUS " %d "
				       AO_TELEM_CAL_ACCEL_MINUS " %d ",
				       recv_orig.telemetry_orig.ground_accel,
				       recv_orig.telemetry_orig.ground_pres,
				       recv_orig.telemetry_orig.accel_plus_g,
				       recv_orig.telemetry_orig.accel_minus_g);

				if (recv_orig.telemetry_orig.u.k.unused == 0x8000) {
					/* Kalman state values */
					printf(AO_TELEM_KALMAN_HEIGHT " %d "
					       AO_TELEM_KALMAN_SPEED " %d "
					       AO_TELEM_KALMAN_ACCEL " %d ",
					       recv_orig.telemetry_orig.height,
					       recv_orig.telemetry_orig.u.k.speed,
					       recv_orig.telemetry_orig.accel);
				} else {
					/* Ad-hoc flight values */
					printf(AO_TELEM_ADHOC_ACCEL " %d "
					       AO_TELEM_ADHOC_SPEED " %ld "
					       AO_TELEM_ADHOC_BARO " %d ",
					       recv_orig.telemetry_orig.accel,
					       recv_orig.telemetry_orig.u.flight_vel,
					       recv_orig.telemetry_orig.height);
				}
				ao_gps_print(&recv_orig.telemetry_orig.gps);
				ao_gps_tracking_print(&recv_orig.telemetry_orig.gps_tracking);
				putchar('\n');
#if HAS_RSSI
				ao_rssi_set(rssi);
#endif
			} else {
				printf("CRC INVALID RSSI %3d\n", rssi);
			}
			break;
#endif /* LEGACY_MONITOR */
		default:
#if AO_PROFILE
		{
			extern uint32_t	ao_rx_start_tick, ao_rx_packet_tick, ao_rx_done_tick, ao_rx_last_done_tick;
			extern uint32_t ao_fec_decode_start, ao_fec_decode_end;

			printf ("between packet: %d\n", ao_rx_start_tick - ao_rx_last_done_tick);
			printf ("receive start delay: %d\n", ao_rx_packet_tick - ao_rx_start_tick);
			printf ("decode time: %d\n", ao_fec_decode_end - ao_fec_decode_start);
			printf ("rx cleanup: %d\n", ao_rx_done_tick - ao_fec_decode_end);
		}
#endif
			printf("TELEM ");
			hex((uint8_t) (ao_monitoring + 2));
			sum = 0x5a;
			for (state = 0; state < ao_monitoring + 2; state++) {
				byte = recv_raw.packet[state];
				sum += byte;
				hex(byte);
			}
			hex(sum);
			putchar ('\n');
#if HAS_RSSI
			if (recv_raw.packet[ao_monitoring + 1] & PKT_APPEND_STATUS_1_CRC_OK) {
				rssi = AO_RSSI_FROM_RADIO(recv_raw.packet[ao_monitoring]);
				ao_rssi_set(rssi);
			}
#endif
			break;
		}
		ao_usb_flush();
	}
}

__xdata struct ao_task ao_monitor_put_task;
#endif

__xdata struct ao_task ao_monitor_get_task;

void
ao_monitor_set(uint8_t monitoring)
{
	ao_internal_monitoring = monitoring;
	_ao_monitor_adjust();
}

void
ao_monitor_disable(void)
{
	++ao_monitor_disabled;
	_ao_monitor_adjust();
}

void
ao_monitor_enable(void)
{
	--ao_monitor_disabled;
	_ao_monitor_adjust();
}

#if HAS_MONITOR_PUT
static void
set_monitor(void)
{
	ao_cmd_hex();
	ao_external_monitoring = ao_cmd_lex_i;
	ao_wakeup(DATA_TO_XDATA(&ao_external_monitoring));
	ao_wakeup(DATA_TO_XDATA(&ao_monitor_head));
	_ao_monitor_adjust();
}

__code struct ao_cmds ao_monitor_cmds[] = {
	{ set_monitor,	"m <0 off, 1 old, 20 std>\0Set radio monitoring" },
	{ 0,	NULL },
};
#endif

void
ao_monitor_init(void) __reentrant
{
#if HAS_MONITOR_PUT
	ao_cmd_register(&ao_monitor_cmds[0]);
	ao_add_task(&ao_monitor_put_task, ao_monitor_put, "monitor_put");
#endif
	ao_add_task(&ao_monitor_get_task, ao_monitor_get, "monitor_get");
#if AO_MONITOR_LED
	ao_add_task(&ao_monitor_blink_task, ao_monitor_blink, "monitor_blink");
#endif
}
