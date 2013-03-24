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

__xdata struct ao_fifo	ao_serial1_rx_fifo;
__xdata struct ao_fifo	ao_serial1_tx_fifo;

void
ao_debug_out(char c)
{
	if (c == '\n')
		ao_debug_out('\r');
	loop_until_bit_is_set(UCSR1A, UDRE1);
	UDR1 = c;
}

ISR(USART1_RX_vect)
{
	if (!ao_fifo_full(ao_serial1_rx_fifo))
		ao_fifo_insert(ao_serial1_rx_fifo, UDR1);
	ao_wakeup(&ao_serial1_rx_fifo);
#if USE_SERIAL_1_STDIN
	ao_wakeup(&ao_stdin_ready);
#endif
}

static __xdata uint8_t ao_serial_tx1_started;

static void
ao_serial1_tx_start(void)
{
	if (!ao_fifo_empty(ao_serial1_tx_fifo) &&
	    !ao_serial_tx1_started)
	{
		ao_serial_tx1_started = 1;
		ao_fifo_remove(ao_serial1_tx_fifo, UDR1);
	}
}

ISR(USART1_UDRE_vect)
{
	ao_serial1_tx_started = 0;
	ao_serial1_tx_start();
	ao_wakeup(&ao_serial1_tx_fifo);
}

#if USE_SERIAL_1_STDIN
int
_ao_serial1_pollchar(void)
{
	char	c;
	if (ao_fifo_empty(ao_serial1_rx_fifo)) {
		sei();
		return AO_READ_AGAIN;
	}
	ao_fifo_remove(ao_serial1_rx_fifo,c);
	return c;
}
#endif

char
ao_serial1_getchar(void) __critical
{
	char	c;

	ao_arch_block_interrupts();
	while (ao_fifo_empty(ao_serial1_rx_fifo))
		ao_sleep(&ao_serial1_rx_fifo);
	ao_fifo_remove(ao_serial1_rx_fifo, c);
	ao_arch_release_interrupts();
	return c;
}

void
ao_serial1_putchar(char c)
{
	ao_arch_block_interrupts();
	while (ao_fifo_full(ao_serial1_tx_fifo))
		ao_sleep(&ao_serial1_tx_fifo);
	ao_fifo_insert(ao_serial1_tx_fifo, c);
	ao_serial_tx1_start();
	ao_arch_release_interrupts();
}

void
ao_serial1_drain(void) __critical
{
	ao_arch_block_interrupts();
	while (!ao_fifo_empty(ao_serial1_tx_fifo))
		ao_sleep(&ao_serial1_tx_fifo);
	ao_arch_release_interrupts();
}

static const struct {
	uint16_t ubrr;
} ao_serial_speeds[] = {
	/* [AO_SERIAL_SPEED_4800] = */ {
		F_CPU / (16UL * 4800UL) - 1
	},
	/* [AO_SERIAL_SPEED_9600] = */ {
		F_CPU / (16UL * 9600UL) - 1
	},
	/* [AO_SERIAL_SPEED_19200] = */ {
		F_CPU / (16UL * 19200UL) - 1
	},
	/* [AO_SERIAL_SPEED_57600] = */ {
		F_CPU / (16UL * 57600UL) - 1
	},
};

void
ao_serial1_set_speed(uint8_t speed)
{
	ao_serial_drain();
	if (speed > AO_SERIAL_SPEED_57600)
		return;
	UBRR1L = ao_serial_speeds[speed].ubrr;
	UBRR1H = ao_serial_speeds[speed].ubrr >> 8;
}

void
ao_serial_init(void)
{
	/* Ensure the uart is powered up */

	PRR1 &= ~(1 << PRUSART1);

	/* Pick a 9600 baud rate */
	ao_serial_set_speed(AO_SERIAL_SPEED_9600);

	UCSR1A = 0;
	UCSR1C = ((0 << UMSEL10) |	/* Asynchronous mode */
		  (0 << UPM10) |	/* No parity */
		  (0 << USBS1) |	/* 1 stop bit */
		  (3 << UCSZ10) |	/* 8 bit characters */
		  (0 << UCPOL1));	/* MBZ for async mode */
	UCSR1B = ((1 << RXEN1) |	/* Enable receiver */
		  (1 << TXEN1) |	/* Enable transmitter */
		  (1 << RXCIE1) |	/* Enable receive interrupts */
		  (1 << UDRIE1));	/* Enable transmit empty interrupts */
#if USE_SERIAL_1_STDIN
	ao_add_stdio(_ao_serial1_pollchar,
		     ao_serial1_putchar,
		     NULL);
#endif
}
