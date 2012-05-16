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

struct ao_i2c_stm_info {
	uint8_t	tx_dma_index;
	uint8_t	rx_dma_index;
	struct stm_i2c	*stm_i2c;
};

#define I2C_TIMEOUT	100

#define I2C_IDLE	0
#define I2C_RUNNING	1
#define I2C_ERROR	2

static uint8_t	ao_i2c_state[STM_NUM_I2C];
static uint16_t	ao_i2c_addr[STM_NUM_I2C];
uint8_t 	ao_i2c_mutex[STM_NUM_I2C];

#define AO_STM_I2C_CR1 ((0 << STM_I2C_CR1_SWRST) |	\
			(0 << STM_I2C_CR1_ALERT) |	\
			(0 << STM_I2C_CR1_PEC) |	\
			(0 << STM_I2C_CR1_POS) |	\
			(0 << STM_I2C_CR1_ACK) |	\
			(0 << STM_I2C_CR1_STOP) |	\
			(0 << STM_I2C_CR1_START) |	\
			(0 << STM_I2C_CR1_NOSTRETCH) |	\
			(0 << STM_I2C_CR1_ENGC) |	\
			(0 << STM_I2C_CR1_ENPEC) |	\
			(0 << STM_I2C_CR1_ENARP) |	\
			(0 << STM_I2C_CR1_SMBTYPE) |	\
			(0 << STM_I2C_CR1_SMBUS) |	\
			(1 << STM_I2C_CR1_PE))

#define AO_STM_I2C_CR2  ((0 << STM_I2C_CR2_LAST) |			\
			 (0 << STM_I2C_CR2_DMAEN) |			\
			 (0 << STM_I2C_CR2_ITBUFEN) |			\
			 (0 << STM_I2C_CR2_ITEVTEN) |			\
			 (0 << STM_I2C_CR2_ITERREN) |			\
			 (STM_I2C_CR2_FREQ_16_MHZ << STM_I2C_CR2_FREQ))

static const struct ao_i2c_stm_info	ao_i2c_stm_info[STM_NUM_I2C] = {
	{
		.tx_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_I2C1_TX),
		.rx_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_I2C1_RX),
		.stm_i2c = &stm_i2c1
	},
	{
		.tx_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_I2C2_TX),
		.rx_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_I2C2_RX),
		.stm_i2c = &stm_i2c2
	},
};

static uint8_t	*ao_i2c_recv_data[STM_NUM_I2C];
static uint16_t	ao_i2c_recv_len[STM_NUM_I2C];
static uint16_t	ev_count;

static void
ao_i2c_ev_isr(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint32_t	sr1;

	++ev_count;
	sr1 = stm_i2c->sr1;
	if (sr1 & (1 << STM_I2C_SR1_SB))
		stm_i2c->dr = ao_i2c_addr[index];
	if (sr1 & (1 << STM_I2C_SR1_ADDR)) {
		stm_i2c->cr2 &= ~(1 << STM_I2C_CR2_ITEVTEN);
		ao_i2c_state[index] = I2C_RUNNING;
		ao_wakeup(&ao_i2c_state[index]);
	}
	if (sr1 & (1 << STM_I2C_SR1_BTF)) {
		stm_i2c->cr2 &= ~(1 << STM_I2C_CR2_ITEVTEN);
		ao_wakeup(&ao_i2c_state[index]);
	}
	if (sr1 & (1 << STM_I2C_SR1_RXNE)) {
		if (ao_i2c_recv_len[index]) {			
			*(ao_i2c_recv_data[index]++) = stm_i2c->dr;
			if (!--ao_i2c_recv_len[index])
				ao_wakeup(&ao_i2c_recv_len[index]);
		}
	}
}

void stm_i2c1_ev_isr(void) { ao_i2c_ev_isr(0); }
void stm_i2c2_ev_isr(void) { ao_i2c_ev_isr(1); }

static void
ao_i2c_er_isr(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint32_t	sr1;

	sr1 = stm_i2c->sr1;
	if (sr1 & (1 << STM_I2C_SR1_AF)) {
		ao_i2c_state[index] = I2C_ERROR;
		stm_i2c->sr1 = sr1 & ~(1 << STM_I2C_SR1_AF);
		ao_wakeup(&ao_i2c_state[index]);
	}
}

void stm_i2c1_er_isr(void) { ao_i2c_er_isr(0); }
void stm_i2c2_er_isr(void) { ao_i2c_er_isr(1); }

void
ao_i2c_get(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	ao_mutex_get(&ao_i2c_mutex[index]);

	stm_i2c->sr1 = 0;
	stm_i2c->sr2 = 0;
}

void
ao_i2c_put(uint8_t index)
{
	ao_mutex_put(&ao_i2c_mutex[index]);
}

#define I2C_DEBUG	0
#if I2C_DEBUG
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	
#endif

