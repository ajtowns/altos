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

#define I2C_FAST	1

#define I2C_TIMEOUT	100

#define I2C_IDLE	0
#define I2C_RUNNING	1
#define I2C_ERROR	2

static uint8_t	ao_i2c_state[STM_NUM_I2C];
static uint16_t	ao_i2c_addr[STM_NUM_I2C];
uint8_t 	ao_i2c_mutex[STM_NUM_I2C];

# define I2C_HIGH_SLOW	5000	/* ns, 100kHz clock */
#ifdef TELEMEGA
# define I2C_HIGH_FAST	2000	/* ns, 167kHz clock */
#else
# define I2C_HIGH_FAST	1000	/* ns, 333kHz clock */
#endif

# define I2C_RISE_SLOW	500	/* ns */
# define I2C_RISE_FAST	100	/* ns */

/* Clock period in ns */
#define CYCLES(period)	(((period) * (AO_PCLK1 / 1000)) / 1000000)

#define max(a,b)	((a) > (b) ? (a) : (b))
#define I2C_CCR_HIGH_SLOW	max(4,CYCLES(I2C_HIGH_SLOW))
#define I2C_CCR_HIGH_FAST	max(4,CYCLES(I2C_HIGH_FAST))
#define I2C_TRISE_SLOW		(CYCLES(I2C_RISE_SLOW) + 1)
#define I2C_TRISE_FAST		(CYCLES(I2C_RISE_FAST) + 1)

#if I2C_FAST
#define I2C_TRISE	I2C_TRISE_FAST
#define I2C_CCR_HIGH	I2C_CCR_HIGH_FAST
#else
#define I2C_TRISE	I2C_TRISE_SLOW
#define I2C_CCR_HIGH	I2C_CCR_HIGH_SLOW
#endif

#if AO_PCLK1 == 2000000
# define AO_STM_I2C_CR2_FREQ	STM_I2C_CR2_FREQ_2_MHZ
#endif
#if AO_PCLK1 == 4000000
#  define AO_STM_I2C_CR2_FREQ	STM_I2C_CR2_FREQ_4_MHZ
#endif
#if AO_PCLK1 == 8000000
# define AO_STM_I2C_CR2_FREQ	STM_I2C_CR2_FREQ_8_MHZ
#endif
#if AO_PCLK1 == 16000000
# define AO_STM_I2C_CR2_FREQ	STM_I2C_CR2_FREQ_16_MHZ
#endif
#if AO_PCLK1 == 32000000
# define AO_STM_I2C_CR2_FREQ	STM_I2C_CR2_FREQ_32_MHZ
#endif

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
			 (AO_STM_I2C_CR2_FREQ << STM_I2C_CR2_FREQ))

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

uint8_t
ao_i2c_start(uint8_t index, uint16_t addr)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	int		t;

	ao_i2c_state[index] = I2C_IDLE;
	ao_i2c_addr[index] = addr;
	stm_i2c->cr2 = AO_STM_I2C_CR2;
	stm_i2c->cr1 = AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_START);
	for (t = 0; t < I2C_TIMEOUT; t++) {
		if (!(stm_i2c->cr1 & (1 << STM_I2C_CR1_START)))
			break;
	}
	ao_alarm(AO_MS_TO_TICKS(250));
	ao_arch_block_interrupts();
	stm_i2c->cr2 = AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_ITEVTEN) | (1 << STM_I2C_CR2_ITERREN);
	ao_i2c_ev_isr(index);
	while (ao_i2c_state[index] == I2C_IDLE)
		if (ao_sleep(&ao_i2c_state[index]))
			break;
	ao_arch_release_interrupts();
	ao_clear_alarm();
	return ao_i2c_state[index] == I2C_RUNNING;
}

static void
ao_i2c_wait_stop(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	int	t;

	for (t = 0; t < I2C_TIMEOUT; t++) {
		if (!(stm_i2c->cr1 & (1 << STM_I2C_CR1_STOP)))
			break;
		ao_yield();
	}
	ao_i2c_state[index] = I2C_IDLE;
}

static void
ao_i2c_wait_addr(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	int	t;

	for (t = 0; t < I2C_TIMEOUT; t++)
		if (!(stm_i2c->sr1 & (1 << STM_I2C_SR1_ADDR)))
			break;
	if (t)
		printf ("wait_addr %d\n", t);
}

