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

/* Default pin usage for existing Altus Metrum devices */
#if !HAS_SPI_0 && !HAS_SPI_1
#define HAS_SPI_0	1
#define SPI_0_ALT_2	1
#endif

#ifndef SPI_CONST
#define SPI_CONST	0xff
#endif

/*
 * USART0 SPI config alt 1
 * 
 *	MO	P0_3
 *	MI	P0_2
 *	CLK	P0_5
 *	SS	P0_4
 *
 * USART0 SPI config alt 2
 *
 * 	MO	P1_5
 * 	MI	P1_4
 * 	CLK	P1_3
 *	CSS	P1_2
 *
 * USART1 SPI config alt 1
 *
 *	MO	P0_4
 *	MI	P0_5
 *	CLK	P0_3
 *	SS	P0_2
 *
 * USART1 SPI config alt 2
 *
 *	MO	P1_6
 *	MI	P1_7
 *	CLK	P1_5
 *	SS	P1_4
 *
 *
 * Chip select is the responsibility of the caller in master mode
 */

#if HAS_SPI_0
#define SPI_CSR		U0CSR
#define SPI_BUF		U0DBUFXADDR
#define SPI_BAUD	U0BAUD
#define SPI_GCR		U0GCR
#define SPI_CFG_MASK	PERCFG_U0CFG_ALT_MASK
#define SPI_DMA_TX	DMA_CFG0_TRIGGER_UTX0
#define SPI_DMA_RX	DMA_CFG0_TRIGGER_URX0

#if SPI_0_ALT_1
#define SPI_CFG		PERCFG_U0CFG_ALT_1
#define SPI_SEL		P0SEL
#define SPI_BITS	(1 << 3) | (1 << 2) | (1 << 5)
#define SPI_CSS_BIT	(1 << 4)
#endif

#if SPI_0_ALT_2
#define SPI_CFG		PERCFG_U0CFG_ALT_2
#define SPI_SEL		P1SEL
#define SPI_PRI		P2SEL_PRI3P1_USART0
#define SPI_BITS	(1 << 5) | (1 << 4) | (1 << 3)
#define SPI_CSS_BIT	(1 << 2)
#endif

#endif

#if HAS_SPI_1
#define SPI_CSR		U1CSR
#define SPI_BUF		U1DBUFXADDR
#define SPI_BAUD	U1BAUD
#define SPI_GCR		U1GCR
#define SPI_CFG_MASK	PERCFG_U1CFG_ALT_MASK
#define SPI_DMA_TX	DMA_CFG0_TRIGGER_UTX1
#define SPI_DMA_RX	DMA_CFG0_TRIGGER_URX1

#if SPI_1_ALT_1
#define SPI_CFG		PERCFG_U1CFG_ALT_1
#define SPI_SEL		P0SEL
#define SPI_BITS	(1 << 4) | (1 << 5) | (1 << 3)
#define SPI_CSS_BIT	(1 << 2)
#endif

#if SPI_1_ALT_2
#define SPI_CFG		PERCFG_U1CFG_ALT_2
#define SPI_SEL		P1SEL
#define SPI_PRI		P2SEL_PRI3P1_USART1
#define SPI_BITS	(1 << 6) | (1 << 7) | (1 << 5)
#define SPI_CSS_BIT	(1 << 4)
#endif

#endif

#if AO_SPI_SLAVE
#define CSS		SPI_CSS_BIT
#define UxCSR_DIRECTION	UxCSR_SLAVE
#else
#define CSS		0
#define UxCSR_DIRECTION	UxCSR_MASTER
#endif

/* Shared mutex to protect SPI bus, must cover the entire
 * operation, from CS low to CS high. This means that any SPI
 * user must protect the SPI bus with this mutex
 */
__xdata uint8_t	ao_spi_mutex;
__xdata uint8_t ao_spi_dma_in_done;
__xdata uint8_t ao_spi_dma_out_done;

uint8_t	ao_spi_dma_out_id;
uint8_t ao_spi_dma_in_id;

static __xdata uint8_t ao_spi_const;

/* Send bytes over SPI.
 *
 * This sets up two DMA engines, one writing the data and another reading
 * bytes coming back.  We use the bytes coming back to tell when the transfer
 * is complete, as the transmit register is double buffered and hence signals
 * completion one byte before the transfer is actually complete
 */
void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant
{
	ao_dma_set_transfer(ao_spi_dma_in_id,
			    &SPI_BUF,
			    &ao_spi_const,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_RX,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);
	ao_dma_set_transfer(ao_spi_dma_out_id,
			    block,
			    &SPI_BUF,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_TX,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_spi_dma_in_id);
	ao_dma_start(ao_spi_dma_out_id);
	ao_dma_trigger(ao_spi_dma_out_id);
#if !AO_SPI_SLAVE
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
#endif
}

#if AO_SPI_SLAVE
void
ao_spi_send_wait(void)
{
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
}
#endif

/* Receive bytes over SPI.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the SPI transmitter as that is what
 * clocks the data coming in.
 */
void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant
{
	ao_dma_set_transfer(ao_spi_dma_in_id,
			    &SPI_BUF,
			    block,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_RX,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_spi_const = SPI_CONST;

#if !AO_SPI_SLAVE
	ao_dma_set_transfer(ao_spi_dma_out_id,
			    &ao_spi_const,
			    &SPI_BUF,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_TX,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);
#endif

	ao_dma_start(ao_spi_dma_in_id);
#if !AO_SPI_SLAVE
	ao_dma_start(ao_spi_dma_out_id);
	ao_dma_trigger(ao_spi_dma_out_id);
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
#endif
}

#if AO_SPI_SLAVE
void
ao_spi_recv_wait(void)
{
	__critical while (!ao_spi_dma_in_done)
		ao_sleep(&ao_spi_dma_in_done);
}
#endif

void
ao_spi_init(void)
{
	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~SPI_CFG_MASK) | SPI_CFG;

	/* Ensure that SPI USART takes precidence over the other USART
	 * for pins that they share
	 */
#ifdef SPI_PRI
	P2SEL = (P2SEL & ~P2SEL_PRI3P1_MASK) | SPI_PRI;
#endif

	/* Make the SPI pins be controlled by the USART peripheral */
	SPI_SEL |= SPI_BITS | CSS;

	/* Set up OUT DMA */
	ao_spi_dma_out_id = ao_dma_alloc(&ao_spi_dma_out_done);

	/* Set up IN DMA */
	ao_spi_dma_in_id = ao_dma_alloc(&ao_spi_dma_in_done);

	/* Set up the USART.
	 *
	 * SPI master/slave mode
	 */
	SPI_CSR = (UxCSR_MODE_SPI | UxCSR_RE | UxCSR_DIRECTION);

	/* Set the baud rate and signal parameters
	 *
	 * The cc1111 is limited to a 24/8 MHz SPI clock.
	 * Every peripheral I've ever seen goes faster than that,
	 * so set the clock to 3MHz (BAUD_E 17, BAUD_M 0)
	 */
	SPI_BAUD = 0;
	SPI_GCR = (UxGCR_CPOL_NEGATIVE |
		   UxGCR_CPHA_FIRST_EDGE |
		   UxGCR_ORDER_MSB |
		   (17 << UxGCR_BAUD_E_SHIFT));
}
