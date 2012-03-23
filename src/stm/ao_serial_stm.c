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

struct ao_stm_usart {
	struct ao_fifo		rx_fifo;
	struct ao_fifo		tx_fifo;
	struct stm_usart	*reg;
	uint8_t			tx_started;
};

void
ao_debug_out(char c)
{
	if (c == '\n')
		ao_debug_out('\r');
	while (!(stm_usart1.sr & (1 << STM_USART_SR_TXE)));
	stm_usart1.dr = c;
}

static void
ao_usart_tx_start(struct ao_stm_usart *usart)
{
	if (!ao_fifo_empty(usart->tx_fifo) && !usart->tx_started)
	{
		usart->tx_started = 1;
		ao_fifo_remove(usart->tx_fifo, usart->reg->dr);
	}
}

static void
ao_usart_isr(struct ao_stm_usart *usart, int stdin)
{
	uint32_t	sr;

	sr = usart->reg->sr;
	usart->reg->sr = 0;

	if (sr & (1 << STM_USART_SR_RXNE)) {
		char c = usart->reg->dr;
		if (!ao_fifo_full(usart->rx_fifo))
			ao_fifo_insert(usart->rx_fifo, c);
		ao_wakeup(&usart->rx_fifo);
		if (stdin)
			ao_wakeup(&ao_stdin_ready);
	}
	if (sr & (1 << STM_USART_SR_TC)) {
		usart->tx_started = 0;
		ao_usart_tx_start(usart);
		ao_wakeup(&usart->tx_fifo);
	}
}

char
ao_usart_getchar(struct ao_stm_usart *usart)
{
	char c;
	cli();
	while (ao_fifo_empty(usart->rx_fifo))
		ao_sleep(&usart->rx_fifo);
	ao_fifo_remove(usart->rx_fifo, c);
	sei();
	return c;
}

char
ao_usart_pollchar(struct ao_stm_usart *usart)
{
	char	c;
	cli();
	if (ao_fifo_empty(usart->rx_fifo)) {
		sei();
		return AO_READ_AGAIN;
	}
	ao_fifo_remove(usart->rx_fifo,c);
	sei();
	return c;
}

void
ao_usart_putchar(struct ao_stm_usart *usart, char c)
{
	cli();
	while (ao_fifo_full(usart->tx_fifo))
		ao_sleep(&usart->tx_fifo);
	ao_fifo_insert(usart->tx_fifo, c);
	ao_usart_tx_start(usart);
	sei();
}

void
ao_usart_drain(struct ao_stm_usart *usart)
{
	cli();
	while (!ao_fifo_empty(usart->tx_fifo))
		ao_sleep(&usart->tx_fifo);
	sei();
}

static const struct {
	uint32_t brr;
} ao_usart_speeds[] = {
	[AO_SERIAL_SPEED_4800] = {
		STM_APB1 / 4800
	},
	[AO_SERIAL_SPEED_9600] = {
		STM_APB1 / 9600
	},
	[AO_SERIAL_SPEED_19200] = {
		STM_APB1 / 19200
	},
	[AO_SERIAL_SPEED_57600] = {
		STM_APB1 / 57600
	},
};

void
ao_usart_set_speed(struct ao_stm_usart *usart, uint8_t speed)
{
	if (speed > AO_SERIAL_SPEED_57600)
		return;
	stm_usart1.brr = ao_usart_speeds[speed].brr;
}

void
ao_usart_init(struct ao_stm_usart *usart)
{
	usart->reg->cr1 = ((0 << STM_USART_CR1_OVER8) |
			  (1 << STM_USART_CR1_UE) |
			  (0 << STM_USART_CR1_M) |
			  (0 << STM_USART_CR1_WAKE) |
			  (0 << STM_USART_CR1_PCE) |
			  (0 << STM_USART_CR1_PS) |
			  (0 << STM_USART_CR1_PEIE) |
			  (0 << STM_USART_CR1_TXEIE) |
			  (1 << STM_USART_CR1_TCIE) |
			  (1 << STM_USART_CR1_RXNEIE) |
			  (0 << STM_USART_CR1_IDLEIE) |
			  (1 << STM_USART_CR1_TE) |
			  (1 << STM_USART_CR1_RE) |
			  (0 << STM_USART_CR1_RWU) |
			  (0 << STM_USART_CR1_SBK));

	usart->reg->cr2 = ((0 << STM_USART_CR2_LINEN) |
			  (STM_USART_CR2_STOP_1 << STM_USART_CR2_STOP) |
			  (0 << STM_USART_CR2_CLKEN) |
			  (0 << STM_USART_CR2_CPOL) |
			  (0 << STM_USART_CR2_CPHA) |
			  (0 << STM_USART_CR2_LBCL) |
			  (0 << STM_USART_CR2_LBDIE) |
			  (0 << STM_USART_CR2_LBDL) |
			  (0 << STM_USART_CR2_ADD));

	usart->reg->cr3 = ((0 << STM_USART_CR3_ONEBITE) |
			  (0 << STM_USART_CR3_CTSIE) |
			  (0 << STM_USART_CR3_CTSE) |
			  (0 << STM_USART_CR3_RTSE) |
			  (0 << STM_USART_CR3_DMAT) |
			  (0 << STM_USART_CR3_DMAR) |
			  (0 << STM_USART_CR3_SCEN) |
			  (0 << STM_USART_CR3_NACK) |
			  (0 << STM_USART_CR3_HDSEL) |
			  (0 << STM_USART_CR3_IRLP) |
			  (0 << STM_USART_CR3_IREN) |
			  (0 << STM_USART_CR3_EIE));

	/* Pick a 9600 baud rate */
	ao_usart_set_speed(usart, AO_SERIAL_SPEED_9600);
}

