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

static void (*display_packet)(__xdata void *pkt, int16_t rssi, uint8_t status)
__reentrant;
static uint16_t packet_size;

void
ao_monitor(void)
{
	__xdata uint8_t recv[2 + sizeof (struct ao_telemetry)];
	int16_t rssi;
	uint8_t status;

	for (;;) {
		__critical while (!ao_monitoring)
			ao_sleep(&ao_monitoring);
		if (!ao_radio_recv(&recv, packet_size + 2))
			continue;

		/* Typical RSSI offset for 38.4kBaud at 433 MHz is 74 */
		rssi = (int16_t) (((int8_t)recv[packet_size]) >> 1) - 74;
		status = recv[packet_size+1];

		if (status & PKT_APPEND_STATUS_1_CRC_OK) {
			display_packet(recv, rssi, status);
			ao_rssi_set(rssi);
		} else {
			printf("CRC INVALID. RSSI %3d\n", rssi);
		}
		ao_usb_flush();
		ao_led_toggle(ao_monitor_led);
	}
}

static char
tohex(int i)
{
	if (0 <= i && i <= 9)
		return '0' + i;
	else if (10 <= i && i <= 15)
		return 'a' + i - 10;
	else
		return '?';
}

static void
display_hex(__xdata uint8_t *pkt, int16_t rssi, uint8_t status) __reentrant
{
	int i;
	for (i = 0; i < packet_size; i++) {
		putchar( tohex( (*pkt & 0xF0) >> 4 ) );
		putchar( tohex( (*pkt & 0x0F) ) );
		if (i % 4 == 3)
			putchar(' ');
		pkt++;
	}
	if (i % 4 != 0)
		putchar(' ');
	printf("RSSI: %d STATUS: %02x\n", rssi, status);
}

static void
display_telem(__xdata struct ao_telemetry *telemetry,
	      int16_t rssi, uint8_t status) __reentrant
{
	static __xdata char callsign[AO_MAX_CALLSIGN+1];
	uint8_t state = telemetry->flight_state;

	memcpy(callsign, telemetry->callsign, AO_MAX_CALLSIGN);
	callsign[AO_MAX_CALLSIGN] = '\0';

	if (state > ao_flight_invalid)
		state = ao_flight_invalid;

	printf("VERSION %d CALL %s SERIAL %3d FLIGHT %5u RSSI %4d STATUS %02x STATE %7s ",
	AO_TELEMETRY_VERSION,
	callsign,
	telemetry->serial,
	telemetry->flight,
	rssi, status,
	ao_state_names[state]);
	printf("%5u a: %5d p: %5d t: %5d v: %5d d: %5d m: %5d "
	"fa: %5d ga: %d fv: %7ld fp: %5d gp: %5d a+: %5d a-: %5d ",
	telemetry->adc.tick,
	telemetry->adc.accel,
	telemetry->adc.pres,
	telemetry->adc.temp,
	telemetry->adc.v_batt,
	telemetry->adc.sense_d,
	telemetry->adc.sense_m,
	telemetry->flight_accel,
	telemetry->ground_accel,
	telemetry->flight_vel,
	telemetry->flight_pres,
	telemetry->ground_pres,
	telemetry->accel_plus_g,
	telemetry->accel_minus_g);
	ao_gps_print(&telemetry->gps);
	putchar(' ');
	ao_gps_tracking_print(&telemetry->gps_tracking);
	putchar('\n');
}

__xdata struct ao_task ao_monitor_task;

void
ao_set_monitor(uint8_t monitoring)
{
	ao_monitoring = monitoring;
	if (monitoring)
		ao_radio_set_fixed_pkt(packet_size);
	ao_wakeup(&ao_monitoring);
	if (!ao_monitoring)
		ao_radio_recv_abort();
}

static void
set_monitor(void)
{
	ao_set_monitor(0);
	ao_cmd_white();
	switch (ao_cmd_lex_c) {
	case '0':
		break;
	case '1':
	case 'T':
		display_packet = display_telem;
		packet_size = sizeof(struct ao_telemetry);
		ao_set_monitor(1);
		break;
	case 'H':
		ao_cmd_lex();
		ao_cmd_decimal();
		if (ao_cmd_status != ao_cmd_success)
			break;

		if (ao_cmd_lex_i <= 0 || sizeof(struct ao_telemetry) < ao_cmd_lex_i) {
			printf("Packet size must be between 1 and %d\n",
			       sizeof(struct ao_telemetry));
			return;
		}
		display_packet = display_hex;
		packet_size = ao_cmd_lex_i;
		ao_set_monitor(1);
		break;
	default:
		ao_cmd_status = ao_cmd_syntax_error;
		break;
	}
}

__code struct ao_cmds ao_monitor_cmds[] = {
	{ 'm',	set_monitor,	"m <0 off, T|1 telem, H n hex>      Enable/disable telemetry monitoring" },
	{ 0,	set_monitor,	NULL },
};

void
ao_monitor_init(uint8_t monitor_led, uint8_t monitoring) __reentrant
{
	display_packet = display_telem;
	ao_monitor_led = monitor_led;
	ao_monitoring = monitoring;
	ao_cmd_register(&ao_monitor_cmds[0]);
	ao_add_task(&ao_monitor_task, ao_monitor, "monitor");
}
