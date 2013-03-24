/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
#include <ao_pad.h>
#include <ao_74hc497.h>
#include <ao_radio_cmac_cmd.h>

void
main(void)
{
	ao_clock_init();

	ao_led_init(LEDS_AVAILABLE);

	ao_task_init();

	ao_timer_init();
	ao_adc_init();
	ao_beep_init();
	ao_cmd_init();
	ao_spi_init();
	ao_74hc497_init();
	ao_storage_init();
	ao_usb_init();
	ao_radio_init();
	ao_aes_init();
	ao_pad_init();
//	ao_radio_cmac_cmd_init();
	ao_config_init();
	ao_start_scheduler();
}
