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

struct ao_spi_stm_info {
	uint8_t	miso_dma_index;
	uint8_t mosi_dma_index;
	struct stm_spi *stm_spi;
};

uint8_t	ao_spi_mutex[STM_NUM_SPI];

static const struct ao_spi_stm_info ao_spi_stm_info[STM_NUM_SPI] = {
	{
		.miso_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_SPI1_RX),
		.mosi_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_SPI1_TX),
		&stm_spi1
	},
	{
		.miso_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_SPI2_RX),
		.mosi_dma_index = STM_DMA_INDEX(STM_DMA_CHANNEL_SPI2_TX),
		&stm_spi2
	}
};

static uint8_t	spi_dev_null;

void
ao_spi_send(void *block, uint16_t len, uint8_t spi_index)
{
	struct stm_spi *stm_spi = ao_spi_stm_info[spi_index].stm_spi;
	uint8_t	mosi_dma_index = ao_spi_stm_info[spi_index].mosi_dma_index;
	uint8_t	miso_dma_index = ao_spi_stm_info[spi_index].miso_dma_index;

	ao_dma_set_transfer(mosi_dma_index,
			    &stm_spi->dr,
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
	/* Clear any stale data */
	(void) stm_spi->dr;
	ao_dma_set_transfer(miso_dma_index,
			    &stm_spi->dr,
			    &spi_dev_null,
			    len,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (0 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR));
	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(1 << STM_SPI_CR2_TXDMAEN) |
			(1 << STM_SPI_CR2_RXDMAEN));
	ao_dma_start(miso_dma_index);
	ao_dma_start(mosi_dma_index);
	ao_arch_critical(
		while (!ao_dma_done[miso_dma_index])
			ao_sleep(&ao_dma_done[miso_dma_index]);
		);
	ao_dma_done_transfer(mosi_dma_index);
	ao_dma_done_transfer(miso_dma_index);
}

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index)
{
	struct stm_spi *stm_spi = ao_spi_stm_info[spi_index].stm_spi;
#if 0
	uint8_t *d = block;

	while (len--) {
		stm_spi->dr = 0xff;
		while (!(stm_spi->sr & (1 << STM_SPI_SR_RXNE)));
		*d++ = stm_spi->dr;
	}
	while (stm_spi->sr & (1 << STM_SPI_SR_BSY));
#else
	uint8_t	mosi_dma_index = ao_spi_stm_info[spi_index].mosi_dma_index;
	uint8_t	miso_dma_index = ao_spi_stm_info[spi_index].miso_dma_index;

	ao_dma_set_transfer(mosi_dma_index,
			    &stm_spi->dr,
			    &spi_dev_null,
			    len,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (0 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_MEM_TO_PER << STM_DMA_CCR_DIR));
	/* Clear any stale data */
	(void) stm_spi->dr;
	ao_dma_set_transfer(miso_dma_index,
			    &stm_spi->dr,
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
	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(1 << STM_SPI_CR2_TXDMAEN) |
			(1 << STM_SPI_CR2_RXDMAEN));
	ao_dma_start(miso_dma_index);
	ao_dma_start(mosi_dma_index);
	ao_arch_critical(
		while (!ao_dma_done[miso_dma_index])
			ao_sleep(&ao_dma_done[miso_dma_index]);
		);
	ao_dma_done_transfer(mosi_dma_index);
	ao_dma_done_transfer(miso_dma_index);
#endif
}

static void
ao_spi_channel_init(uint8_t spi_index)
{
	struct stm_spi	*stm_spi = ao_spi_stm_info[spi_index].stm_spi;

	stm_spi->cr1 = 0;
	(void) stm_spi->sr;
	stm_spi->cr1 = ((0 << STM_SPI_CR1_BIDIMODE) |
			(0 << STM_SPI_CR1_BIDIOE) |
			(0 << STM_SPI_CR1_CRCEN) |
			(0 << STM_SPI_CR1_CRCNEXT) |
			(0 << STM_SPI_CR1_DFF) |
			(0 << STM_SPI_CR1_RXONLY) |
			(1 << STM_SPI_CR1_SSM) |
			(1 << STM_SPI_CR1_SSI) |
			(0 << STM_SPI_CR1_LSBFIRST) |
			(1 << STM_SPI_CR1_SPE) |
			(STM_SPI_CR1_BR_PCLK_4 << STM_SPI_CR1_BR) |
			(1 << STM_SPI_CR1_MSTR) |
			(0 << STM_SPI_CR1_CPOL) |
			(0 << STM_SPI_CR1_CPHA));
	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(0 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));
}

void
ao_spi_init(void)
{
#if HAS_SPI_1
# if SPI_1_PA5_PA6_PA7
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);
	stm_afr_set(&stm_gpioa, 5, STM_AFR_AF5);
	stm_afr_set(&stm_gpioa, 6, STM_AFR_AF5);
	stm_afr_set(&stm_gpioa, 7, STM_AFR_AF5);
# else
#  if SPI_1_PB3_PB4_PB5
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
	stm_afr_set(&stm_gpiob, 3, STM_AFR_AF5);
	stm_afr_set(&stm_gpiob, 4, STM_AFR_AF5);
	stm_afr_set(&stm_gpiob, 5, STM_AFR_AF5);
#  else
#   if SPI_1_PE13_PE14_PE15
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOEEN);
	stm_afr_set(&stm_gpioe, 13, STM_AFR_AF5);
	stm_afr_set(&stm_gpioe, 14, STM_AFR_AF5);
	stm_afr_set(&stm_gpioe, 15, STM_AFR_AF5);
#   else
#    error "No SPI_1 port configuration specified"
#   endif
#  endif
# endif

	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SPI1EN);

	ao_spi_channel_init(0);

	stm_nvic_set_enable(STM_ISR_SPI1_POS);
	stm_nvic_set_priority(STM_ISR_SPI1_POS, 3);
#endif

#if HAS_SPI_2
# if SPI_2_PB13_PB14_PB15
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
	stm_afr_set(&stm_gpiob, 13, STM_AFR_AF5);
	stm_afr_set(&stm_gpiob, 14, STM_AFR_AF5);
	stm_afr_set(&stm_gpiob, 15, STM_AFR_AF5);
# else
#  if SPI_2_PPD1_PD3_PD4
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIODEN);
	stm_afr_set(&stm_gpiod, 1, STM_AFR_AF5);
	stm_afr_set(&stm_gpiod, 3, STM_AFR_AF5);
	stm_afr_set(&stm_gpiod, 4, STM_AFR_AF5);
#  else
#   error "No SPI_2 port configuration specified"
#  endif
# endif

	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_SPI2EN);

	ao_spi_channel_init(1);

	stm_nvic_set_enable(STM_ISR_SPI2_POS);
	stm_nvic_set_priority(STM_ISR_SPI2_POS, 3);
#endif
}
