/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include "25lc1024.h"

/*
 * Using SPI on USART 0, with P1_2 as the chip select
 */

#define EE_CS		P1_2
#define EE_CS_INDEX	2

__xdata uint8_t ao_ee_dma_in_done;
__xdata uint8_t ao_ee_dma_out_done;
__xdata uint8_t ao_ee_mutex;

uint8_t	ao_ee_dma_out_id;
uint8_t ao_ee_dma_in_id;

static __xdata uint8_t	ao_ee_const = 0xff;

#define ao_ee_delay() do { \
	_asm nop _endasm; \
	_asm nop _endasm; \
	_asm nop _endasm; \
} while(0)

void ao_ee_cs_low(void)
{
	ao_ee_delay();
	EE_CS = 0;
	ao_ee_delay();
}

void ao_ee_cs_high(void)
{
	ao_ee_delay();
	EE_CS = 1;
	ao_ee_delay();
}

/* Send bytes over SPI.
 *
 * This sets up two DMA engines, one writing the data and another reading
 * bytes coming back.  We use the bytes coming back to tell when the transfer
 * is complete, as the transmit register is double buffered and hence signals
 * completion one byte before the transfer is actually complete
 */
static void
ao_ee_send(void __xdata *block, uint16_t len)
{
	ao_dma_set_transfer(ao_ee_dma_in_id,
			    &U0DBUFXADDR,
			    &ao_ee_const,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_ee_dma_out_id,
			    block,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_ee_dma_in_id);
	ao_dma_start(ao_ee_dma_out_id);
	ao_dma_trigger(ao_ee_dma_out_id);
	__critical while (!ao_ee_dma_in_done)
		ao_sleep(&ao_ee_dma_in_done);
}

/* Receive bytes over SPI.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the SPI transmitter as that is what
 * clocks the data coming in.
 */
static void
ao_ee_recv(void __xdata *block, uint16_t len)
{
	ao_dma_set_transfer(ao_ee_dma_in_id,
			    &U0DBUFXADDR,
			    block,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_ee_dma_out_id,
			    &ao_ee_const,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_ee_dma_in_id);
	ao_dma_start(ao_ee_dma_out_id);
	ao_dma_trigger(ao_ee_dma_out_id);
	__critical while (!ao_ee_dma_in_done)
		ao_sleep(&ao_ee_dma_in_done);
}

#define EE_BLOCK	256

struct ao_ee_instruction {
	uint8_t	instruction;
	uint8_t	address[3];
} __xdata ao_ee_instruction;

static void
ao_ee_write_enable(void)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_WREN;
	ao_ee_send(&ao_ee_instruction, 1);
	ao_ee_cs_high();
}

static uint8_t
ao_ee_rdsr(void)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_RDSR;
	ao_ee_send(&ao_ee_instruction, 1);
	ao_ee_recv(&ao_ee_instruction, 1);
	ao_ee_cs_high();
	return ao_ee_instruction.instruction;
}

static void
ao_ee_wrsr(uint8_t status)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_WRSR;
	ao_ee_instruction.address[0] = status;
	ao_ee_send(&ao_ee_instruction, 2);
	ao_ee_cs_high();
}

#define EE_BLOCK_NONE	0xffff

__xdata uint8_t ao_ee_data[EE_BLOCK];
__data uint16_t ao_ee_block = EE_BLOCK_NONE;
__data uint8_t	ao_ee_block_dirty;

/* Write the current block to the EEPROM */
static void
ao_ee_write_block(void)
{
	uint8_t	status;

	status = ao_ee_rdsr();
	if (status & (EE_STATUS_BP0|EE_STATUS_BP1|EE_STATUS_WPEN)) {
		status &= ~(EE_STATUS_BP0|EE_STATUS_BP1|EE_STATUS_WPEN);
		ao_ee_wrsr(status);
	}
	ao_ee_write_enable();
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_WRITE;
	ao_ee_instruction.address[0] = ao_ee_block >> 8;
	ao_ee_instruction.address[1] = ao_ee_block;
	ao_ee_instruction.address[2] = 0;
	ao_ee_send(&ao_ee_instruction, 4);
	ao_ee_send(ao_ee_data, EE_BLOCK);
	ao_ee_cs_high();
	for (;;) {
		uint8_t	status = ao_ee_rdsr();
		if ((status & EE_STATUS_WIP) == 0)
			break;
	}
}

/* Read the current block from the EEPROM */
static void
ao_ee_read_block(void)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_READ;
	ao_ee_instruction.address[0] = ao_ee_block >> 8;
	ao_ee_instruction.address[1] = ao_ee_block;
	ao_ee_instruction.address[2] = 0;
	ao_ee_send(&ao_ee_instruction, 4);
	ao_ee_recv(ao_ee_data, EE_BLOCK);
	ao_ee_cs_high();
}
	
