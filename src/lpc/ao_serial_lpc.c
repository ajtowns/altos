/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
#include <ao_serial.h>

struct ao_fifo	ao_usart_rx_fifo;
struct ao_fifo	ao_usart_tx_fifo;
uint8_t		ao_usart_tx_avail;
uint8_t		ao_usart_tx_avail_min;

#define LPC_USART_TX_FIFO_SIZE	16

void
ao_debug_out(char c)
{
	if (c == '\n')
		ao_debug_out('\r');
	while (!(lpc_usart.lsr & (1 << LPC_USART_LSR_TEMT)))
		;
	lpc_usart.rbr_thr = c;
}

static void
_ao_serial_tx_start(void)
{
	if (!ao_fifo_empty(ao_usart_tx_fifo) && ao_usart_tx_avail) {
		ao_usart_tx_avail--;
		if (ao_usart_tx_avail < ao_usart_tx_avail_min)
			ao_usart_tx_avail_min = ao_usart_tx_avail;
		ao_fifo_remove(ao_usart_tx_fifo, lpc_usart.rbr_thr);
	}
}

void
lpc_usart_isr(void)
{
	uint8_t wake_input = 0;
	(void) lpc_usart.iir_fcr;

	while (lpc_usart.lsr & (1 << LPC_USART_LSR_RDR)) {
		char c = lpc_usart.rbr_thr;
		if (!ao_fifo_full(ao_usart_rx_fifo))
			ao_fifo_insert(ao_usart_rx_fifo, c);
		wake_input = 1;
	}
	if (lpc_usart.lsr & (1 << LPC_USART_LSR_THRE)) {
		ao_usart_tx_avail = LPC_USART_TX_FIFO_SIZE;
		_ao_serial_tx_start();
		ao_wakeup(&ao_usart_tx_fifo);
	}
	if (wake_input) {
		ao_wakeup(&ao_usart_rx_fifo);
		if (stdin)
			ao_wakeup(&ao_stdin_ready);
	}
}

int
_ao_serial0_pollchar(void)
{
	int	c;
	
	if (ao_fifo_empty(ao_usart_rx_fifo))
		c = AO_READ_AGAIN;
	else {
		uint8_t	u;
		ao_fifo_remove(ao_usart_rx_fifo,u);
		c = u;
	}
	return c;
}

char
ao_serial0_getchar(void)
{
	int c;
	ao_arch_block_interrupts();
	while ((c = _ao_serial0_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_usart_rx_fifo);
	ao_arch_release_interrupts();
	return (char) c;
}

void
ao_serial0_putchar(char c)
{
	ao_arch_block_interrupts();
	while (ao_fifo_full(ao_usart_tx_fifo))
		ao_sleep(&ao_usart_tx_fifo);
	ao_fifo_insert(ao_usart_tx_fifo, c);
	_ao_serial_tx_start();
	ao_arch_release_interrupts();
}

void
ao_serial0_drain(void)
{
	ao_arch_block_interrupts();
	while (!ao_fifo_empty(ao_usart_tx_fifo))
		ao_sleep(&ao_usart_tx_fifo);
	ao_arch_release_interrupts();
}

#include "ao_serial_lpc.h"

void
ao_serial0_set_speed(uint8_t speed)
{
	if (speed > AO_SERIAL_SPEED_115200)
		return;

	/* Flip to allow access to divisor latches */
	lpc_usart.lcr |= (1 << LPC_USART_LCR_DLAB);

	/* DL LSB */
	lpc_usart.rbr_thr = ao_usart_speeds[speed].dl & 0xff;
	
	/* DL MSB */
	lpc_usart.ier = (ao_usart_speeds[speed].dl >> 8) & 0xff;

	lpc_usart.fdr = ((ao_usart_speeds[speed].divaddval << LPC_USART_FDR_DIVADDVAL) |
			 (ao_usart_speeds[speed].mulval << LPC_USART_FDR_MULVAL));

	/* Turn access to divisor latches back off */
	lpc_usart.lcr &= ~(1 << LPC_USART_LCR_DLAB);
}

void
ao_serial_init(void)
{
#if SERIAL_0_18_19
	lpc_ioconf.pio0_18 = ((LPC_IOCONF_FUNC_PIO0_18_RXD << LPC_IOCONF_FUNC) |
			      (LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) |
			      (0 << LPC_IOCONF_HYS) |
			      (0 << LPC_IOCONF_INV) |
			      (0 << LPC_IOCONF_OD));
	lpc_ioconf.pio0_19 = ((LPC_IOCONF_FUNC_PIO0_19_TXD << LPC_IOCONF_FUNC) |
			      (LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) |
			      (0 << LPC_IOCONF_HYS) |
			      (0 << LPC_IOCONF_INV) |
			      (0 << LPC_IOCONF_OD));
#endif

	/* Turn on the USART */
	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_USART);

	/* Turn on the USART clock */
	lpc_scb.uartclkdiv = AO_LPC_CLKOUT / AO_LPC_USARTCLK;

	/* Configure USART */

	/* Enable FIFOs, reset fifo contents, interrupt on 1 received char */
	lpc_usart.iir_fcr = ((1 << LPC_USART_FCR_FIFOEN) |
			 (1 << LPC_USART_FCR_RXFIFORES) |
			 (1 << LPC_USART_FCR_TXFIFORES) |
			 (LPC_USART_FCR_RXTL_1 << LPC_USART_FCR_RXTL));

	ao_usart_tx_avail = LPC_USART_TX_FIFO_SIZE;
	ao_usart_tx_avail_min = LPC_USART_TX_FIFO_SIZE;

	/* 8 n 1 */
	lpc_usart.lcr = ((LPC_USART_LCR_WLS_8 << LPC_USART_LCR_WLS) |
			 (LPC_USART_LCR_SBS_1 << LPC_USART_LCR_SBS) |
			 (0 << LPC_USART_LCR_PE) |
			 (LPC_USART_LCR_PS_ODD << LPC_USART_LCR_PS) |
			 (0 << LPC_USART_LCR_BC) |
			 (0 << LPC_USART_LCR_DLAB));

	/* Disable flow control */
	lpc_usart.mcr = ((0 << LPC_USART_MCR_DTRCTRL) |
			 (0 << LPC_USART_MCR_RTSCTRL) |
			 (0 << LPC_USART_MCR_LMS) |
			 (0 << LPC_USART_MCR_RTSEN) |
			 (0 << LPC_USART_MCR_CTSEN));

	/* 16x oversampling */
	lpc_usart.osr = ((0 << LPC_USART_OSR_OSFRAC) |
			 ((16 - 1) << LPC_USART_OSR_OSINT) |
			 (0 << LPC_USART_OSR_FDINT));

	/* Full duplex */
	lpc_usart.hden = ((0 << LPC_USART_HDEN_HDEN));

	/* Set baud rate */
	ao_serial0_set_speed(AO_SERIAL_SPEED_9600);

	/* Enable interrupts */
	lpc_usart.ier = ((1 << LPC_USART_IER_RBRINTEN) |
			 (1 << LPC_USART_IER_THREINTEN));

	lpc_nvic_set_enable(LPC_ISR_USART_POS);
	lpc_nvic_set_priority(LPC_ISR_USART_POS, 0);
#if USE_SERIAL_0_STDIN
	ao_add_stdio(_ao_serial0_pollchar,
		     ao_serial0_putchar,
		     NULL);
#endif
}
