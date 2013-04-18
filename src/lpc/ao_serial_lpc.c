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
uint8_t		ao_usart_tx_started;

void
ao_debug_out(char c)
{
	if (c == '\n')
		ao_debug_out('\r');
#if 0
	while (!(stm_usart1.sr & (1 << STM_USART_SR_TXE)));
	stm_usart1.dr = c;
#endif
}

static void
_ao_serial_tx_start(void)
{
	if (!ao_fifo_empty(ao_usart_tx_fifo) & !ao_usart_tx_started)
	{
		ao_usart_tx_started = 1;
#if 0
		ao_fifo_remove(ao_usart_tx_fifo, usart->reg->dr);
#endif
	}
}

void
lpc_usart_isr(void)
{
#if 0
	uint32_t	sr;

	sr = usart->reg->sr;
	usart->reg->sr = 0;

	if (sr & (1 << STM_USART_SR_RXNE)) {
		char c = usart->reg->dr;
		if (!ao_fifo_full(ao_usart_rx_fifo))
			ao_fifo_insert(ao_usart_rx_fifo, c);
		ao_wakeup(ao_usart_rx_fifo);
		if (stdin)
			ao_wakeup(&ao_stdin_ready);
	}
	if (sr & (1 << STM_USART_SR_TC)) {
		ao_usart_tx_started = 0;
		_ao_usart_tx_start(usart);
		ao_wakeup(ao_usart_tx_fifo);
	}
#endif
}

int
_ao_serial_pollchar(void)
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
ao_serial_getchar(void)
{
	int c;
	ao_arch_block_interrupts();
	while ((c = _ao_serial_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_usart_rx_fifo);
	ao_arch_release_interrupts();
	return (char) c;
}

void
ao_serial_putchar(char c)
{
	ao_arch_block_interrupts();
	while (ao_fifo_full(ao_usart_tx_fifo))
		ao_sleep(&ao_usart_tx_fifo);
	ao_fifo_insert(ao_usart_tx_fifo, c);
	_ao_serial_tx_start();
	ao_arch_release_interrupts();
}

void
ao_serial_drain(void)
{
	ao_arch_block_interrupts();
	while (!ao_fifo_empty(ao_usart_tx_fifo))
		ao_sleep(&ao_usart_tx_fifo);
	ao_arch_release_interrupts();
}

void
ao_serial_set_speed(uint8_t speed)
{
	if (speed > AO_SERIAL_SPEED_115200)
		return;
#if 0
	usart->reg->brr = ao_usart_speeds[speed].brr;
#endif
}

#include "ao_serial_lpc.h"

void
ao_serial_init(void)
{
	/* Turn on the USART clock */
	lpc_scb.uartclkdiv = 1;

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
}