void
ao_ee_flush(void)
{
	if (ao_ee_block_dirty) {
		ao_ee_write_block();
		ao_ee_block_dirty = 0;
	}
}

static void
ao_ee_fill(uint16_t block)
{
	if (block != ao_ee_block) {
		ao_ee_flush();
		ao_ee_block = block;
		ao_ee_read_block();
	}
}

uint8_t
ao_ee_write(uint32_t pos, uint8_t *buf, uint16_t len)
{
	uint16_t block;
	uint16_t this_len;
	uint8_t	this_off;
	
	if (pos >= AO_EE_DATA_SIZE || pos + len > AO_EE_DATA_SIZE)
		return 0;
	while (len) {
		
		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = pos;
		this_len = 256 - (uint16_t) this_off;
		block = (uint16_t) (pos >> 8);
		if (this_len > len)
			this_len = len;
		if (this_len & 0xff00)
			ao_panic(AO_PANIC_EE);

		/* Transfer the data */
		ao_mutex_get(&ao_ee_mutex); {
			if (this_len != 256)
				ao_ee_fill(block);
			else {
				ao_ee_flush();
				ao_ee_block = block;
			}
			memcpy(ao_ee_data + this_off, buf, this_len);
			ao_ee_block_dirty = 1;
		} ao_mutex_put(&ao_ee_mutex);

		/* See how much is left */
		buf += this_len;
		len -= this_len;
	}
	return 1;
}

uint8_t
ao_ee_read(uint32_t pos, uint8_t *buf, uint16_t len)
{
	uint16_t block;
	uint16_t this_len;
	uint8_t	this_off;
	
	if (pos >= AO_EE_DATA_SIZE || pos + len > AO_EE_DATA_SIZE)
		return 0;
	while (len) {
		
		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = pos;
		this_len = 256 - (uint16_t) this_off;
		block = (uint16_t) (pos >> 8);
		if (this_len > len)
			this_len = len;
		if (this_len & 0xff00)
			ao_panic(AO_PANIC_EE);

		/* Transfer the data */
		ao_mutex_get(&ao_ee_mutex); {
			ao_ee_fill(block);
			memcpy(buf, ao_ee_data + this_off, this_len);
		} ao_mutex_put(&ao_ee_mutex);

		/* See how much is left */
		buf += this_len;
		len -= this_len;
	}
	return 1;
}

/*
 * Read/write the config block, which is in
 * the last block of the ao_eeprom
 */
uint8_t
ao_ee_write_config(uint8_t *buf, uint16_t len)
{
	if (len > AO_EE_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_ee_mutex); {
		ao_ee_fill(AO_EE_CONFIG_BLOCK);
		memcpy(ao_ee_data, buf, len);
		ao_ee_block_dirty = 1;
	} ao_mutex_put(&ao_ee_mutex);
	return 1;
}

uint8_t
ao_ee_read_config(uint8_t *buf, uint16_t len)
{
	if (len > AO_EE_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_ee_mutex); {
		ao_ee_fill(AO_EE_CONFIG_BLOCK);
		memcpy(buf, ao_ee_data, len);
	} ao_mutex_put(&ao_ee_mutex);
	return 1;
}

/*
 * To initialize the chip, set up the CS line and
 * the SPI interface
 */
void
ao_ee_init(void)
{
	/* set up CS */
	EE_CS = 1;
	P1DIR |= (1 << EE_CS_INDEX);
	P1SEL &= ~(1 << EE_CS_INDEX);

	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U0CFG_ALT_MASK) | PERCFG_U0CFG_ALT_2;

	/* Ensure that USART0 takes precidence over USART1 for pins that
	 * they share
	 */
	P2SEL = (P2SEL & ~P2SEL_PRI3P1_MASK) | P2SEL_PRI3P1_USART0;

	/* Make the SPI pins be controlled by the USART peripheral */
	P1SEL |= ((1 << 5) | (1 << 4) | (1 << 3));

	/* Set up OUT DMA */
	ao_ee_dma_out_id = ao_dma_alloc(&ao_ee_dma_out_done);

	/* Set up IN DMA */
	ao_ee_dma_in_id = ao_dma_alloc(&ao_ee_dma_in_done);

	/* Set up the USART.
	 *
	 * SPI master mode
	 */
	U0CSR = (UxCSR_MODE_SPI | UxCSR_RE | UxCSR_MASTER);

	/* Set the baud rate and signal parameters
	 *
	 * The cc1111 is limited to a 24/8 MHz SPI clock,
	 * while the 25LC1024 is limited to 20MHz. So,
	 * use the 3MHz clock (BAUD_E 17, BAUD_M 0)
	 */
	U0BAUD = 0;
	U0GCR = (UxGCR_CPOL_NEGATIVE |
		 UxGCR_CPHA_FIRST_EDGE |
		 UxGCR_ORDER_MSB |
		 (17 << UxGCR_BAUD_E_SHIFT));
}
