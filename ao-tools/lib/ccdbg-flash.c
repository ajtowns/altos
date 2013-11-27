/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
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

#include "ccdbg.h"

/* From SWRA124 section 3.1.6 */

static uint8_t flash_page[] = {

	MOV_direct_data, P1DIR, 0x02,
	MOV_direct_data, P1,	0xFF,

	MOV_direct_data, FADDRH, 0,
#define FLASH_ADDR_HIGH	8

	MOV_direct_data, FADDRL, 0,
#define FLASH_ADDR_LOW	11

	MOV_DPTR_data16, 0, 0,
#define RAM_ADDR_HIGH	13
#define RAM_ADDR_LOW	14

	MOV_Rn_data(7), 0,
#define FLASH_WORDS_HIGH	16

	MOV_Rn_data(6), 0,
#define FLASH_WORDS_LOW		18

	MOV_direct_data, FWT, 0x20,
#define FLASH_TIMING		21

	MOV_direct_data, FCTL, FCTL_ERASE,
/* eraseWaitLoop: */
		MOV_A_direct,		FCTL,
	JB, ACC(FCTL_BUSY_BIT), 0xfb,

	MOV_direct_data, P1, 0xfd,

	MOV_direct_data, FCTL, FCTL_WRITE,
/* writeLoop: */
		MOV_Rn_data(5), 2,
/* writeWordLoop: */
			MOVX_A_atDPTR,
			INC_DPTR,
			MOV_direct_A, FWDATA,
		DJNZ_Rn_rel(5), 0xfa,		/* writeWordLoop */
/* writeWaitLoop: */
			MOV_A_direct, FCTL,
		JB, ACC(FCTL_SWBSY_BIT), 0xfb,		/* writeWaitLoop */
	DJNZ_Rn_rel(6), 0xf1,			/* writeLoop */
	DJNZ_Rn_rel(7), 0xef,			/* writeLoop */

	MOV_direct_data, P1DIR, 0x00,
	MOV_direct_data, P1,	0xFF,
	TRAP,
};

#define FLASH_RAM	0xf000

#if 0
static uint8_t	flash_erase_page[] = {
	3,	MOV_direct_data, FADDRH, 0,
#define ERASE_PAGE_HIGH	3

	3,	MOV_direct_data, FADDRL, 0,
#define ERASE_PAGE_LOW	7

	3,	MOV_direct_data, FWT, 0x2A,
	3,	MOV_direct_data, FCTL, FCTL_ERASE,
	0
};

static uint8_t	flash_read_control[] = {
	2,	MOV_A_direct,	FCTL,
	0
};
#endif

#if 0
static uint8_t	flash_control_clear[] = {
	3,	MOV_direct_data,	FCTL, 0,
	2,	MOV_A_direct,		FCTL,
	0
};
#endif

#if 0
static uint8_t
ccdbg_flash_erase_page(struct ccdbg *dbg, uint16_t addr)
{
	uint16_t	page_addr = addr >> 1;
	uint8_t		status;
	uint8_t		old[0x10], new[0x10];
	int		i;

	ccdbg_read_memory(dbg, addr, old, 0x10);
	flash_erase_page[ERASE_PAGE_HIGH] = page_addr >> 8;
	flash_erase_page[ERASE_PAGE_LOW] = page_addr & 0xff;
	status = ccdbg_execute(dbg, flash_erase_page);
	ccdbg_debug(CC_DEBUG_FLASH, "erase status 0x%02x\n", status);
	do {
		status = ccdbg_execute(dbg, flash_read_control);
		ccdbg_debug(CC_DEBUG_FLASH, "fctl 0x%02x\n", status);
	} while (status & FCTL_BUSY);
	ccdbg_read_memory(dbg, addr, new, 0x10);
	for (i = 0; i < 0x10; i++)
		ccdbg_debug(CC_DEBUG_FLASH, "0x%02x -> 0x%02x\n", old[i], new[i]);
	status = ccdbg_execute(dbg, flash_control_clear);
	ccdbg_debug(CC_DEBUG_FLASH, "clear fctl 0x%02x\n", status);
	return 0;
}
#endif

