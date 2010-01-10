/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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
#include "at45db161d.h"

/*
 * Using SPI on USART 0, with P1_1 as the chip select
 */

#define FLASH_CS		P1_1
#define FLASH_CS_INDEX		1

__xdata uint8_t ao_flash_dma_in_done;
__xdata uint8_t ao_flash_dma_out_done;
__xdata uint8_t ao_flash_mutex;

uint8_t	ao_flash_dma_out_id;
uint8_t ao_flash_dma_in_id;

static __xdata uint8_t	ao_flash_const = 0xff;

#define ao_flash_delay() do { \
	_asm nop _endasm; \
	_asm nop _endasm; \
	_asm nop _endasm; \
} while(0)

void ao_flash_cs_low(void)
{
	ao_flash_delay();
	FLASH_CS = 0;
	ao_flash_delay();
}

void ao_flash_cs_high(void)
{
	ao_flash_delay();
	FLASH_CS = 1;
	ao_flash_delay();
}

/* Send bytes over SPI.
 *
 * This sets up two DMA engines, one writing the data and another reading
 * bytes coming back.  We use the bytes coming back to tell when the transfer
 * is complete, as the transmit register is double buffered and hence signals
 * completion one byte before the transfer is actually complete
 */
static void
ao_flash_send(void __xdata *block, uint16_t len)
{
	ao_dma_set_transfer(ao_flash_dma_in_id,
			    &U0DBUFXADDR,
			    &ao_flash_const,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_flash_dma_out_id,
			    block,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_1 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_flash_dma_in_id);
	ao_dma_start(ao_flash_dma_out_id);
	ao_dma_trigger(ao_flash_dma_out_id);
	__critical while (!ao_flash_dma_in_done)
		ao_sleep(&ao_flash_dma_in_done);
}

/* Receive bytes over SPI.
 *
 * This sets up tow DMA engines, one reading the data and another
 * writing constant values to the SPI transmitter as that is what
 * clocks the data coming in.
 */
