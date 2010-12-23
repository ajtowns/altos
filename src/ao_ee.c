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
#include "25lc1024.h"

/*
 * Using SPI on USART 0, with P1_2 as the chip select
 */

#define EE_CS		P1_2
#define EE_CS_INDEX	2

__xdata uint8_t ao_ee_mutex;

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
	ao_spi_send(&ao_ee_instruction, 1);
	ao_ee_cs_high();
}

static uint8_t
ao_ee_rdsr(void)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_RDSR;
	ao_spi_send(&ao_ee_instruction, 1);
	ao_spi_recv(&ao_ee_instruction, 1);
	ao_ee_cs_high();
	return ao_ee_instruction.instruction;
}

static void
ao_ee_wrsr(uint8_t status)
{
	ao_ee_cs_low();
	ao_ee_instruction.instruction = EE_WRSR;
	ao_ee_instruction.address[0] = status;
	ao_spi_send(&ao_ee_instruction, 2);
	ao_ee_cs_high();
}

#define EE_BLOCK_NONE	0xffff

static __xdata uint8_t ao_ee_data[EE_BLOCK];
static __pdata uint16_t ao_ee_block = EE_BLOCK_NONE;
static __pdata uint8_t	ao_ee_block_dirty;

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
	ao_spi_send(&ao_ee_instruction, 4);
	ao_spi_send(ao_ee_data, EE_BLOCK);
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
	ao_spi_send(&ao_ee_instruction, 4);
	ao_spi_recv(ao_ee_data, EE_BLOCK);
	ao_ee_cs_high();
}

static void
ao_ee_flush_internal(void)
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
		ao_ee_flush_internal();
		ao_ee_block = block;
		ao_ee_read_block();
	}
}

uint8_t
ao_ee_write(uint32_t pos, uint8_t *buf, uint16_t len) __reentrant
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
				ao_ee_flush_internal();
				ao_ee_block = block;
			}
			memcpy(ao_ee_data + this_off, buf, this_len);
			ao_ee_block_dirty = 1;
		} ao_mutex_put(&ao_ee_mutex);

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
		pos += this_len;
	}
	return 1;
}

void
ao_ee_flush(void) __reentrant
{
	ao_mutex_get(&ao_ee_mutex); {
		ao_ee_flush_internal();
	} ao_mutex_put(&ao_ee_mutex);
}

/*
 * Read/write the config block, which is in
 * the last block of the ao_eeprom
 */
uint8_t
ao_ee_write_config(uint8_t *buf, uint16_t len) __reentrant
{
	if (len > AO_EE_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_ee_mutex); {
		ao_ee_fill(AO_EE_CONFIG_BLOCK);
		memcpy(ao_ee_data, buf, len);
		ao_ee_block_dirty = 1;
		ao_ee_flush_internal();
	} ao_mutex_put(&ao_ee_mutex);
	return 1;
}

uint8_t
ao_ee_read_config(uint8_t *buf, uint16_t len) __reentrant
{
	if (len > AO_EE_BLOCK_SIZE)
		return 0;
	ao_mutex_get(&ao_ee_mutex); {
		ao_ee_fill(AO_EE_CONFIG_BLOCK);
		memcpy(buf, ao_ee_data, len);
	} ao_mutex_put(&ao_ee_mutex);
	return 1;
}

static void
ee_dump(void) __reentrant
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
ee_store(void) __reentrant
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

__code struct ao_cmds ao_ee_cmds[] = {
	{ 'e', ee_dump, 	"e <block>                          Dump a block of EEPROM data" },
	{ 'w', ee_store,	"w <block> <start> <len> <data> ... Write data to EEPROM" },
	{ 0,   ee_store, NULL },
};

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

	ao_cmd_register(&ao_ee_cmds[0]);
}
