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
#include <ao_exti.h>
#include <ao_packet.h>
#include <ao_companion.h>
#include <ao_profile.h>
#include <ao_pyro.h>
#include <ao_aes.h>
#include <ao_seven_segment.h>
#include <ao_quadrature.h>
#include <ao_button.h>
#include <ao_lco.h>
#include <ao_lco_cmd.h>
#include <ao_radio_cmac_cmd.h>

int
main(void)
{
	ao_clock_init();
	
	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_GREEN);
	ao_task_init();

	ao_timer_init();

	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_beep_init();
	ao_cmd_init();

	ao_lcd_stm_init();
	ao_seven_segment_init();
	ao_quadrature_init();
	ao_button_init();

	ao_storage_init();
	
	ao_radio_init();

	ao_usb_init();

	ao_config_init();
	
	ao_quadrature_init();
	ao_button_init();
	ao_lco_init();
	ao_lco_cmd_init();
//	ao_radio_cmac_cmd_init();
	
	ao_start_scheduler();
	return 0;
}