static void
ao_flash_recv(void __xdata *block, uint16_t len)
{
	ao_dma_set_transfer(ao_flash_dma_in_id,
			    &U0DBUFXADDR,
			    block,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_URX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_1 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_set_transfer(ao_flash_dma_out_id,
			    &ao_flash_const,
			    &U0DBUFXADDR,
			    len,
			    DMA_CFG0_WORDSIZE_8 |
			    DMA_CFG0_TMODE_SINGLE |
			    DMA_CFG0_TRIGGER_UTX0,
			    DMA_CFG1_SRCINC_0 |
			    DMA_CFG1_DESTINC_0 |
			    DMA_CFG1_PRIORITY_NORMAL);

	ao_dma_start(ao_flash_dma_in_id);
	ao_dma_start(ao_flash_dma_out_id);
	ao_dma_trigger(ao_flash_dma_out_id);
	__critical while (!ao_flash_dma_in_done)
		ao_sleep(&ao_flash_dma_in_done);
}

struct ao_flash_instruction {
	uint8_t	instruction;
	uint8_t	address[3];
} __xdata ao_flash_instruction;

static void
ao_flash_set_pagesize_512(void)
{
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_SET_CONFIG;
	ao_flash_instruction.address[0] = FLASH_SET_512_BYTE_0;
	ao_flash_instruction.address[1] = FLASH_SET_512_BYTE_1;
	ao_flash_instruction.address[2] = FLASH_SET_512_BYTE_2;
	ao_flash_send(&ao_flash_instruction, 4);
	ao_flash_cs_high();
}


static uint8_t
ao_flash_read_status(void)
{
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_READ_STATUS;
	ao_flash_send(&ao_flash_instruction, 1);
	ao_flash_recv(&ao_flash_instruction, 1);
	ao_flash_cs_high();
	return ao_flash_instruction.instruction;
}

#define FLASH_BLOCK_NONE	0xffff

static __xdata uint8_t ao_flash_data[FLASH_BLOCK_SIZE_MAX];
static __pdata uint16_t ao_flash_block = FLASH_BLOCK_NONE;
static __pdata uint8_t	ao_flash_block_dirty;
static __pdata uint8_t  ao_flash_write_pending;
static __pdata uint8_t	ao_flash_setup_done;
static __data uint32_t	ao_flash_device_size;
static __data uint8_t	ao_flash_block_shift;
static __data uint16_t	ao_flash_block_size;

static void
ao_flash_setup(void)
{
	uint8_t	status;

	if (ao_flash_setup_done)
		return;

	ao_mutex_get(&ao_flash_mutex);
	if (ao_flash_setup_done) {
		ao_mutex_put(&ao_flash_mutex);
		return;
	}

	/* On first use, check to see if the flash chip has
	 * been programmed to use 512 byte pages. If not, do so.
	 * And then, because the flash part must be power cycled
	 * for that change to take effect, panic.
	 */
	status = ao_flash_read_status();

	if (!(status & FLASH_STATUS_PAGESIZE_512)) {
		ao_flash_set_pagesize_512();
		ao_panic(AO_PANIC_FLASH);
	}

	switch (status & 0x3c) {

	/* AT45DB321D */
	case 0x34:
		ao_flash_block_shift = 9;
		ao_flash_device_size = ((uint32_t) 4 * (uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB161D */
	case 0x2c:
		ao_flash_block_shift = 9;
		ao_flash_device_size = ((uint32_t) 2 * (uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB081D */
	case 0x24:
		ao_flash_block_shift = 8;
		ao_flash_device_size = ((uint32_t) 1024 * (uint32_t) 1024);
		break;

	/* AT45DB041D */
	case 0x1c:
		ao_flash_block_shift = 8;
		ao_flash_device_size = ((uint32_t) 512 * (uint32_t) 1024);
		break;

	/* AT45DB021D */
	case 0x14:
		ao_flash_block_shift = 8;
		ao_flash_device_size = ((uint32_t) 256 * (uint32_t) 1024);
		break;

	/* AT45DB011D */
	case 0x0c:
		ao_flash_block_shift = 8;
		ao_flash_device_size = ((uint32_t) 128 * (uint32_t) 1024);
		break;

	default:
		ao_panic(AO_PANIC_FLASH);
	}
	ao_flash_block_size = 1 << ao_flash_block_shift;
	ao_flash_setup_done = 1;
	ao_mutex_put(&ao_flash_mutex);
}

static void
ao_flash_wait_write(void)
{
	if (ao_flash_write_pending) {
		for (;;) {
			uint8_t status = ao_flash_read_status();
			if ((status & FLASH_STATUS_RDY))
				break;
		}
		ao_flash_write_pending = 0;
	}
}

/* Write the current block to the FLASHPROM */
static void
ao_flash_write_block(void)
{
	ao_flash_wait_write();
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_WRITE;

	/* 13/14 block bits + 9/8 byte bits (always 0) */
	ao_flash_instruction.address[0] = ao_flash_block >> (16 - ao_flash_block_shift);
	ao_flash_instruction.address[1] = ao_flash_block << (ao_flash_block_shift - 8);
	ao_flash_instruction.address[2] = 0;
	ao_flash_send(&ao_flash_instruction, 4);
	ao_flash_send(ao_flash_data, FLASH_BLOCK_SIZE);
	ao_flash_cs_high();
	ao_flash_write_pending = 1;
}

/* Read the current block from the FLASHPROM */
static void
ao_flash_read_block(void)
{
	ao_flash_wait_write();
	ao_flash_cs_low();
	ao_flash_instruction.instruction = FLASH_READ;

	/* 13/14 block bits + 9/8 byte bits (always 0) */
	ao_flash_instruction.address[0] = ao_flash_block >> (16 - ao_flash_block_shift);
	ao_flash_instruction.address[1] = ao_flash_block << (ao_flash_block_shift - 8);
	ao_flash_instruction.address[2] = 0;
	ao_flash_send(&ao_flash_instruction, 4);
	ao_flash_recv(ao_flash_data, FLASH_BLOCK_SIZE);
	ao_flash_cs_high();
}

static void
ao_flash_flush_internal(void)
{
	if (ao_flash_block_dirty) {
		ao_flash_write_block();
		ao_flash_block_dirty = 0;
	}
}

static void
ao_flash_fill(uint16_t block)
{
	if (block != ao_flash_block) {
		ao_flash_flush_internal();
		ao_flash_block = block;
		ao_flash_read_block();
	}
}

uint8_t
ao_ee_write(uint32_t pos, uint8_t *buf, uint16_t len) __reentrant
{
	uint16_t block;
	uint16_t this_len;
	uint16_t	this_off;

	ao_flash_setup();
	if (pos >= FLASH_DATA_SIZE || pos + len > FLASH_DATA_SIZE)
		return 0;
	while (len) {

		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) pos & FLASH_BLOCK_MASK;
		this_len = FLASH_BLOCK_SIZE - this_off;
		block = (uint16_t) (pos >> FLASH_BLOCK_SHIFT);
		if (this_len > len)
			this_len = len;

		/* Transfer the data */
		ao_mutex_get(&ao_flash_mutex); {
			if (this_len != FLASH_BLOCK_SIZE)
				ao_flash_fill(block);
			else {
				ao_flash_flush_internal();
				ao_flash_block = block;
			}
			memcpy(ao_flash_data + this_off, buf, this_len);
			ao_flash_block_dirty = 1;
		} ao_mutex_put(&ao_flash_mutex);

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

uint8_t
ao_ee_read(uint32_t pos, uint8_t *buf, uint16_t len) __reentrant
{
	uint16_t block;
	uint16_t this_len;
	uint16_t this_off;

	ao_flash_setup();
	if (pos >= FLASH_DATA_SIZE || pos + len > FLASH_DATA_SIZE)
		return 0;
	while (len) {


		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) pos & FLASH_BLOCK_MASK;
		this_len = FLASH_BLOCK_SIZE - this_off;
		block = (uint16_t) (pos >> FLASH_BLOCK_SHIFT);
		if (this_len > len)
			this_len = len;

		/* Transfer the data */
		ao_mutex_get(&ao_flash_mutex); {
			ao_flash_fill(block);
			memcpy(buf, ao_flash_data + this_off, this_len);
		} ao_mutex_put(&ao_flash_mutex);

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

void
ao_ee_flush(void) __reentrant
{
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_flush_internal();
	} ao_mutex_put(&ao_flash_mutex);
}

/*
 * Read/write the config block, which is in
 * the last block of the flash
 */

uint8_t
ao_ee_write_config(uint8_t *buf, uint16_t len) __reentrant
{
	ao_flash_setup();
	if (len > FLASH_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_fill(FLASH_CONFIG_BLOCK);
		memcpy(ao_flash_data, buf, len);
		ao_flash_block_dirty = 1;
		ao_flash_flush_internal();
	} ao_mutex_put(&ao_flash_mutex);
	return 1;
}

uint8_t
ao_ee_read_config(uint8_t *buf, uint16_t len) __reentrant
{
	ao_flash_setup();
	if (len > FLASH_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_fill(FLASH_CONFIG_BLOCK);
		memcpy(buf, ao_flash_data, len);
	} ao_mutex_put(&ao_flash_mutex);
	return 1;
}

static void
flash_dump(void) __reentrant
{
	uint8_t	b;
	uint16_t block;
	uint8_t i;

	ao_cmd_hex();
	block = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	i = 0;
	do {
		if ((i & 7) == 0) {
			if (i)
				putchar('\n');
			ao_cmd_put16((uint16_t) i);
		}
		putchar(' ');
		ao_ee_read(((uint32_t) block << 8) | i, &b, 1);
		ao_cmd_put8(b);
		++i;
	} while (i != 0);
	putchar('\n');
}

static void
flash_store(void) __reentrant
{
	uint16_t block;
	uint8_t i;
	uint16_t len;
	uint8_t b;
	uint32_t addr;

	ao_cmd_hex();
	block = ao_cmd_lex_i;
	ao_cmd_hex();
	i = ao_cmd_lex_i;
	addr = ((uint32_t) block << 8) | i;
	ao_cmd_hex();
	len = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	while (len--) {
		ao_cmd_hex();
		if (ao_cmd_status != ao_cmd_success)
			return;
		b = ao_cmd_lex_i;
		ao_ee_write(addr, &b, 1);
		addr++;
	}
	ao_ee_flush();
}

void
ao_ee_dump_config(void) __reentrant
{
	uint16_t	i;
	printf("Configuration block %d\n", FLASH_CONFIG_BLOCK);
	ao_mutex_get(&ao_flash_mutex); {
		ao_flash_flush_internal();
		ao_flash_block = FLASH_BLOCK_NONE;
		ao_flash_fill(FLASH_CONFIG_BLOCK);
		i = 0;
		do {
			if ((i & 7) == 0) {
				if (i)
					putchar('\n');
				ao_cmd_put16((uint16_t) i);
			}
			putchar(' ');
			ao_cmd_put8(ao_flash_data[i]);
			++i;
		} while (i < sizeof (ao_config));
	} ao_mutex_put(&ao_flash_mutex);
}

static void
flash_status(void) __reentrant
{
	uint8_t	status;

	ao_flash_setup();
	ao_mutex_get(&ao_flash_mutex); {
		status = ao_flash_read_status();
		printf ("Flash status: 0x%02x\n", status);
		printf ("Flash block shift: %d\n", FLASH_BLOCK_SHIFT);
		printf ("Flash block size: %d\n", FLASH_BLOCK_SIZE);
		printf ("Flash block mask: %d\n", FLASH_BLOCK_MASK);
		printf ("Flash device size: %ld\n", FLASH_DEVICE_SIZE);
		printf ("Flash data size: %ld\n", FLASH_DATA_SIZE);
		printf ("Flash config block: %d\n", FLASH_CONFIG_BLOCK);
	} ao_mutex_put(&ao_flash_mutex);
	ao_ee_dump_config();
}

__code struct ao_cmds ao_flash_cmds[] = {
	{ 'e', flash_dump, 	"e <block>                          Dump a block of flash data" },
	{ 'w', flash_store,	"w <block> <start> <len> <data> ... Write data to flash" },
	{ 'f', flash_status,	"f                                  Show flash status register" },
	{ 0,   flash_store, NULL },
};

/*
 * To initialize the chip, set up the CS line and
 * the SPI interface
 */
void
ao_ee_init(void)
{
	/* set up CS */
	FLASH_CS = 1;
	P1DIR |= (1 << FLASH_CS_INDEX);
	P1SEL &= ~(1 << FLASH_CS_INDEX);

	/* Set up the USART pin assignment */
	PERCFG = (PERCFG & ~PERCFG_U0CFG_ALT_MASK) | PERCFG_U0CFG_ALT_2;

	/* Ensure that USART0 takes precidence over USART1 for pins that
	 * they share
	 */
	P2SEL = (P2SEL & ~P2SEL_PRI3P1_MASK) | P2SEL_PRI3P1_USART0;

	/* Make the SPI pins be controlled by the USART peripheral */
	P1SEL |= ((1 << 5) | (1 << 4) | (1 << 3));

	/* Set up OUT DMA */
	ao_flash_dma_out_id = ao_dma_alloc(&ao_flash_dma_out_done);

	/* Set up IN DMA */
	ao_flash_dma_in_id = ao_dma_alloc(&ao_flash_dma_in_done);

	/* Set up the USART.
	 *
	 * SPI master mode
	 */
	U0CSR = (UxCSR_MODE_SPI | UxCSR_RE | UxCSR_MASTER);

	/* Set the baud rate and signal parameters
	 *
	 * The cc1111 is limited to a 24/8 MHz SPI clock,
	 * while the at45db161d.h is limited to 20MHz. So,
	 * use the 3MHz clock (BAUD_E 17, BAUD_M 0)
	 */
	U0BAUD = 0;
	U0GCR = (UxGCR_CPOL_NEGATIVE |
		 UxGCR_CPHA_FIRST_EDGE |
		 UxGCR_ORDER_MSB |
		 (17 << UxGCR_BAUD_E_SHIFT));
	ao_cmd_register(&ao_flash_cmds[0]);
}
