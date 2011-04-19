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
int8_t			ao_btm_stdio;
__xdata uint8_t		ao_btm_connected;

void
ao_btm_putchar(char c);

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

/*
 * Set the stdio echo for the bluetooth link
 */
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
	if (ao_cur_stdio != ao_btm_stdio)
		return 0;
	ao_cmd_lex();
	while (ao_cmd_lex_c != '\n') {
		if (ao_match_word("CONNECT"))
			return 1;
		if (ao_match_word("DISCONNECT"))
			return 1;
		if (ao_match_word("ERROR"))
			return 1;
		if (ao_match_word("OK"))
			return 1;
		ao_cmd_lex();
	}
	ao_cmd_status = 0;
	return 0;
}

/*
 * Delay between command charaters; the BT module
 * can't keep up with 57600 baud
 */

void
ao_btm_putchar(char c)
{
	ao_serial_putchar(c);
	ao_delay(1);
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
		if (!strncmp(ao_btm_reply, "OK", 2))
			return 1;
		if (!strncmp(ao_btm_reply, "ERROR", 5))
			return -1;
		if (ao_btm_reply[0] == '\0')
			return 0;
	}
}

void
ao_btm_string(__code char *cmd)
{
	char	c;

	while (c = *cmd++)
		ao_btm_putchar(c);
}

uint8_t
ao_btm_cmd(__code char *cmd)
{
	ao_btm_drain();
	ao_btm_string(cmd);
	return ao_btm_wait_reply();
}

uint8_t
ao_btm_set_name(void)
{
	char	sn[8];
	char	*s = sn + 8;
	char	c;
	int	n;
	ao_btm_string("ATN=TeleBT-");
	*--s = '\0';
	*--s = '\r';
	n = ao_serial_number;
	do {
		*--s = '0' + n % 10;
	} while (n /= 10);
	while ((c = *s++))
		ao_btm_putchar(c);
	return ao_btm_wait_reply();
}

uint8_t
ao_btm_try_speed(uint8_t speed)
{
	ao_serial_set_speed(speed);
	ao_btm_drain();
	(void) ao_btm_cmd("\rATE0\rATQ0\r");
	if (ao_btm_cmd("AT\r") == 1)
		return 1;
	return 0;
}

/*
 * A thread to initialize the bluetooth device and
 * hang around to blink the LED when connected
 */
void
ao_btm(void)
{
	/*
	 * Wait for the bluetooth device to boot
	 */
	ao_delay(AO_SEC_TO_TICKS(3));

	/*
	 * The first time we connect, the BTM-180 comes up at 19200 baud.
	 * After that, it will remember and come up at 57600 baud. So, see
	 * if it is already running at 57600 baud, and if that doesn't work
	 * then tell it to switch to 57600 from 19200 baud.
	 */
	while (!ao_btm_try_speed(AO_SERIAL_SPEED_57600)) {
		ao_delay(AO_SEC_TO_TICKS(1));
		if (ao_btm_try_speed(AO_SERIAL_SPEED_19200))
			ao_btm_cmd("ATL4\r");
		ao_delay(AO_SEC_TO_TICKS(1));
	}

	/* Disable echo */
	ao_btm_cmd("ATE0\r");

	/* Enable flow control */
	ao_btm_cmd("ATC1\r");

	/* Set the reported name to something we can find on the host */
	ao_btm_set_name();

	/* Turn off status reporting */
	ao_btm_cmd("ATQ1\r");

	ao_btm_stdio = ao_add_stdio(ao_serial_pollchar,
				    ao_serial_putchar,
				    NULL);
	ao_btm_echo(0);

	ao_btm_running = 1;
	for (;;) {
		while (!ao_btm_connected)
			ao_sleep(&ao_btm_connected);
		while (ao_btm_connected) {
			ao_led_for(AO_LED_GREEN, AO_MS_TO_TICKS(20));
			ao_delay(AO_SEC_TO_TICKS(3));
		}
	}
}

__xdata struct ao_task ao_btm_task;

void
ao_btm_check_link() __critical
{
	if (P2_1) {
		ao_btm_connected = 0;
		PICTL |= PICTL_P2ICON;
	} else {
		ao_btm_connected = 1;
		PICTL &= ~PICTL_P2ICON;
	}
}

void
ao_btm_isr(void)
{
	if (P2IFG & (1 << 1)) {
		ao_btm_check_link();
		ao_wakeup(&ao_btm_connected);
	}
	P2IFG = 0;
}

void
ao_btm_init (void)
{
	ao_serial_init();
	ao_serial_set_speed(AO_SERIAL_SPEED_19200);

	/*
	 * Configure link status line
	 */

	/* Set P2_1 to input, pull-down */
	P2DIR &= ~(1 << 1);
	P2INP |= P2INP_MDP2_1_TRISTATE;

	/* Enable P2 interrupts */
	IEN2 |= IEN2_P2IE;
	ao_btm_check_link();
	PICTL |= PICTL_P2IEN;

	ao_add_task(&ao_btm_task, ao_btm, "bt");
}
