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

int8_t			ao_btm_stdio;
__xdata uint8_t		ao_btm_connected;

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

#if HAS_BEEP
	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
#endif

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

#if BT_LINK_ON_P2
#define BT_PICTL_ICON	PICTL_P2ICON
#define BT_PIFG		P2IFG
#define BT_PDIR		P2DIR
#define BT_PINP		P2INP
#define BT_IEN2_PIE	IEN2_P2IE
#endif
#if BT_LINK_ON_P1
#define BT_PICTL_ICON	PICTL_P1ICON
#define BT_PIFG		P1IFG
#define BT_PDIR		P1DIR
#define BT_PINP		P1INP
#define BT_IEN2_PIE	IEN2_P1IE
#endif

void
ao_btm_check_link() __critical
{
	/* Check the pin and configure the interrupt detector to wait for the
	 * pin to flip the other way
	 */
	if (BT_LINK_PIN) {
		ao_btm_connected = 0;
		PICTL |= BT_PICTL_ICON;
	} else {
		ao_btm_connected = 1;
		PICTL &= ~BT_PICTL_ICON;
	}
}

void
ao_btm_isr(void)
#if BT_LINK_ON_P1
	__interrupt 15
#endif
{
#if BT_LINK_ON_P1
	P1IF = 0;
#endif
	if (BT_PIFG & (1 << BT_LINK_PIN_INDEX)) {
		ao_btm_check_link();
		ao_wakeup(&ao_btm_connected);
	}
	BT_PIFG = 0;
}

void
ao_btm_init (void)
{
	ao_serial_init();
	ao_serial_set_speed(AO_SERIAL_SPEED_19200);

#if BT_LINK_ON_P1
	/*
	 * Configure ser reset line
	 */

	P1_6 = 0;
	P1DIR |= (1 << 6);
#endif

	/*
	 * Configure link status line
	 */

	/* Set pin to input */
	BT_PDIR &= ~(1 << BT_LINK_PIN_INDEX);

	/* Set pin to tri-state */
	BT_PINP |= (1 << BT_LINK_PIN_INDEX);

	/* Enable interrupts */
	IEN2 |= BT_IEN2_PIE;

	/* Check current pin state */
	ao_btm_check_link();

#if BT_LINK_ON_P2
	/* Eable the pin interrupt */
	PICTL |= PICTL_P2IEN;
#endif
#if BT_LINK_ON_P1
	/* Enable pin interrupt */
	P1IEN |= (1 << BT_LINK_PIN_INDEX);
#endif

	ao_add_task(&ao_btm_task, ao_btm, "bt");
}
