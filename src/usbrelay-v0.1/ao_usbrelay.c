/*
 * Copyright © 2014 Bdale Garbee <bdale@gag.com>
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

uint8_t		relay_output;

void
ao_relay_init(void)
{
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_GPIO);
        lpc_gpio.dir[RELAY_PORT] |= RELAY_BIT;
}

// switch relay to selected output, turn correct LED on as a side effect
static void
ao_relay_control(uint8_t output)
{
	switch (output) {
	case 1:
		lpc_gpio.pin[RELAY_PORT] |= RELAY_BIT;
		ao_led_on(AO_LED_RED);
		ao_led_off(AO_LED_GREEN);
		break;
	default:
		lpc_gpio.pin[RELAY_PORT] &= ~RELAY_BIT;
		ao_led_off(AO_LED_RED);
		ao_led_on(AO_LED_GREEN);
	}
}

static void
ao_relay_select(void) __reentrant
{
	uint8_t output;

	ao_cmd_decimal();
        if (ao_cmd_status != ao_cmd_success)
                return;
	output = ao_cmd_lex_i;
	if (output > 1) 
		printf ("Invalid relay position %u\n", output);
	else
		ao_relay_control(output);
}

static __code struct ao_cmds ao_relay_cmds[] = {
	{ ao_relay_select, "R <output>\0Select relay output" },
	{ 0, NULL }
};

void
main(void)
{
	ao_clock_init();
	ao_task_init();
	ao_timer_init();

	ao_usb_init();

	ao_serial_init();

	ao_led_init(LEDS_AVAILABLE);

	ao_relay_init();

	// initialize to default output
	relay_output = 0;
	ao_relay_control(relay_output);

	ao_cmd_init();

	ao_cmd_register(ao_relay_cmds);

	ao_start_scheduler();
}
