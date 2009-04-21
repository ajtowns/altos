/*
 * $Id: $
 *
 * Copyright © 2009 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "ao.h"

const char const * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "coast",
	"apogee", "drogue", "main", "landed", "invalid"
};

void
ao_monitor(void)
{
	__xdata struct ao_radio_recv recv;
	uint8_t state;

	for (;;) {
		ao_radio_recv(&recv);
		state = recv.telemetry.flight_state;
		if (state > ao_flight_invalid)
			state = ao_flight_invalid;
		printf ("SERIAL %3d RSSI %3d STATUS %02x STATE %s ",
			recv.telemetry.addr, recv.rssi, recv.status,
			ao_state_names[state]);
		if (!(recv.status & PKT_APPEND_STATUS_1_CRC_OK))
			printf("CRC INVALID ");
		switch (recv.telemetry.type) {
		case AO_TELEMETRY_SENSOR:
			printf("%5u a: %d p: %d t: %d v: %d d: %d m: %d\n",
			       recv.telemetry.u.adc.tick,
			       recv.telemetry.u.adc.accel,
			       recv.telemetry.u.adc.pres,
			       recv.telemetry.u.adc.temp,
			       recv.telemetry.u.adc.v_batt,
			       recv.telemetry.u.adc.sense_d,
			       recv.telemetry.u.adc.sense_m);
			break;
		case AO_TELEMETRY_GPS:
			ao_gps_print(&recv.telemetry.u.gps);
			break;
		}
		ao_usb_flush();
	}
}

__xdata struct ao_task ao_monitor_task;

void
ao_monitor_init(void)
{
	ao_add_task(&ao_monitor_task, ao_monitor, "monitor");
}
