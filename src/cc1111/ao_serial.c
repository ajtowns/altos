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

const __code struct ao_serial_speed ao_serial_speeds[] = {
	/* [AO_SERIAL_SPEED_4800] = */ {
		/* .baud = */ 163,
		/* .gcr  = */ (7 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_9600] = */ {
		/* .baud = */ 163,
		/* .gcr  = */ (8 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_19200] = */ {
		/* .baud = */ 163,
		/* .gcr  = */ (9 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_57600] = */ {
		/* .baud = */ 59,
		/* .gcr =  */ (11 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
	/* [AO_SERIAL_SPEED_115200] = */ {
		/* .baud = */ 59,
		/* .gcr =  */ (12 << UxGCR_BAUD_E_SHIFT) | UxGCR_ORDER_LSB
	},
};

#define AO_SERIAL_SPEED_MAX	AO_SERIAL_SPEED_115200

#if HAS_SERIAL_0

volatile __xdata struct ao_fifo	ao_serial0_rx_fifo;
volatile __xdata struct ao_fifo	ao_serial0_tx_fifo;

void
ao_serial0_rx_isr(void) __interrupt 2
{
	if (!ao_fifo_full(ao_serial0_rx_fifo))
		ao_fifo_insert(ao_serial0_rx_fifo, U0DBUF);
	ao_wakeup(&ao_serial0_rx_fifo);
#if USE_SERIAL_0_STDIN
	ao_wakeup(&ao_stdin_ready);
#endif
}

static __xdata uint8_t ao_serial0_tx_started;

static void
ao_serial0_tx_start(void)
{
	if (!ao_fifo_empty(ao_serial0_tx_fifo) &&
	    !ao_serial0_tx_started)
	{
		ao_serial0_tx_started = 1;
		ao_fifo_remove(ao_serial0_tx_fifo, U0DBUF);
	}
}

void
ao_serial0_tx_isr(void) __interrupt 7
{
	UTX0IF = 0;
	ao_serial0_tx_started = 0;
	ao_serial0_tx_start();
	ao_wakeup(&ao_serial0_tx_fifo);
}

char
ao_serial0_getchar(void) __critical
{
	char	c;
	while (ao_fifo_empty(ao_serial0_rx_fifo))
		ao_sleep(&ao_serial0_rx_fifo);
	ao_fifo_remove(ao_serial0_rx_fifo, c);
	return c;
}

#if USE_SERIAL_0_STDIN
int
_ao_serial0_pollchar(void)
{
	uint8_t	c;
	if (ao_fifo_empty(ao_serial0_rx_fifo))
		return AO_READ_AGAIN;
	ao_fifo_remove(ao_serial0_rx_fifo,c);
	return c;
}
#endif

void
ao_serial0_putchar(char c) __critical
{
	while (ao_fifo_full(ao_serial0_tx_fifo))
		ao_sleep(&ao_serial0_tx_fifo);
	ao_fifo_insert(ao_serial0_tx_fifo, c);
	ao_serial0_tx_start();
}

void
ao_serial0_drain(void) __critical
{
	while (!ao_fifo_empty(ao_serial0_tx_fifo))
		ao_sleep(&ao_serial0_tx_fifo);
}

void
ao_serial0_set_speed(uint8_t speed)
{
	ao_serial0_drain();
	if (speed > AO_SERIAL_SPEED_MAX)
		return;
	U0UCR |= UxUCR_FLUSH;
	U0BAUD = ao_serial_speeds[speed].baud;
	U0GCR = ao_serial_speeds[speed].gcr;
}
#endif /* HAS_SERIAL_0 */

#if HAS_SERIAL_1

volatile __xdata struct ao_fifo	ao_serial1_rx_fifo;
volatile __xdata struct ao_fifo	ao_serial1_tx_fifo;

void
ao_serial1_rx_isr(void) __interrupt 3
{
	if (!ao_fifo_full(ao_serial1_rx_fifo))
		ao_fifo_insert(ao_serial1_rx_fifo, U1DBUF);
	ao_wakeup(&ao_serial1_rx_fifo);
#if USE_SERIAL_1_STDIN
	ao_wakeup(&ao_stdin_ready);
#endif
}

static __xdata uint8_t ao_serial1_tx_started;

static void
ao_serial1_tx_start(void)
{
	if (!ao_fifo_empty(ao_serial1_tx_fifo) &&
	    !ao_serial1_tx_started)
	{
		ao_serial1_tx_started = 1;
		ao_fifo_remove(ao_serial1_tx_fifo, U1DBUF);
	}
}

void
ao_serial1_tx_isr(void) __interrupt 14
{
	UTX1IF = 0;
	ao_serial1_tx_started = 0;
	ao_serial1_tx_start();
	ao_wakeup(&ao_serial1_tx_fifo);
}

char
ao_serial1_getchar(void) __critical
{
	char	c;
	while (ao_fifo_empty(ao_serial1_rx_fifo))
		ao_sleep(&ao_serial1_rx_fifo);
	ao_fifo_remove(ao_serial1_rx_fifo, c);
	return c;
}

#if USE_SERIAL_1_STDIN
int
_ao_serial1_pollchar(void)
{
	uint8_t	c;
	if (ao_fifo_empty(ao_serial1_rx_fifo))
		return AO_READ_AGAIN;
	ao_fifo_remove(ao_serial1_rx_fifo,c);
	return c;
}
#endif

void
ao_serial1_putchar(char c) __critical
{
	while (ao_fifo_full(ao_serial1_tx_fifo))
		ao_sleep(&ao_serial1_tx_fifo);
	ao_fifo_insert(ao_serial1_tx_fifo, c);
	ao_serial1_tx_start();
}

void
ao_serial1_drain(void) __critical
{
	while (!ao_fifo_empty(ao_serial1_tx_fifo))
		ao_sleep(&ao_serial1_tx_fifo);
}

void
ao_serial1_set_speed(uint8_t speed)
{
	ao_serial1_drain();
	if (speed > AO_SERIAL_SPEED_MAX)
		return;
	U1UCR |= UxUCR_FLUSH;
	U1BAUD = ao_serial_speeds[speed].baud;
	U1GCR = ao_serial_speeds[speed].gcr;
}

#endif /* HAS_SERIAL_1 */

void
ao_serial_init(void)
{
#if HAS_SERIAL_0
#if HAS_SERIAL_0_ALT_1
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U0CFG_ALT_MASK) | PERCFG_U0CFG_ALT_1;

	P2DIR = (P2DIR & ~P2DIR_PRIP0_MASK) | P2DIR_PRIP0_USART0_USART1;

	/* Make the USART pins be controlled by the USART */
	P0SEL |= (1 << 2) | (1 << 3);
#if HAS_SERIAL_0_HW_FLOW
	P0SEL |= (1 << 4) | (1 << 5);
#endif
#else
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U0CFG_ALT_MASK) | PERCFG_U0CFG_ALT_2;

	P2SEL = (P2SEL & ~(P2SEL_PRI3P1_MASK | P2SEL_PRI0P1_MASK)) |
		(P2SEL_PRI3P1_USART0 | P2SEL_PRI0P1_USART0);

	/* Make the USART pins be controlled by the USART */
	P1SEL |= (1 << 5) | (1 << 4);
#if HAS_SERIAL_0_HW_FLOW
	P1SEL |= (1 << 3) | (1 << 2);
#endif
#endif

	/* UART mode with receiver enabled */
	U0CSR = (UxCSR_MODE_UART | UxCSR_RE);

	/* Pick a 9600 baud rate */
	ao_serial0_set_speed(AO_SERIAL_SPEED_9600);

	/* Reasonable serial parameters */
	U0UCR = (UxUCR_FLUSH |
#if HAS_SERIAL_0_HW_FLOW
		 UxUCR_FLOW_ENABLE |
#else
		 UxUCR_FLOW_DISABLE |
#endif
		 UxUCR_D9_EVEN_PARITY |
		 UxUCR_BIT9_8_BITS |
		 UxUCR_PARITY_DISABLE |
		 UxUCR_SPB_1_STOP_BIT |
		 UxUCR_STOP_HIGH |
		 UxUCR_START_LOW);

	IEN0 |= IEN0_URX0IE;
	IEN2 |= IEN2_UTX0IE;
#if USE_SERIAL_0_STDIN && !DELAY_SERIAL_0_STDIN
	ao_add_stdio(_ao_serial0_pollchar,
		     ao_serial0_putchar,
		     NULL);
#endif
#endif /* HAS_SERIAL_0 */

#if HAS_SERIAL_1
#if HAS_SERIAL_1_ALT_1
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U1CFG_ALT_MASK) | PERCFG_U1CFG_ALT_1;

	P2DIR = (P2DIR & ~P2DIR_PRIP0_MASK) | P2DIR_PRIP0_USART1_USART0;

	/* Make the USART pins be controlled by the USART */
	P0SEL |= (1 << 5) | (1 << 4);
#if HAS_SERIAL_1_HW_FLOW
	P0SEL |= (1 << 3) | (1 << 2);
#endif
#else
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U1CFG_ALT_MASK) | PERCFG_U1CFG_ALT_2;

	P2SEL = (P2SEL & ~(P2SEL_PRI3P1_MASK | P2SEL_PRI2P1_MASK)) |
		(P2SEL_PRI3P1_USART1 | P2SEL_PRI2P1_USART1);

	/* Make the USART pins be controlled by the USART */
	P1SEL |= (1 << 6) | (1 << 7);
#if HAS_SERIAL_1_HW_FLOW
	P1SEL |= (1 << 5) | (1 << 4);
#endif
#endif

	/* UART mode with receiver enabled */
	U1CSR = (UxCSR_MODE_UART | UxCSR_RE);

	/* Pick a 4800 baud rate */
	ao_serial1_set_speed(AO_SERIAL_SPEED_4800);

	/* Reasonable serial parameters */
	U1UCR = (UxUCR_FLUSH |
#if HAS_SERIAL_1_HW_FLOW
		 UxUCR_FLOW_ENABLE |
#else
		 UxUCR_FLOW_DISABLE |
#endif
		 UxUCR_D9_EVEN_PARITY |
		 UxUCR_BIT9_8_BITS |
		 UxUCR_PARITY_DISABLE |
		 UxUCR_SPB_1_STOP_BIT |
		 UxUCR_STOP_HIGH |
		 UxUCR_START_LOW);

	IEN0 |= IEN0_URX1IE;
	IEN2 |= IEN2_UTX1IE;

#if USE_SERIAL_1_STDIN && !DELAY_SERIAL_1_STDIN
	ao_add_stdio(_ao_serial1_pollchar,
		     ao_serial1_putchar,
		     NULL);
#endif
#endif /* HAS_SERIAL_1 */
}
