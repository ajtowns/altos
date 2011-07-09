/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

/* Shared mutex to protect SPI bus, must cover the entire
 * operation, from CS low to CS high. This means that any SPI
 * user must protect the SPI bus with this mutex
 */
__xdata uint8_t	ao_spi_mutex;
__xdata uint8_t ao_spi_dma_in_done;
__xdata uint8_t ao_spi_dma_out_done;

uint8_t	ao_spi_dma_out_id;
uint8_t ao_spi_dma_in_id;

static __xdata uint8_t ao_spi_const = 0xff;

/* Send bytes over SPI.
 *
 * This sets up two DMA engines, one writing the data and another reading
 * bytes coming back.  We use the bytes coming back to tell when the transfer
 * is complete, as the transmit register is double buffered and hence signals
 * completion one byte before the transfer is actually complete
 */
void
ao_spi_send(void __xdata *block, uint16_t len) __reentrant
{
	ao_dma_set_transfer(ao_spi_dma_in_id,
			    &U0DBUFXADDR,
			    &ao_spi_const,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_spi_dma_out_id,
			    block,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_spi_dma_in_id);
	ao_dma_start(ao_spi_dma_out_id);
	ao_dma_trigger(ao_spi_dma_out_id);
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
}

/* Receive bytes over SPI.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the SPI transmitter as that is what
 * clocks the data coming in.
 */
void
ao_spi_recv(void __xdata *block, uint16_t len) __reentrant
{
	ao_dma_set_transfer(ao_spi_dma_in_id,
			    &U0DBUFXADDR,
			    block,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_spi_dma_out_id,
			    &ao_spi_const,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_spi_dma_in_id);
	ao_dma_start(ao_spi_dma_out_id);
	ao_dma_trigger(ao_spi_dma_out_id);
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
}

/*
 * Initialize USART0 for SPI using config alt 2
 *
 * 	MO	P1_5
 * 	MI	P1_4
 * 	CLK	P1_3
 *
 * Chip select is the responsibility of the caller
 */

void
ao_spi_init(void)
{
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U0CFG_ALT_MASK) | PERCFG_U0CFG_ALT_2;

	/* Ensure that USART0 takes precidence over USART1 for pins that
	 * they share
	 */
	P2SEL = (P2SEL & ~P2SEL_PRI3P1_MASK) | P2SEL_PRI3P1_USART0;

	/* Make the SPI pins be controlled by the USART peripheral */
	P1SEL |= ((1 << 5) | (1 << 4) | (1 << 3));

	/* Set up OUT DMA */
	ao_spi_dma_out_id = ao_dma_alloc(&ao_spi_dma_out_done);

	/* Set up IN DMA */
	ao_spi_dma_in_id = ao_dma_alloc(&ao_spi_dma_in_done);

	/* Set up the USART.
	 *
	 * SPI master mode
	 */
	U0CSR = (UxCSR_MODE_SPI | UxCSR_RE | UxCSR_MASTER);

	/* Set the baud rate and signal parameters
	 *
	 * The cc1111 is limited to a 24/8 MHz SPI clock.
	 * Every peripheral I've ever seen goes faster than that,
	 * so set the clock to 3MHz (BAUD_E 17, BAUD_M 0)
	 */
	U0BAUD = 0;
	U0GCR = (UxGCR_CPOL_NEGATIVE |
		 UxGCR_CPHA_FIRST_EDGE |
		 UxGCR_ORDER_MSB |
		 (17 << UxGCR_BAUD_E_SHIFT));
}