#if 0
static uint8_t flash_write[] = {
	MOV_direct_data, P1DIR, 0x02,
	MOV_direct_data, P1,	0xFD,

	MOV_A_direct, FCTL,
	JB,	ACC(FCTL_BUSY_BIT), 0xf1,

	MOV_direct_data, FCTL, 0x20,

	MOV_direct_data, FADDRH, 0,
#define WRITE_PAGE_HIGH	16

	MOV_direct_data, FADDRL, 0,
#define WRITE_PAGE_LOW	19

	MOV_direct_data, FCTL, FCTL_WRITE,
	MOV_direct_data, FWDATA, 0,
#define WRITE_BYTE_0	25
	MOV_direct_data, FWDATA, 0,
#define WRITE_BYTE_1	28
	MOV_A_direct, FCTL,
	JB,	ACC(FCTL_SWBSY_BIT), 0xf1,

	MOV_direct_data, P1,	0xFF,
	TRAP,
};
#endif

static uint8_t
ccdbg_clock_init(struct ccdbg *dbg)
{
	static uint8_t set_clkcon_fast[] = {
		3,	MOV_direct_data,	CLKCON, 0x00,
		0
	};

	static uint8_t get_sleep[] = {
		2,	MOV_A_direct, SLEEP,
		0
	};

	uint8_t status;

	ccdbg_execute(dbg, set_clkcon_fast);
	do {
		status = ccdbg_execute(dbg, get_sleep);
	} while (!(status & 0x40));
	return 0;
}

#if 0
static uint8_t
ccdbg_flash_write_word(struct ccdbg *dbg, uint16_t addr, uint8_t data[2])
{
	uint16_t page_addr = addr >> 1;
	uint8_t check[2];
	uint8_t status;
	int i;

	flash_write[WRITE_PAGE_HIGH] = page_addr >> 8;
	flash_write[WRITE_PAGE_LOW] = page_addr & 0xff;
	flash_write[WRITE_BYTE_0] = data[0];
	flash_write[WRITE_BYTE_1] = data[1];
	ccdbg_debug(CC_DEBUG_FLASH, "upload flash write\n");
	ccdbg_write_memory(dbg, 0xf000, flash_write, sizeof(flash_write));
	ccdbg_set_pc(dbg, 0xf000);
	ccdbg_resume(dbg);
	for (;;) {
		status = ccdbg_read_status(dbg);
		ccdbg_debug(CC_DEBUG_FLASH, "waiting for write 0x%02x\n", status);
		if ((status & CC_STATUS_CPU_HALTED) != 0)
			break;
		sleep (1);
	}
	status = ccdbg_execute(dbg, flash_control_clear);
	ccdbg_debug(CC_DEBUG_FLASH, "clear fctl 0x%02x\n", status);
	ccdbg_read_memory(dbg, addr, check, 2);
	for (i = 0; i < 2; i++)
		ccdbg_debug(CC_DEBUG_FLASH, "0x%02x : 0x%02x\n", data[i], check[i]);
	return 0;
}
#endif

#define TIMERS_OFF		0x08
#define DMA_PAUSE		0x04
#define TIMER_SUSPEND		0x02
#define SEL_FLASH_INFO_PAGE	0x01

#if 0
static uint8_t
ccdbg_flash_lock(struct ccdbg *dbg, uint8_t lock)
{
	uint8_t	config;
	uint8_t bytes[2];
	uint8_t	old[1], new[1];

	config = ccdbg_rd_config(dbg);
	ccdbg_wr_config(dbg, config|SEL_FLASH_INFO_PAGE);
	bytes[0] = lock;
	bytes[1] = 0;
	ccdbg_flash_erase_page(dbg, 0);
	ccdbg_read_memory(dbg, 0, old, 1);
	ccdbg_flash_write_word(dbg, 0, bytes);
	ccdbg_read_memory(dbg, 0, new, 1);
	ccdbg_debug(CC_DEBUG_FLASH, "flash lock 0x%02x -> 0x%02x\n", old[0], new[0]);
	ccdbg_wr_config(dbg, config & ~SEL_FLASH_INFO_PAGE);
	return 0;
}
#endif

