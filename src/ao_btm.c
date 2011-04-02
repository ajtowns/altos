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

uint8_t			ao_btm_running;
uint8_t			ao_btm_stdio;
__xdata uint8_t		ao_btm_connected;
uint8_t			ao_btm_chat;

__xdata char		ao_btm_buffer[1024];
int			ao_btm_ptr;

#define AO_BTM_MAX_REPLY	16
__xdata char		ao_btm_reply[AO_BTM_MAX_REPLY];

extern volatile __xdata struct ao_fifo	ao_usart1_rx_fifo;

/*
 * Read a line of data from the serial port, truncating
 * it after a few characters.
 */

uint8_t
ao_btm_get_line(void)
{
	uint8_t ao_btm_reply_len = 0;
	char c;

	for (;;) {

		while ((c = ao_serial_pollchar()) != AO_READ_AGAIN) {
			if (ao_btm_reply_len < sizeof (ao_btm_reply))
				ao_btm_reply[ao_btm_reply_len++] = c;
			if (ao_btm_ptr < sizeof (ao_btm_buffer))
				ao_btm_buffer[ao_btm_ptr++] = c;
			if (c == '\r' || c == '\n')
				goto done;
		}
		for (c = 0; c < 10; c++) {
			ao_delay(AO_MS_TO_TICKS(10));
			if (!ao_fifo_empty(ao_usart1_rx_fifo))
				break;
		}
		if (c == 10)
			goto done;
	}
done:
	for (c = ao_btm_reply_len; c < sizeof (ao_btm_reply);)
		ao_btm_reply[c++] = '\0';
	return ao_btm_reply_len;
}

/*
 * Drain the serial port completely
 */
void
ao_btm_drain()
{
	while (ao_btm_get_line())
		;
}

void
ao_btm_echo(uint8_t echo)
{
	ao_stdios[ao_btm_stdio].echo = echo;
}

/*
 * A command line pre-processor to detect connect/disconnect messages
 * and update the internal state
 */

uint8_t
ao_cmd_filter(void)
{
	ao_cmd_lex();
	while (ao_cmd_lex_c != '\n') {
		if (ao_match_word("CONNECT")) {
			ao_btm_connected = 1;
			ao_btm_echo(1);
			ao_wakeup(&ao_btm_connected);
			return 1;
		}
		if (ao_match_word("DISCONNECT")) {
			ao_btm_connected = 0;
			ao_btm_echo(0);
			ao_wakeup(&ao_btm_connected);
			return 1;
		}
		if (ao_match_word("ERROR"))
			return 1;
		if (ao_match_word("OK"))
			return 1;
		ao_cmd_lex();
	}
	ao_cmd_status = 0;
	return !ao_btm_connected;
}

/*
 * A wrapper for ao_serial_pollchar that
 * doesn't return any characters while we're
 * initializing the bluetooth device
 */
char
ao_btm_pollchar(void)
{
	char	c;
	if (!ao_btm_running)
		return AO_READ_AGAIN;
	c = ao_serial_pollchar();
	if (c != AO_READ_AGAIN)
		if (ao_btm_ptr < sizeof (ao_btm_buffer))
			ao_btm_buffer[ao_btm_ptr++] = c;
	return c;
}

/*
 * Wait for the bluetooth device to return
 * status from the previously executed command
 */
uint8_t
ao_btm_wait_reply(void)
{
	for (;;) {
		ao_btm_get_line();
		if (!strcmp(ao_btm_reply, "OK"))
			return 1;
		if (!strcmp(ao_btm_reply, "ERROR"))
			return -1;
		if (ao_btm_reply[0] == '\0')
			return 0;
	}
}

void
ao_btm_cmd(__code char *cmd)
{
	ao_cur_stdio = ao_btm_stdio;
	printf(cmd);
	ao_btm_wait_reply();
}

/*
 * A thread to initialize the bluetooth device and
 * hang around to blink the LED when connected
 */
void
ao_btm(void)
{
	ao_serial_set_speed(AO_SERIAL_SPEED_19200);
	ao_add_stdio(ao_btm_pollchar,
		     ao_serial_putchar,
		     NULL);
	ao_btm_stdio = ao_num_stdios - 1;
	ao_cur_stdio = ao_btm_stdio;
	ao_btm_echo(0);
	ao_btm_drain();
	ao_delay(AO_SEC_TO_TICKS(1));
	printf("+++");
	ao_btm_drain();
	ao_delay(AO_SEC_TO_TICKS(1));
	printf("\r");
	ao_btm_drain();
	ao_btm_cmd("ATQ0\r");
	ao_btm_cmd("ATE0\r");
	ao_btm_cmd("ATH\r");
	ao_delay(AO_SEC_TO_TICKS(1));
	ao_btm_cmd("ATC0\r");
	ao_btm_cmd("ATL4\r");
	ao_serial_set_speed(AO_SERIAL_SPEED_57600);
	ao_btm_drain();
	printf("ATN=TeleBT-%d\r", ao_serial_number);
	ao_btm_wait_reply();
	ao_btm_running = 1;
	for (;;) {
		while (!ao_btm_connected && !ao_btm_chat)
			ao_sleep(&ao_btm_connected);
		if (ao_btm_chat) {
			ao_btm_running = 0;
			while (ao_btm_chat) {
				char	c;
				c = ao_serial_pollchar();
				if (c != AO_READ_AGAIN)
					ao_usb_putchar(c);
				else {
					ao_usb_flush();
					ao_sleep(&ao_usart1_rx_fifo);
				}
			}
			ao_btm_running = 1;
		}
		while (ao_btm_connected) {
			ao_led_for(AO_LED_GREEN, AO_MS_TO_TICKS(20));
			ao_delay(AO_SEC_TO_TICKS(3));
		}
	}
}

__xdata struct ao_task ao_btm_task;

/*
 * Connect directly to the bluetooth device, mostly
 * useful for testing
 */
static void
ao_btm_forward(void)
{
	char c;

	ao_btm_chat = 1;
	ao_wakeup(&ao_btm_connected);
	ao_usb_flush();
	while ((c = ao_usb_getchar()) != '~') {
		if (c == '\n') c = '\r';
		ao_serial_putchar(c);
	}
	ao_btm_chat = 0;
	while (!ao_btm_running) {
		ao_wakeup(&ao_usart1_rx_fifo);
		ao_delay(AO_MS_TO_TICKS(10));
	}
}

/*
 * Dump everything received from the bluetooth device during startup
 */
static void
ao_btm_dump(void)
{
	int i;

	for (i = 0; i < ao_btm_ptr; i++)
		putchar(ao_btm_buffer[i]);
	putchar('\n');
}

__code struct ao_cmds ao_btm_cmds[] = {
	{ ao_btm_forward,	"B\0BTM serial link." },
	{ ao_btm_dump,		"d\0Dump btm buffer." },
	{ 0, NULL },
};

void
ao_btm_init (void)
{
	ao_serial_init();
	ao_serial_set_speed(AO_SERIAL_SPEED_19200);
	ao_add_task(&ao_btm_task, ao_btm, "bt");
	ao_cmd_register(&ao_btm_cmds[0]);
}