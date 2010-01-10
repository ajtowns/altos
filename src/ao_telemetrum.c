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

/* stub so as telemetrum doesn't have monitor mode */
void
ao_set_monitor(uint8_t monitoring)
{
	(void) monitoring;
}

void
main(void)
{
	ao_clock_init();


	/* Turn on the red LED until the system is stable */
	ao_led_init(AO_LED_RED);
	ao_led_on(AO_LED_RED);

	/* A hack -- look at the SPI clock pin, if it's sitting at
	 *  ground, then we force the computer to idle mode instead of
	 *  flight mode
	 */
	if (P1_3 == 0) {
		ao_flight_force_idle = 1;
		while (P1_3 == 0)
			;
	}
	ao_timer_init();
	ao_adc_init();
	ao_beep_init();
	ao_cmd_init();
	ao_ee_init();
	ao_flight_init();
	ao_log_init();
	ao_report_init();
	ao_usb_init();
	ao_serial_init();
	ao_gps_init();
	ao_gps_report_init();
	ao_telemetry_init();
	ao_radio_init();
	ao_packet_slave_init();
	ao_igniter_init();
	ao_config_init();
	ao_start_scheduler();
}
