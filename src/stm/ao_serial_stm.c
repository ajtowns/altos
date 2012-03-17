/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

__xdata struct ao_fifo	ao_usart1_rx_fifo;
__xdata struct ao_fifo	ao_usart1_tx_fifo;

void
ao_debug_out(char c)
{
	if (c == '\n')
		ao_debug_out('\r');
	while (!(STM_USART1->usart_sr & (1 << STM_USART_SR_TXE)));
	STM_USART1->usart_dr = c;
}

#if 0
static __xdata uint8_t ao_serial_tx1_started;

static void
ao_serial_tx1_start(void)
{
	if (!ao_fifo_empty(ao_usart1_tx_fifo) &&
	    !ao_serial_tx1_started)
	{
		ao_serial_tx1_started = 1;
		ao_fifo_remove(ao_usart1_tx_fifo, STM_USART1->usart_dr);
	}
}

void usart1_isr(void)
{
	if (STM_USART1->usart_sr & (1 << STM_USART_SR_RXNE)) {
		if (!ao_fifo_full(ao_usart1_rx_fifo))
			ao_fifo_insert(ao_usart1_rx_fifo, STM_USART1->usart_dr);
		ao_wakeup(&ao_usart1_rx_fifo);
#if USE_SERIAL_STDIN
		ao_wakeup(&ao_stdin_ready);
#endif
	}
	if (STM_USART1->usart_sr & (1 << STM_USART_SR_TXE)) {
		ao_serial_tx1_started = 0;
		ao_serial_tx1_start();
		ao_wakeup(&ao_usart1_tx_fifo);
	}
}

char
ao_serial_getchar(void) __critical
{
	char	c;
	cli();
	while (ao_fifo_empty(ao_usart1_rx_fifo))
		ao_sleep(&ao_usart1_rx_fifo);
	ao_fifo_remove(ao_usart1_rx_fifo, c);
	sei();
	return c;
}

#if USE_SERIAL_STDIN
char
ao_serial_pollchar(void) __critical
{
	char	c;
	cli();
	if (ao_fifo_empty(ao_usart1_rx_fifo)) {
		sei();
		return AO_READ_AGAIN;
	}
	ao_fifo_remove(ao_usart1_rx_fifo,c);
	sei();
	return c;
}
#endif

void
ao_serial_putchar(char c) __critical
{
	cli();
	while (ao_fifo_full(ao_usart1_tx_fifo))
		ao_sleep(&ao_usart1_tx_fifo);
	ao_fifo_insert(ao_usart1_tx_fifo, c);
	ao_serial_tx1_start();
	sei();
}

void
ao_serial_drain(void) __critical
{
	cli();
	while (!ao_fifo_empty(ao_usart1_tx_fifo))
		ao_sleep(&ao_usart1_tx_fifo);
	sei();
}

#endif

int _write(int file, char *ptr, int len)
{
	int l = len;
	while (l--)
		ao_debug_out(*ptr++);
	return len;
}

#define F_CPU	24000000

static const struct {
	uint32_t usart_brr;
} ao_serial_speeds[] = {
	[AO_SERIAL_SPEED_4800] = {
		(F_CPU * 16) / (16 * 4800) 
	},
	[AO_SERIAL_SPEED_9600] = {
		(F_CPU * 16) / (16 * 9600) 
	},
	[AO_SERIAL_SPEED_19200] = {
		(F_CPU * 16) / (16 * 19200) 
	},
	[AO_SERIAL_SPEED_57600] = {
		(F_CPU * 16) / (16 * 57600) 
	},
};

void
ao_serial_set_speed(uint8_t speed)
{
#if 0
	ao_serial_drain();
#endif
	if (speed > AO_SERIAL_SPEED_57600)
		return;
	STM_USART1->usart_brr = ao_serial_speeds[speed].usart_brr;
}

void
ao_serial_init(void)
{
	STM_USART1->usart_cr1 = ((0 << STM_USART_CR1_OVER8) |
			     (1 << STM_USART_CR1_UE) |
			     (0 << STM_USART_CR1_M) |
			     (0 << STM_USART_CR1_WAKE) |
			     (0 << STM_USART_CR1_PCE) |
			     (0 << STM_USART_CR1_PS) |
			     (0 << STM_USART_CR1_PEIE) |
			     (0 << STM_USART_CR1_TXEIE) |	/* XXX enable */
			     (0 << STM_USART_CR1_TCIE) |
			     (0 << STM_USART_CR1_RXNEIE) |		/* XXX enable */
			     (0 << STM_USART_CR1_IDLEIE) |
			     (1 << STM_USART_CR1_TE) |
			     (1 << STM_USART_CR1_RE) |
			     (0 << STM_USART_CR1_RWU) |
			     (0 << STM_USART_CR1_SBK));

	STM_USART1->usart_cr2 = 0;
	STM_USART1->usart_cr3 = 0;

	/* Pick a 9600 baud rate */
	ao_serial_set_speed(AO_SERIAL_SPEED_9600);

	printf ("serial initialized\n");
#if 0
#if USE_SERIAL_STDIN
	ao_add_stdio(ao_serial_pollchar,
		     ao_serial_putchar,
		     NULL);
#endif
#endif
}