uint8_t
ao_i2c_send(void *block, uint16_t len, uint8_t index, uint8_t stop)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	uint8_t		tx_dma_index = ao_i2c_stm_info[index].tx_dma_index;

	/* Clear any pending ADDR bit */
	(void) stm_i2c->sr2;
	ao_i2c_wait_addr(index);
	stm_i2c->cr2 = AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_DMAEN);
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
	ao_arch_block_interrupts();
	while (!ao_dma_done[tx_dma_index])
		if (ao_sleep(&ao_dma_done[tx_dma_index]))
			break;
	ao_clear_alarm();
	ao_dma_done_transfer(tx_dma_index);
	stm_i2c->cr2 = AO_STM_I2C_CR2 | (1 << STM_I2C_CR2_ITEVTEN) | (1 << STM_I2C_CR2_ITERREN);
	while ((stm_i2c->sr1 & (1 << STM_I2C_SR1_BTF)) == 0)
		if (ao_sleep(&ao_i2c_state[index]))
			break;
	stm_i2c->cr2 = AO_STM_I2C_CR2;
	ao_arch_release_interrupts();
	if (stop) {
		stm_i2c->cr1 = AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP);
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
	uint8_t		ret = TRUE;

	if (len == 0)
		return TRUE;
	if (len == 1) {
		ao_i2c_recv_data[index] = block;
		ao_i2c_recv_len[index] = 1;
		stm_i2c->cr1 = AO_STM_I2C_CR1;

		/* Clear any pending ADDR bit */
		stm_i2c->sr2;
		ao_i2c_wait_addr(index);

		/* Enable interrupts to transfer the byte */
		stm_i2c->cr2 = (AO_STM_I2C_CR2 |
				(1 << STM_I2C_CR2_ITEVTEN) |
				(1 << STM_I2C_CR2_ITERREN) |
				(1 << STM_I2C_CR2_ITBUFEN));
		if (stop)
			stm_i2c->cr1 = AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP);

		ao_alarm(1);
		ao_arch_block_interrupts();
		while (ao_i2c_recv_len[index])
			if (ao_sleep(&ao_i2c_recv_len[index]))
				break;
		ao_arch_release_interrupts();
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
		stm_i2c->cr1 = AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_ACK);
		stm_i2c->cr2 = AO_STM_I2C_CR2 |
			(1 << STM_I2C_CR2_DMAEN) | (1 << STM_I2C_CR2_LAST);
		/* Clear any pending ADDR bit */
		(void) stm_i2c->sr2;
		ao_i2c_wait_addr(index);

		ao_dma_start(rx_dma_index);
		ao_alarm(len);
		ao_arch_block_interrupts();
		while (!ao_dma_done[rx_dma_index])
			if (ao_sleep(&ao_dma_done[rx_dma_index]))
				break;
		ao_arch_release_interrupts();
		ao_clear_alarm();
		ret = ao_dma_done[rx_dma_index];
		ao_dma_done_transfer(rx_dma_index);
		stm_i2c->cr1 = AO_STM_I2C_CR1 | (1 << STM_I2C_CR1_STOP);
	}
	if (stop)
		ao_i2c_wait_stop(index);
	return ret;
}

void
ao_i2c_channel_init(uint8_t index)
{
	struct stm_i2c	*stm_i2c = ao_i2c_stm_info[index].stm_i2c;
	int i;

	/* Turn I2C off while configuring */
	stm_i2c->cr1 = (1 << STM_I2C_CR1_SWRST);
	for (i = 0; i < 100; i++)
		asm("nop");
	stm_i2c->cr1 = 0;
	stm_i2c->cr2 = AO_STM_I2C_CR2;

	(void) stm_i2c->sr1;
	(void) stm_i2c->sr2;
	(void) stm_i2c->dr;

	stm_i2c->sr1 = 0;
	stm_i2c->sr2 = 0;

	stm_i2c->ccr = ((I2C_FAST << STM_I2C_CCR_FS) |
			(0 << STM_I2C_CCR_DUTY) |
			(I2C_CCR_HIGH << STM_I2C_CCR_CCR));

	stm_i2c->trise = I2C_TRISE;

	stm_i2c->cr1 = AO_STM_I2C_CR1;
}

static inline void
i2c_pin_set(struct stm_gpio *gpio, int pin)
{
	stm_afr_set(gpio, pin, STM_AFR_AF4);
	stm_ospeedr_set(gpio, pin, STM_OSPEEDR_400kHz);
	stm_pupdr_set(gpio, pin, STM_PUPDR_PULL_UP);
}

void
ao_i2c_init(void)
{
	/* All of the I2C configurations are on port B */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
#if HAS_I2C_1
# if I2C_1_PB6_PB7
	i2c_pin_set(&stm_gpiob, 6);
	i2c_pin_set(&stm_gpiob, 7);
# else
#  if I2C_1_PB8_PB9
	i2c_pin_set(&stm_gpiob, 8);
	i2c_pin_set(&stm_gpiob, 9);
#  else
#   error "No I2C_1 port configuration specified"
#  endif
# endif

	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_I2C1EN);
	ao_i2c_channel_init(0);

	stm_nvic_set_enable(STM_ISR_I2C1_EV_POS);
	stm_nvic_set_priority(STM_ISR_I2C1_EV_POS, AO_STM_NVIC_MED_PRIORITY);
	stm_nvic_set_enable(STM_ISR_I2C1_ER_POS);
	stm_nvic_set_priority(STM_ISR_I2C1_ER_POS, AO_STM_NVIC_MED_PRIORITY);
#endif

#if HAS_I2C_2
# if I2C_2_PB10_PB11
	i2c_pin_set(&stm_gpiob, 10);
	i2c_pin_set(&stm_gpiob, 11);
# else
#  error "No I2C_2 port configuration specified"
# endif
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_I2C2EN);
	ao_i2c_channel_init(1);

	stm_nvic_set_enable(STM_ISR_I2C2_EV_POS);
	stm_nvic_set_priority(STM_ISR_I2C2_EV_POS, AO_STM_NVIC_MED_PRIORITY);
	stm_nvic_set_enable(STM_ISR_I2C2_ER_POS);
	stm_nvic_set_priority(STM_ISR_I2C2_ER_POS, AO_STM_NVIC_MED_PRIORITY);
#endif
}
