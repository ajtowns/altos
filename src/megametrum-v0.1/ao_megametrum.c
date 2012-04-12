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

#include "ao.h"

void
beep(void)
{
	ao_beep(AO_BEEP_MID);
	printf ("Hit a character to stop..."); flush();
	getchar();
	putchar('\n');
	ao_beep(0);
}

const struct ao_cmds ao_mm_cmds[] = {
	{ beep, "b\0Beep" },
	{ 0, NULL },
};

int
main(void)
{
	ao_clock_init();
	
	ao_serial_init();
	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_GREEN);
	ao_timer_init();
	ao_cmd_init();
	ao_gps_init();
	ao_config_init();
	ao_dma_init();
	ao_spi_init();
	ao_ms5607_init();
	ao_beep_init();
	ao_adc_init();
	ao_storage_init();
	ao_usb_init();
	
	ao_cmd_register(&ao_mm_cmds[0]);
	ao_start_scheduler();
	return 0;
}