static inline uint32_t in_sr1(char *where, struct stm_i2c *stm_i2c) {
	uint32_t	sr1 = stm_i2c->sr1;
	DBG("%s: sr1: %x\n", where, sr1); flush();
	return sr1;
}

static inline uint32_t in_sr2(char *where, struct stm_i2c *stm_i2c) {
	uint32_t	sr2 = stm_i2c->sr2;
	DBG("%s: sr2: %x\n", where, sr2); flush();
	return sr2;
}

static inline void out_cr1(char *where, struct stm_i2c *stm_i2c, uint32_t cr1) {
	DBG("%s: cr1: %x\n", where, cr1); flush();
	stm_i2c->cr1 = cr1;
}

static inline uint32_t in_cr1(char *where, struct stm_i2c *stm_i2c) {
	uint32_t	cr1 = stm_i2c->cr1;
	DBG("%s: cr1: %x\n", where, cr1); flush();
	return cr1;
}

static inline void out_cr2(char *where, struct stm_i2c *stm_i2c, uint32_t cr2) {
	DBG("%s: cr2: %x\n", where, cr2); flush();
	stm_i2c->cr2 = cr2;
}

static inline uint32_t in_dr(char *where, struct stm_i2c *stm_i2c) {
	uint32_t	dr = stm_i2c->dr;
	DBG("%s: dr: %x\n", where, dr); flush();
	return dr;
}

static inline void out_dr(char *where, struct stm_i2c *stm_i2c, uint32_t dr) {
	DBG("%s: dr: %x\n", where, dr); flush();
	stm_i2c->dr = dr;
}

uint8_t
ao_i2c_start(uint8_t index, uint16_t addr)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint32_t	sr1, sr2;
	int		t;

	ao_i2c_state[index] = I2C_IDLE;
	ao_i2c_addr[index] = addr;
	out_cr2("start", stm_i2c, AO_STM_I2C_CR2);
	out_cr1("start", stm_i2c,
		AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_START));
	out_cr2("start", stm_i2c,
		AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_ITEVTEN) | (1 << STM_I2C_CR2_ITERREN));
	ao_alarm(1);
	cli();
	while (ao_i2c_state[index] == I2C_IDLE)
		if (ao_sleep(&ao_i2c_state[index]))
			break;
	sei();
	ao_clear_alarm();
	return ao_i2c_state[index] == I2C_RUNNING;
}

static void
ao_i2c_wait_stop(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	int	t;

	for (t = 0; t < I2C_TIMEOUT; t++) {
		if (!(in_cr1("wait stop", stm_i2c) & (1 << STM_I2C_CR1_STOP)))
			break;
		ao_yield();
	}
	ao_i2c_state[index] = I2C_IDLE;
}

uint8_t
ao_i2c_send(void *block, uint16_t len, uint8_t index, uint8_t stop)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint8_t		*b = block;
	uint32_t	sr1;
	int		t;

	uint8_t		tx_dma_index = ao_i2c_stm_info[index].tx_dma_index;

	/* Clear any pending ADDR bit */
	in_sr2("send clear addr", stm_i2c);
	out_cr2("send", stm_i2c, AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_DMAEN));
	ao_dma_set_transfer(tx_dma_index,
			    &stm_i2c->dr,
			    block,
			    len,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_MEM_TO_PER << STM_DMA_CCR_DIR));
			   
	ao_dma_start(tx_dma_index);
	ao_alarm(1 + len);
	cli();
	while (!ao_dma_done[tx_dma_index])
		if (ao_sleep(&ao_dma_done[tx_dma_index])) {
			printf ("send timeout\n");
			break;
		}
	ao_dma_done_transfer(tx_dma_index);
	out_cr2("send enable isr", stm_i2c,
		AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_ITEVTEN) | (1 << STM_I2C_CR2_ITERREN));
	while ((in_sr1("send_btf", stm_i2c) & (1 << STM_I2C_SR1_BTF)) == 0)
		if (ao_sleep(&ao_i2c_state[index]))
			break;
	out_cr2("send disable isr", stm_i2c, AO_STM_I2C_CR2);
	sei();
	if (stop) {
		out_cr1("stop", stm_i2c, AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP));
		ao_i2c_wait_stop(index);
	}
	return TRUE;
}

void
ao_i2c_recv_dma_isr(int index)
{
	int		i;
	struct stm_i2c	*stm_i2c = NULL;

	for (i = 0; i < STM_NUM_I2C; i++)
		if (index == ao_i2c_stm_info[i].rx_dma_index) {
			stm_i2c = ao_i2c_stm_info[i].stm_i2c;
			break;
		}
	if (!stm_i2c)
		return;
	stm_i2c->cr2 = AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_LAST);
	ao_dma_done[index] = 1;
	ao_wakeup(&ao_dma_done[index]);
}

