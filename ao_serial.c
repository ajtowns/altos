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

#define SERIAL_FIFO	32

struct ao_fifo {
	uint8_t	insert;
	uint8_t	remove;
	uint8_t	fifo[SERIAL_FIFO];
};

volatile __xdata struct ao_fifo	ao_usart1_rx_fifo;
volatile __xdata struct ao_fifo	ao_usart1_tx_fifo;

#define ao_fifo_insert(f,c) do { \
	(f).fifo[(f).insert] = (c); \
	(f).insert = ((f).insert + 1) & (SERIAL_FIFO-1); \
} while(0)

#define ao_fifo_remove(f,c) do {\
	c = (f).fifo[(f).remove]; \
	(f).remove = ((f).remove + 1) & (SERIAL_FIFO-1); \
} while(0)

#define ao_fifo_full(f)	((((f).insert + 1) & (SERIAL_FIFO-1)) == (f).remove)
#define ao_fifo_empty(f)	((f).insert == (f).remove)

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

uint8_t
ao_serial_getchar(void) __critical
{
	uint8_t	c;
	while (ao_fifo_empty(ao_usart1_rx_fifo))
		ao_sleep(&ao_usart1_rx_fifo);
	ao_fifo_remove(ao_usart1_rx_fifo, c);
	return c;
}

void
ao_serial_putchar(uint8_t c) __critical
{
	while (ao_fifo_full(ao_usart1_tx_fifo))
		ao_sleep(&ao_usart1_tx_fifo);
	ao_fifo_insert(ao_usart1_tx_fifo, c);
	ao_serial_tx1_start();
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
	U1BAUD = 163;				/* 4800 */
	U1GCR = 7 << UxGCR_BAUD_E_SHIFT;	/* 4800 */

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
}
