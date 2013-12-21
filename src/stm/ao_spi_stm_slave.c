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

struct ao_spi_stm_slave_info {
	uint8_t	miso_dma_index;
	uint8_t mosi_dma_index;
	struct stm_spi *stm_spi;
};

static uint8_t		ao_spi_slave_mutex[STM_NUM_SPI];
static uint8_t		ao_spi_slave_index[STM_NUM_SPI];

static const struct ao_spi_stm_slave_info ao_spi_stm_slave_info[STM_NUM_SPI] = {
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
ao_spi_slave_send(void *block, uint16_t len)
{
	struct stm_spi *stm_spi = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].stm_spi;
	uint8_t	mosi_dma_index = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].mosi_dma_index;
	uint8_t	miso_dma_index = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].miso_dma_index;

	/* Set up the transmit DMA to deliver data */
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

	/* Clear RXNE */
	(void) stm_spi->dr;

	/* Set up the receive DMA -- when this is done, we know the SPI unit
	 * is idle. Without this, we'd have to poll waiting for the BSY bit to
	 * be cleared
	 */
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

uint8_t
ao_spi_slave_recv(void *block, uint16_t len)
{
	struct stm_spi *stm_spi = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].stm_spi;
	uint8_t	mosi_dma_index = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].mosi_dma_index;
	uint8_t	miso_dma_index = ao_spi_stm_slave_info[AO_SPI_INDEX(SPI_SLAVE_INDEX)].miso_dma_index;

	/* Set up transmit DMA to make the SPI hardware actually run */
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

	/* Clear RXNE */
	(void) stm_spi->dr;

	/* Set up the receive DMA to capture data */
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

	/* Wait until the SPI unit is done */
	ao_arch_critical(
		while (!ao_dma_done[miso_dma_index])
			ao_sleep(&ao_dma_done[miso_dma_index]);
		);

	ao_dma_done_transfer(mosi_dma_index);
	ao_dma_done_transfer(miso_dma_index);
	return 1;
}

static void
ao_spi_slave_disable_index(uint8_t spi_index)
{
	/* Disable current config
	 */
	switch (AO_SPI_INDEX(spi_index)) {
	case STM_SPI_INDEX(1):
		switch (spi_index) {
		case AO_SPI_1_PA5_PA6_PA7:
			stm_gpio_set(&stm_gpioa, 5, 1);
			stm_moder_set(&stm_gpioa, 5, STM_MODER_OUTPUT);
			stm_moder_set(&stm_gpioa, 6, STM_MODER_INPUT);
			stm_moder_set(&stm_gpioa, 7, STM_MODER_OUTPUT);
			break;
		case AO_SPI_1_PB3_PB4_PB5:
			stm_gpio_set(&stm_gpiob, 3, 1);
			stm_moder_set(&stm_gpiob, 3, STM_MODER_OUTPUT);
			stm_moder_set(&stm_gpiob, 4, STM_MODER_INPUT);
			stm_moder_set(&stm_gpiob, 5, STM_MODER_OUTPUT);
			break;
		case AO_SPI_1_PE13_PE14_PE15:
			stm_gpio_set(&stm_gpioe, 13, 1);
			stm_moder_set(&stm_gpioe, 13, STM_MODER_OUTPUT);
			stm_moder_set(&stm_gpioe, 14, STM_MODER_INPUT);
			stm_moder_set(&stm_gpioe, 15, STM_MODER_OUTPUT);
			break;
		}
		break;
	case STM_SPI_INDEX(2):
		switch (spi_index) {
		case AO_SPI_2_PB13_PB14_PB15:
			stm_gpio_set(&stm_gpiob, 13, 1);
			stm_moder_set(&stm_gpiob, 13, STM_MODER_OUTPUT);
			stm_moder_set(&stm_gpiob, 14, STM_MODER_INPUT);
			stm_moder_set(&stm_gpiob, 15, STM_MODER_OUTPUT);
			break;
		case AO_SPI_2_PD1_PD3_PD4:
			stm_gpio_set(&stm_gpiod, 1, 1);
			stm_moder_set(&stm_gpiod, 1, STM_MODER_OUTPUT);
			stm_moder_set(&stm_gpiod, 3, STM_MODER_INPUT);
			stm_moder_set(&stm_gpiod, 4, STM_MODER_OUTPUT);
			break;
		}
		break;
	}
}