#if HAS_SERIAL_1
struct ao_stm_usart ao_stm_usart1;
void stm_usart1_isr(void) { ao_usart_isr(&ao_stm_usart1, USE_SERIAL_STDIN); }
#endif
#if HAS_SERIAL_2
struct ao_stm_usart ao_stm_usart2;
void stm_usart2_isr(void) { ao_usart_isr(&ao_stm_usart2, 0); }
#endif
#if HAS_SERIAL_3
struct ao_stm_usart ao_stm_usart3;
void stm_usart3_isr(void) { ao_usart_isr(&ao_stm_usart3, 0); }
#endif

void
ao_serial_init(void)
{
#ifdef HAS_SERIAL_1
	/*
	 *	TX	RX
	 *	PA9	PA10
	 *	PB6	PB7	*
	 */

	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);

	stm_moder_set(&stm_gpiob, 6, STM_MODER_ALTERNATE);
	stm_moder_set(&stm_gpiob, 7, STM_MODER_ALTERNATE);
	stm_afr_set(&stm_gpiob, 6, STM_AFR_AF7);
	stm_afr_set(&stm_gpiob, 7, STM_AFR_AF7);
	
	/* Enable USART */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_USART1EN);
	ao_stm_usart1.reg = &stm_usart1;

	ao_usart_init(&ao_stm_usart1);

	stm_nvic_set_enable(STM_ISR_USART1_POS);
	stm_nvic_set_priority(STM_ISR_USART1_POS, 4);
#if USE_SERIAL_STDIN
	ao_add_stdio(ao_serial_pollchar,
		     ao_serial_putchar,
		     NULL);
#endif
#endif

#if HAS_SERIAL_2
	/*
	 *	TX	RX
	 *	PA2	PA3
	 *	PD5	PD6
	 */

	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);

	stm_moder_set(&stm_gpioa, 2, STM_MODER_ALTERNATE);
	stm_moder_set(&stm_gpioa, 3, STM_MODER_ALTERNATE);
	stm_afr_set(&stm_gpioa, 2, STM_AFR_AF7);
	stm_afr_set(&stm_gpioa, 3, STM_AFR_AF7);
	
	/* Enable USART */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_USART2EN);
	ao_usart_init(&stm_usart1);

	ao_stm_usart2.reg = &stm_usart2;

	stm_nvic_set_enable(STM_ISR_USART2_POS);
	stm_nvic_set_priority(STM_ISR_USART2_POS, 4);
#endif

#if HAS_SERIAL_3
	/*
	 * 	TX	RX
	 *	PB10	PB11
	 *	PC10	PC11
	 *	PD8	PD9
	 */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);

	stm_moder_set(&stm_gpiob, 10, STM_MODER_ALTERNATE);
	stm_moder_set(&stm_gpiob, 11, STM_MODER_ALTERNATE);
	stm_afr_set(&stm_gpiob, 10, STM_AFR_AF7);
	stm_afr_set(&stm_gpiob, 11, STM_AFR_AF7);
	
	/* Enable USART */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_USART3EN);
	ao_usart_init(&stm_usart1);

	ao_stm_usart3.reg = &stm_usart3;

	stm_nvic_set_enable(STM_ISR_USART3_POS);
	stm_nvic_set_priority(STM_ISR_USART3_POS, 4);
#endif
}

#if HAS_SERIAL_1
char
ao_serial_getchar(void)
{
	return ao_usart_getchar(&ao_stm_usart1);
}

#if USE_SERIAL_STDIN
char
ao_serial_pollchar(void)
{
	return ao_usart_pollchar(&ao_stm_usart1);
}
#endif

void
ao_serial_putchar(char c)
{
	ao_usart_putchar(&ao_stm_usart1, c);
}
#endif /* HAS_SERIAL_1 */


