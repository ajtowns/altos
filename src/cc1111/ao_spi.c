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
#define SPI_BUF_0	&U0DBUFXADDR
#define SPI_CSR_0	U0CSR
#define SPI_BAUD_0	U0BAUD
#define SPI_GCR_0	U0GCR
#define SPI_CFG_MASK_0	PERCFG_U0CFG_ALT_MASK
#define SPI_DMA_TX_0	DMA_CFG0_TRIGGER_UTX0
#define SPI_DMA_RX_0	DMA_CFG0_TRIGGER_URX0

#if SPI_0_ALT_1
#define SPI_CFG_0	PERCFG_U0CFG_ALT_1
#define SPI_SEL_0	P0SEL
#define SPI_BITS_0	(1 << 3) | (1 << 2) | (1 << 5)
#define SPI_CSS_BIT_0	(1 << 4)
#endif

#if SPI_0_ALT_2
#define SPI_CFG_0	PERCFG_U0CFG_ALT_2
#define SPI_SEL_0	P1SEL
#define SPI_PRI_0	P2SEL_PRI3P1_USART0
#define SPI_BITS_0	(1 << 5) | (1 << 4) | (1 << 3)
#define SPI_CSS_BIT_0	(1 << 2)
#endif

#endif

#if HAS_SPI_1
#define SPI_BUF_1	&U1DBUFXADDR
#define SPI_CSR_1	U1CSR
#define SPI_BAUD_1	U1BAUD
#define SPI_GCR_1	U1GCR
#define SPI_CFG_MASK_1	PERCFG_U1CFG_ALT_MASK
#define SPI_DMA_TX_1	DMA_CFG0_TRIGGER_UTX1
#define SPI_DMA_RX_1	DMA_CFG0_TRIGGER_URX1

#if SPI_1_ALT_1
#define SPI_CFG_1	PERCFG_U1CFG_ALT_1
#define SPI_SEL_1	P0SEL
#define SPI_BITS_1	(1 << 4) | (1 << 5) | (1 << 3)
#define SPI_CSS_BIT_1	(1 << 2)
#endif

#if SPI_1_ALT_2
#define SPI_CFG_1	PERCFG_U1CFG_ALT_2
#define SPI_SEL_1	P1SEL
#define SPI_PRI_1	P2SEL_PRI3P1_USART1
#define SPI_BITS_1	(1 << 6) | (1 << 7) | (1 << 5)
#define SPI_CSS_BIT_1	(1 << 4)
#endif

#endif

#if MULTI_SPI

#define SPI_BUF(bus)		((bus) ? SPI_BUF_1 : SPI_BUF_0)
#define SPI_CSR(bus)		((bus) ? SPI_CSR_1 : SPI_CSR_0)
#define SPI_BAUD(bus)		((bus) ? SPI_BAUD_1 : SPI_BAUD_0)
#define SPI_GCR(bus)		((bus) ? SPI_GCR_1 : SPI_GCR_0)
#define SPI_CFG_MASK(bus)	((bus) ? SPI_CFG_MASK_1 : SPI_CFG_MASK_0)
#define SPI_DMA_TX(bus)		((bus) ? SPI_DMA_TX_1 : SPI_DMA_TX_0)
#define SPI_DMA_RX(bus)		((bus) ? SPI_DMA_RX_1 : SPI_DMA_RX_0)
#define SPI_CFG(bus)		((bus) ? SPI_CFG_1 : SPI_CFG_0)
#define SPI_SEL(bus)		((bus) ? SPI_SEL_1 : SPI_SEL_0)
#define SPI_BITS(bus)		((bus) ? SPI_BITS_1 : SPI_BITS_0)
#define SPI_CSS_BIT(bus)	((bus) ? SPI_CSS_BIT_1 : SPI_CSS_BIT_0)

#else

#if HAS_SPI_0
#define SPI_BUF(bus)		SPI_BUF_0
#define SPI_CSR(bus)		SPI_CSR_0
#define SPI_BAUD(bus)		SPI_BAUD_0
#define SPI_GCR(bus)		SPI_GCR_0
#define SPI_CFG_MASK(bus)	SPI_CFG_MASK_0
#define SPI_DMA_TX(bus)		SPI_DMA_TX_0
#define SPI_DMA_RX(bus)		SPI_DMA_RX_0
#define SPI_CFG(bus)		SPI_CFG_0
#define SPI_SEL(bus)		SPI_SEL_0
#define SPI_BITS(bus)		SPI_BITS_0
#define SPI_CSS_BIT(bus)	SPI_CSS_BIT_0
#endif
#if HAS_SPI_1
#define SPI_BUF(bus)		SPI_BUF_1
#define SPI_CSR(bus)		SPI_CSR_1
#define SPI_BAUD(bus)		SPI_BAUD_1
#define SPI_GCR(bus)		SPI_GCR_1
#define SPI_CFG_MASK(bus)	SPI_CFG_MASK_1
#define SPI_DMA_TX(bus)		SPI_DMA_TX_1
#define SPI_DMA_RX(bus)		SPI_DMA_RX_1
#define SPI_CFG(bus)		SPI_CFG_1
#define SPI_SEL(bus)		SPI_SEL_1
#define SPI_BITS(bus)		SPI_BITS_1
#define SPI_CSS_BIT(bus)	SPI_CSS_BIT_1
#endif

#endif /* MULTI_SPI */