static void
ao_spi_slave_enable_index(uint8_t spi_index)
{
	switch (AO_SPI_INDEX(spi_index)) {
	case STM_SPI_INDEX(1):
		switch (spi_index) {
		case AO_SPI_1_PA5_PA6_PA7:
			stm_afr_set(&stm_gpioa, 5, STM_AFR_AF5);
			stm_afr_set(&stm_gpioa, 6, STM_AFR_AF5);
			stm_afr_set(&stm_gpioa, 7, STM_AFR_AF5);
			break;
		case AO_SPI_1_PB3_PB4_PB5:
			stm_afr_set(&stm_gpiob, 3, STM_AFR_AF5);
			stm_afr_set(&stm_gpiob, 4, STM_AFR_AF5);
			stm_afr_set(&stm_gpiob, 5, STM_AFR_AF5);
			break;
		case AO_SPI_1_PE13_PE14_PE15:
			stm_afr_set(&stm_gpioe, 13, STM_AFR_AF5);
			stm_afr_set(&stm_gpioe, 14, STM_AFR_AF5);
			stm_afr_set(&stm_gpioe, 15, STM_AFR_AF5);
			break;
		}
		break;
	case STM_SPI_INDEX(2):
		switch (spi_index) {
		case AO_SPI_2_PB13_PB14_PB15:
			stm_afr_set(&stm_gpiob, 13, STM_AFR_AF5);
			stm_afr_set(&stm_gpiob, 14, STM_AFR_AF5);
			stm_afr_set(&stm_gpiob, 15, STM_AFR_AF5);
			break;
		case AO_SPI_2_PD1_PD3_PD4:
			stm_afr_set(&stm_gpiod, 1, STM_AFR_AF5);
			stm_afr_set(&stm_gpiod, 3, STM_AFR_AF5);
			stm_afr_set(&stm_gpiod, 4, STM_AFR_AF5);
			break;
		}
		break;
	}
}

void
ao_spi_slave_get(uint8_t spi_index, uint32_t speed)
{
	uint8_t		id = AO_SPI_INDEX(spi_index);
	struct stm_spi	*stm_spi = ao_spi_stm_slave_info[id].stm_spi;

	ao_mutex_get(&ao_spi_slave_mutex[id]);
	stm_spi->cr1 = ((0 << STM_SPI_CR1_BIDIMODE) |			/* Three wire mode */
			(0 << STM_SPI_CR1_BIDIOE) |
			(0 << STM_SPI_CR1_CRCEN) |			/* CRC disabled */
			(0 << STM_SPI_CR1_CRCNEXT) |
			(0 << STM_SPI_CR1_DFF) |
			(0 << STM_SPI_CR1_RXONLY) |
			(1 << STM_SPI_CR1_SSM) |        		/* Software SS handling */
			(1 << STM_SPI_CR1_SSI) |			/*  ... */
			(0 << STM_SPI_CR1_LSBFIRST) |			/* Big endian */
			(1 << STM_SPI_CR1_SPE) |			/* Enable SPI unit */
			(speed << STM_SPI_CR1_BR) |	/* baud rate to pclk/4 */
			(1 << STM_SPI_CR1_MSTR) |
			(0 << STM_SPI_CR1_CPOL) |			/* Format 0 */
			(0 << STM_SPI_CR1_CPHA));
	if (spi_index != ao_spi_slave_index[id]) {
		
		/* Disable old config
		 */
		ao_spi_slave_disable_index(ao_spi_slave_index[id]);

		/* Enable new config
		 */
		ao_spi_slave_enable_index(spi_index);
		
		/* Remember current config
		 */
		ao_spi_slave_index[id] = spi_index;
	}
}

void
ao_spi_slave_put(uint8_t spi_index)
{
	uint8_t		id = AO_SPI_INDEX(spi_index);
	struct stm_spi	*stm_spi = ao_spi_stm_slave_info[id].stm_spi;

	stm_spi->cr1 = 0;
	ao_mutex_put(&ao_spi_slave_mutex[id]);
}

static void
ao_spi_channel_init(uint8_t spi_index)
{
	uint8_t		id = AO_SPI_INDEX(spi_index);
	struct stm_spi	*stm_spi = ao_spi_stm_slave_info[id].stm_spi;

	ao_spi_slave_disable_index(spi_index);

	stm_spi->cr1 = 0;
	(void) stm_spi->sr;
	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(0 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));
}

void
ao_spi_slave_init(void)
{
#if HAS_SPI_SLAVE_1
# if SPI_1_PA5_PA6_PA7
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);
# endif
# if SPI_1_PB3_PB4_PB5
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
# endif
# if SPI_1_PE13_PE14_PE15
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOEEN);
# endif
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SPI1EN);
	ao_spi_slave_index[0] = AO_SPI_CONFIG_NONE;
	ao_spi_channel_init(0);
#endif

#if HAS_SPI_SLAVE_2
# if SPI_2_PB13_PB14_PB15
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
# endif
# if SPI_2_PD1_PD3_PD4
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIODEN);
# endif
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_SPI2EN);
	ao_spi_slave_index[1] = AO_SPI_CONFIG_NONE;
	ao_spi_channel_init(1);
#endif
}