uint8_t
ccdbg_flash_hex_image(struct ccdbg *dbg, struct ao_hex_image *image)
{
	uint16_t flash_prog;
	uint16_t flash_len;
	uint8_t	fwt;
	uint16_t flash_addr;
	uint16_t flash_word_addr;
	uint16_t flash_words;
	uint8_t flash_words_high, flash_words_low;
	uint16_t ram_addr;
	uint8_t status;
	uint16_t remain, this_time, start;
	uint8_t verify[0x400];
	int times;

	ccdbg_clock_init(dbg);
	if (image->address + image->length > 0x8000) {
		fprintf(stderr, "cannot flash image from 0x%04x to 0x%04x\n",
			image->address, image->address + image->length);
		return 1;
	}
	if (image->address & 0x3ff) {
		fprintf(stderr, "flash image must start on page boundary\n");
		return 1;
	}
	ram_addr = 0xf000;


	flash_prog = 0xf400;

	fwt = 0x20;

	flash_page[FLASH_TIMING] = fwt;
	ccdbg_debug(CC_DEBUG_FLASH, "Upload %d flash program bytes to 0x%04x\n",
	       sizeof (flash_page), flash_prog);
	ccdbg_write_memory(dbg, flash_prog, flash_page, sizeof(flash_page));

	remain = image->length;
	start = 0;
	while (remain) {
		this_time = remain;
		if (this_time > 0x400)
			this_time = 0x400;

		ccdbg_debug(CC_DEBUG_FLASH, "Upload %d bytes at 0x%04x\n", this_time, ram_addr);
		ccdbg_write_memory(dbg, ram_addr, image->data + start, this_time);
#if 0
		ccdbg_debug(CC_DEBUG_FLASH, "Verify %d bytes in ram\n", this_time);
		ccdbg_read_memory(dbg, ram_addr, verify, this_time);
		if (memcmp (image->data + start, verify, this_time) != 0) {
			fprintf(stderr, "ram verify failed\n");
			return 1;
		}
#endif

		flash_addr = image->address + start;
		flash_word_addr = flash_addr >> 1;
		flash_len = this_time + (this_time & 1);
		flash_words = flash_len >> 1;

		flash_words_low = flash_words & 0xff;
		flash_words_high = flash_words >> 8;

		/* The flash code above is lame */
		if (flash_words_low)
			flash_words_high++;

		ccdbg_write_uint8(dbg, flash_prog + FLASH_ADDR_HIGH, flash_word_addr >> 8);
		ccdbg_write_uint8(dbg, flash_prog + FLASH_ADDR_LOW, flash_word_addr & 0xff);

		ccdbg_write_uint8(dbg, flash_prog + RAM_ADDR_HIGH, ram_addr >> 8);
		ccdbg_write_uint8(dbg, flash_prog + RAM_ADDR_LOW, ram_addr & 0xff);

		ccdbg_write_uint8(dbg, flash_prog + FLASH_WORDS_HIGH, flash_words_high);
		ccdbg_write_uint8(dbg, flash_prog + FLASH_WORDS_LOW, flash_words_low);

		ccdbg_set_pc(dbg, flash_prog);
		ccdbg_debug(CC_DEBUG_FLASH, "Flashing %d bytes at 0x%04x\n",
			    this_time, flash_addr);
		status = ccdbg_resume(dbg);
		for (times = 0; times < 10; times++) {
			status = ccdbg_read_status(dbg);
			ccdbg_debug(CC_DEBUG_FLASH, ".");
			ccdbg_flush(CC_DEBUG_FLASH);
			if ((status & CC_STATUS_CPU_HALTED) != 0)
				break;
			usleep(10000);
		}
		ccdbg_debug(CC_DEBUG_FLASH, "\n");
		if (times == 10) {
			fprintf(stderr, "flash page timed out\n");
			return 1;
		}

		ccdbg_debug(CC_DEBUG_FLASH, "Verify %d bytes in flash\n", this_time);
		ccdbg_read_memory(dbg, flash_addr, verify, this_time);
		if (memcmp (image->data + start, verify, this_time) != 0) {
			int i;
			fprintf(stderr, "flash verify failed\n");
			for (i = 0; i < this_time; i++) {
				if (image->data[start + i] != verify[i])
					fprintf(stderr, "0x%04x: 0x%02x != 0x%02x\n",
						start + i, image->data[start+i], verify[i]);
			}
			return 1;
		}
		remain -= this_time;
		start += this_time;
	}
	return 0;
}