#if AO_SPI_SLAVE
#define CSS(bus)		SPI_CSS_BIT(bus)
#define UxCSR_DIRECTION	UxCSR_SLAVE
#else
#define CSS(bus)		0
#define UxCSR_DIRECTION	UxCSR_MASTER
#endif

/* Shared mutex to protect SPI bus, must cover the entire
 * operation, from CS low to CS high. This means that any SPI
 * user must protect the SPI bus with this mutex
 */
__xdata uint8_t	ao_spi_mutex[N_SPI];
__xdata uint8_t ao_spi_dma_in_done[N_SPI];
__xdata uint8_t ao_spi_dma_out_done[N_SPI];

uint8_t	ao_spi_dma_out_id[N_SPI];
uint8_t ao_spi_dma_in_id[N_SPI];

static __xdata uint8_t ao_spi_const;


/* Send bytes over SPI.
 *
 * This sets up two DMA engines, one writing the data and another reading
 * bytes coming back.  We use the bytes coming back to tell when the transfer
 * is complete, as the transmit register is double buffered and hence signals
 * completion one byte before the transfer is actually complete
 */
#if MULTI_SPI
void
ao_spi_send(void __xdata *block, uint16_t len, uint8_t bus) __reentrant
#else
void
ao_spi_send_bus(void __xdata *block, uint16_t len) __reentrant
#define bus	0
#endif
{
	ao_dma_set_transfer(ao_spi_dma_in_id[bus],
			    SPI_BUF(bus),
			    &ao_spi_const,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_RX(bus),
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);
	ao_dma_set_transfer(ao_spi_dma_out_id[bus],
			    block,
			    SPI_BUF(bus),
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_TX(bus),
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_spi_dma_in_id[bus]);
	ao_dma_start(ao_spi_dma_out_id[bus]);
	ao_dma_trigger(ao_spi_dma_out_id[bus]);
#if !AO_SPI_SLAVE
	__critical while (!ao_spi_dma_in_done[bus])
		ao_sleep(&ao_spi_dma_in_done[bus]);
#endif
#undef bus
}

#if AO_SPI_SLAVE
void
ao_spi_send_wait(void)
{
	__critical while (!ao_spi_dma_in_done[0])
		ao_sleep(&ao_spi_dma_in_done[0]);
}
#endif

/* Receive bytes over SPI.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the SPI transmitter as that is what
 * clocks the data coming in.
 */
#if MULTI_SPI
void
ao_spi_recv(void __xdata *block, uint16_t len, uint8_t bus) __reentrant
#else
void
ao_spi_recv_bus(void __xdata *block, uint16_t len) __reentrant
#define bus 0
#endif
{
	ao_dma_set_transfer(ao_spi_dma_in_id[bus],
			    SPI_BUF(bus),
			    block,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_RX(bus),
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_spi_const = SPI_CONST;

#if !AO_SPI_SLAVE
	ao_dma_set_transfer(ao_spi_dma_out_id[bus],
			    &ao_spi_const,
			    SPI_BUF(bus),
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    SPI_DMA_TX(bus),
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);
#endif

	ao_dma_start(ao_spi_dma_in_id[bus]);
#if !AO_SPI_SLAVE
	ao_dma_start(ao_spi_dma_out_id[bus]);
	ao_dma_trigger(ao_spi_dma_out_id[bus]);
	__critical while (!ao_spi_dma_in_done[bus])
		ao_sleep(&ao_spi_dma_in_done[bus]);
#endif
}

#if AO_SPI_SLAVE
void
ao_spi_recv_wait(void)
{
	__critical while (!ao_spi_dma_in_done[0])
		ao_sleep(&ao_spi_dma_in_done[0]);
}
#endif

/* Set up the USART.
 *
 * SPI master/slave mode
 */
/* Set the baud rate and signal parameters
 *
 * The cc1111 is limited to a 24/8 MHz SPI clock.
 * Every peripheral I've ever seen goes faster than that,
 * so set the clock to 3MHz (BAUD_E 17, BAUD_M 0)
 */
#define SPI_INIT(bus,o)	do {						\
		/* Set up the USART pin assignment */			\
		PERCFG = (PERCFG & ~SPI_CFG_MASK(bus)) | SPI_CFG(bus);	\
									\
		/* Make the SPI pins be controlled by the USART peripheral */ \
		SPI_SEL(bus) |= SPI_BITS(bus) | CSS(bus);		\
		SPI_CSR(bus) = (UxCSR_MODE_SPI | UxCSR_RE | UxCSR_DIRECTION); \
		SPI_BAUD(bus) = 0;					\
		SPI_GCR(bus) = (UxGCR_CPOL_NEGATIVE |			\
				UxGCR_CPHA_FIRST_EDGE |			\
				UxGCR_ORDER_MSB |			\
				(17 << UxGCR_BAUD_E_SHIFT));		\
		/* Set up OUT DMA */					\
		ao_spi_dma_out_id[o] = ao_dma_alloc(&ao_spi_dma_out_done[o]); \
									\
		/* Set up IN DMA */					\
		ao_spi_dma_in_id[o] = ao_dma_alloc(&ao_spi_dma_in_done[o]);	\
	} while (0)

void
ao_spi_init(void)
{
	/* Ensure that SPI USART takes precidence over the other USART
	 * for pins that they share
	 */
#ifdef SPI_PRI
	P2SEL = (P2SEL & ~P2SEL_PRI3P1_MASK) | SPI_PRI;
#endif

#if HAS_SPI_0
	SPI_INIT(0, 0);
#endif
#if HAS_SPI_1
	SPI_INIT(1, MULTI_SPI);
#endif
}
