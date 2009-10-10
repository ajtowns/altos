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

volatile __xdata struct ao_fifo	ao_usart1_rx_fifo;
volatile __xdata struct ao_fifo	ao_usart1_tx_fifo;

void
ao_serial_rx1_isr(void) interrupt 3
{
	if (!ao_fifo_full(ao_usart1_rx_fifo))
		ao_fifo_insert(ao_usart1_rx_fifo, U1DBUF);
	ao_wakeup(&ao_usart1_rx_fifo);
}

static __xdata uint8_t ao_serial_tx1_started;

static void
ao_serial_tx1_start(void)
{
	if (!ao_fifo_empty(ao_usart1_tx_fifo) &&
	    !ao_serial_tx1_started)
	{
		ao_serial_tx1_started = 1;
		ao_fifo_remove(ao_usart1_tx_fifo, U1DBUF);
	}
}

void
ao_serial_tx1_isr(void) interrupt 14
{
	UTX1IF = 0;
	ao_serial_tx1_started = 0;
	ao_serial_tx1_start();
	ao_wakeup(&ao_usart1_tx_fifo);
}

static __pdata serial_echo;

char
ao_serial_getchar(void) __critical
{
	char	c;
	while (ao_fifo_empty(ao_usart1_rx_fifo))
		ao_sleep(&ao_usart1_rx_fifo);
	ao_fifo_remove(ao_usart1_rx_fifo, c);
	if (serial_echo) {
		printf("%02x ", ((int) c) & 0xff);
		if (c >= ' ')
			putchar(c);
		putchar('\n');
		flush();
	}
	return c;
}

void
ao_serial_putchar(char c) __critical
{
	while (ao_fifo_full(ao_usart1_tx_fifo))
		ao_sleep(&ao_usart1_tx_fifo);
	ao_fifo_insert(ao_usart1_tx_fifo, c);
	ao_serial_tx1_start();
}

static void
ao_serial_drain(void) __critical
{
	while (!ao_fifo_empty(ao_usart1_tx_fifo))
		ao_sleep(&ao_usart1_tx_fifo);
}

static void
send_serial(void)
{
	ao_cmd_white();
	while (ao_cmd_lex_c != '\n') {
		ao_serial_putchar(ao_cmd_lex_c);
		ao_cmd_lex();
	}
}

static void
monitor_serial(void)
{
	ao_cmd_hex();
	serial_echo = ao_cmd_lex_i != 0;
}

static void
serial_baud(void)
{
	ao_cmd_hex();
	ao_serial_set_speed(ao_cmd_lex_i);
}

__code struct ao_cmds ao_serial_cmds[] = {
	{ 'S', send_serial,		"S <data>                           Send data to serial line" },
	{ 'M', monitor_serial,		"M <enable>                         Monitor serial data" },
	{ 'B', serial_baud,		"B <0 = 4800, 1 = 57600>            Set serial baud rate" },
	{ 0, send_serial, NULL },
};

static const struct {
	uint8_t	baud;
	uint8_t	gcr;
} ao_serial_speeds[] = {
	/* [AO_SERIAL_SPEED_4800] = */ {
		/* .baud = */ 163,
		/* .gcr  = */ (7 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_9600] = */ {
		/* .baud = */ 163,
		/* .gcr  = */ (8 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_57600] = */ {
		/* .baud = */ 59,
		/* .gcr =  */ (11 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
};

void
ao_serial_set_speed(uint8_t speed)
{
	ao_serial_drain();
	if (speed > AO_SERIAL_SPEED_57600)
		return;
	U1BAUD = ao_serial_speeds[speed].baud;
	U1GCR = ao_serial_speeds[speed].gcr;
}

void
ao_serial_init(void)
{
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U1CFG_ALT_MASK) | PERCFG_U1CFG_ALT_2;

	/* ee has already set the P2SEL bits */

	/* Make the USART pins be controlled by the USART */
	P1SEL |= (1 << 6) | (1 << 7);

	/* UART mode with receiver enabled */
	U1CSR = (UxCSR_MODE_UART | UxCSR_RE);

	/* Pick a 4800 baud rate */
	ao_serial_set_speed(AO_SERIAL_SPEED_4800);

	/* Reasonable serial parameters */
	U1UCR = (UxUCR_FLUSH |
		 UxUCR_FLOW_DISABLE |
		 UxUCR_D9_ODD_PARITY |
		 UxUCR_BIT9_8_BITS |
		 UxUCR_PARITY_DISABLE |
		 UxUCR_SPB_1_STOP_BIT |
		 UxUCR_STOP_HIGH |
		 UxUCR_START_LOW);

	IEN0 |= IEN0_URX1IE;
	IEN2 |= IEN2_UTX1IE;

	ao_cmd_register(&ao_serial_cmds[0]);
}