uint8_t
ao_i2c_recv(void *block, uint16_t len, uint8_t index, uint8_t stop)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint8_t		*b = block;
	int		t;
	uint8_t		ret = TRUE;

	if (len == 0)
		return TRUE;
	if (len == 1) {
		ao_i2c_recv_data[index] = block;
		ao_i2c_recv_len[index] = 1;
		out_cr1("setup recv 1", stm_i2c, AO_STM_I2C_CR1);

		/* Clear any pending ADDR bit */
		in_sr2("clear addr", stm_i2c);

		/* Enable interrupts to transfer the byte */
		out_cr2("setup recv 1", stm_i2c,
			AO_STM_I2C_CR2 |
			(1 << STM_I2C_CR2_ITEVTEN) |
			(1 << STM_I2C_CR2_ITERREN) |
			(1 << STM_I2C_CR2_ITBUFEN));
		if (stop)
			out_cr1("setup recv 1", stm_i2c, AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP));

		ao_alarm(1);
		cli();
		while (ao_i2c_recv_len[index])
			if (ao_sleep(&ao_i2c_recv_len[index]))
				break;
		sei();
		ret = ao_i2c_recv_len[index] == 0;
		ao_clear_alarm();
	} else {
		uint8_t		rx_dma_index = ao_i2c_stm_info[index].rx_dma_index;
		ao_dma_set_transfer(rx_dma_index,
				    &stm_i2c->dr,
				    block,
				    len,
				    (0 << STM_DMA_CCR_MEM2MEM) |
				    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
				    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
				    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
				    (1 << STM_DMA_CCR_MINC) |
				    (0 << STM_DMA_CCR_PINC) |
				    (0 << STM_DMA_CCR_CIRC) |
				    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR));
		out_cr1("recv > 1", stm_i2c, AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_ACK));
		out_cr2("recv > 1", stm_i2c, AO_STM_I2C_CR2 |
			(1 << STM_I2C_CR2_DMAEN) | (1 << STM_I2C_CR2_LAST));
		/* Clear any pending ADDR bit */
		in_sr2("clear addr", stm_i2c);

		ao_dma_start(rx_dma_index);
		ao_alarm(len);
		cli();
		while (!ao_dma_done[rx_dma_index])
			if (ao_sleep(&ao_dma_done[rx_dma_index]))
				break;
		sei();
		ao_clear_alarm();
		ret = ao_dma_done[rx_dma_index];
		ao_dma_done_transfer(rx_dma_index);
		out_cr1("stop recv > 1", stm_i2c, AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP));
	}
	if (stop)
		ao_i2c_wait_stop(index);
	return ret;
}

void
ao_i2c_channel_init(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;

	/* Turn I2C off while configuring */
	stm_i2c->cr1 = 0;
	stm_i2c->cr2 = AO_STM_I2C_CR2;

	(void) stm_i2c->sr1;
	(void) stm_i2c->sr2;
	(void) stm_i2c->dr;

	stm_i2c->sr1 = 0;
	stm_i2c->sr2 = 0;

	stm_i2c->ccr = ((1 << STM_I2C_CCR_FS) |
			(0 << STM_I2C_CCR_DUTY) |
			(20 << STM_I2C_CCR_CCR));
	

	stm_i2c->cr1 = AO_STM_I2C_CR1;
}

void
ao_i2c_init(void)
{
	/* All of the I2C configurations are on port B */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
#if HAS_I2C_1
# if I2C_1_PB6_PB7
	stm_afr_set(&stm_gpiob, 6, STM_AFR_AF4);
	stm_afr_set(&stm_gpiob, 7, STM_AFR_AF4);
# else
#  if I2C_1_PB8_PB9
	stm_afr_set(&stm_gpiob, 8, STM_AFR_AF4);
	stm_afr_set(&stm_gpiob, 9, STM_AFR_AF4);
#  else
#   error "No I2C_1 port configuration specified"
#  endif
# endif

	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_I2C1EN);
	ao_i2c_channel_init(0);

	stm_nvic_set_enable(STM_ISR_I2C1_EV_POS);
	stm_nvic_set_priority(STM_ISR_I2C1_EV_POS, 3);
	stm_nvic_set_enable(STM_ISR_I2C1_ER_POS);
	stm_nvic_set_priority(STM_ISR_I2C1_ER_POS, 3);
#endif

#if HAS_I2C_2
# if I2C_2_PB10_PB11
	stm_afr_set(&stm_gpiob, 10, STM_AFR_AF4);
	stm_afr_set(&stm_gpiob, 11, STM_AFR_AF4);
# else
#  error "No I2C_2 port configuration specified"
# endif
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_I2C2EN);
	ao_i2c_channel_init(1);

	stm_nvic_set_enable(STM_ISR_I2C2_EV_POS);
	stm_nvic_set_priority(STM_ISR_I2C2_EV_POS, 3);
	stm_nvic_set_enable(STM_ISR_I2C2_ER_POS);
	stm_nvic_set_priority(STM_ISR_I2C2_ER_POS, 3);
#endif
}

