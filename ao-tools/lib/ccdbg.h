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

#ifndef _CCDBG_H_
#define _CCDBG_H_

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "ccdbg-debug.h"
#include "cc-bitbang.h"
#include "cc-usb.h"
#include "ao-hex.h"

/* 8051 instructions
 */
#define NOP			0x00
#define MOV_direct_data		0x75
#define LJMP			0x02
#define MOV_Rn_data(n)		(0x78 | (n))
#define DJNZ_Rn_rel(n)		(0xd8 | (n))
#define MOV_A_direct		0xe5
#define MOV_direct1_direct2	0x85
#define MOV_direct_A		0xf5
#define MOV_DPTR_data16		0x90
#define MOV_A_data		0x74
#define MOVX_atDPTR_A		0xf0
#define MOVX_A_atDPTR		0xe0
#define INC_DPTR		0xa3
#define TRAP			0xa5
#define SJMP			0x80
#define JB			0x20

/* 8051 special function registers
 */

#define SFR_P0			0x80
#define SFR_SP			0x81
#define SFR_DPL0		0x82
#define SFR_DPH0		0x83
#define SFR_DPL1		0x84
#define SFR_DPH1		0x85

/* flash controller */
#define FWT			0xAB
#define FADDRL			0xAC
#define FADDRH			0xAD
#define FCTL			0xAE
# define FCTL_BUSY		0x80
# define FCTL_BUSY_BIT		7
# define FCTL_SWBSY		0x40
# define FCTL_SWBSY_BIT		6
# define FCTL_CONTRD		0x10
# define FCTL_WRITE		0x02
# define FCTL_ERASE		0x01
#define FWDATA			0xAF

#define SLEEP			0xBE

/* clock controller */
#define CLKCON			0xC6
#define  CLKCON_OSC32K		0x80
#define  CLKCON_OSC		0x40
#define  CLKCON_TICKSPD		0x38
#define  CLKCON_CLKSPD		0x07

/* I/O pins */
#define P0			0x80
#define P1			0x90
#define P2			0xA0
#define P0DIR			0xFD
#define P1DIR			0xFE
#define P2DIR			0xFF

/* Bit-addressable accumulator */
#define ACC(bit)		(0xE0 | (bit))

/* Bit-addressable status word */
#define PSW(bit)		(0xD0 | (bit))

struct ccdbg {
	struct cc_bitbang	*bb;
	struct cc_usb		*usb;
	struct ao_hex_image	*rom;
};


#define CC_STATE_ACC	0x1
#define CC_STATE_PSW	0x2
#define CC_STATE_DP	0x4

#define CC_STATE_NSFR	5

struct ccstate {
	uint16_t	mask;
	uint8_t		acc;
	uint8_t		sfr[CC_STATE_NSFR];
};

/* CC1111 debug port commands
 */
#define CC_CHIP_ERASE		0x14

#define CC_WR_CONFIG		0x1d
#define CC_RD_CONFIG		0x24
# define CC_CONFIG_TIMERS_OFF		(1 << 3)
# define CC_CONFIG_DMA_PAUSE		(1 << 2)
# define CC_CONFIG_TIMER_SUSPEND	(1 << 1)
# define CC_SET_FLASH_INFO_PAGE		(1 << 0)

#define CC_GET_PC		0x28
#define CC_READ_STATUS		0x34
# define CC_STATUS_CHIP_ERASE_DONE	(1 << 7)
# define CC_STATUS_PCON_IDLE		(1 << 6)
# define CC_STATUS_CPU_HALTED		(1 << 5)
# define CC_STATUS_POWER_MODE_0		(1 << 4)
# define CC_STATUS_HALT_STATUS		(1 << 3)
# define CC_STATUS_DEBUG_LOCKED		(1 << 2)
# define CC_STATUS_OSCILLATOR_STABLE	(1 << 1)
# define CC_STATUS_STACK_OVERFLOW	(1 << 0)

#define CC_SET_HW_BRKPNT	0x3b
# define CC_HW_BRKPNT_N(n)	((n) << 3)
# define CC_HW_BRKPNT_N_MASK	(0x3 << 3)
# define CC_HW_BRKPNT_ENABLE	(1 << 2)

#define CC_HALT			0x44
#define CC_RESUME		0x4c
#define CC_DEBUG_INSTR(n)	(0x54|(n))
#define CC_STEP_INSTR		0x5c
#define CC_STEP_REPLACE(n)	(0x64|(n))
#define CC_GET_CHIP_ID		0x68

/* ccdbg-command.c */
void
ccdbg_debug_mode(struct ccdbg *dbg);

void
ccdbg_reset(struct ccdbg *dbg);

uint8_t
ccdbg_chip_erase(struct ccdbg *dbg);

uint8_t
ccdbg_wr_config(struct ccdbg *dbg, uint8_t config);

uint8_t
ccdbg_rd_config(struct ccdbg *dbg);

uint16_t
ccdbg_get_pc(struct ccdbg *dbg);

uint8_t
ccdbg_read_status(struct ccdbg *dbg);

uint8_t
ccdbg_set_hw_brkpnt(struct ccdbg *dbg, uint8_t number, uint8_t enable, uint16_t addr);

uint8_t
ccdbg_halt(struct ccdbg *dbg);

uint8_t
ccdbg_resume(struct ccdbg *dbg);

uint8_t
ccdbg_debug_instr(struct ccdbg *dbg, uint8_t *instr, int nbytes);

void
ccdbg_debug_instr_discard(struct ccdbg *dbg, uint8_t *instr, int nbytes);

void
ccdbg_debug_instr_queue(struct ccdbg *dbg, uint8_t *instr, int nbytes,
			uint8_t *reply);

uint8_t
ccdbg_step_instr(struct ccdbg *dbg);

uint8_t
ccdbg_step_replace(struct ccdbg *dbg, uint8_t *instr, int nbytes);

uint16_t
ccdbg_get_chip_id(struct ccdbg *dbg);

uint8_t
ccdbg_execute(struct ccdbg *dbg, uint8_t *inst);

uint8_t
ccdbg_set_pc(struct ccdbg *dbg, uint16_t pc);

uint8_t
ccdbg_execute_hex_image(struct ccdbg *dbg, struct ao_hex_image *image);

/* ccdbg-flash.c */
uint8_t
ccdbg_flash_hex_image(struct ccdbg *dbg, struct ao_hex_image *image);

/* ccdbg-io.c */
struct ccdbg *
ccdbg_open(char *tty);

void
ccdbg_close(struct ccdbg *dbg);

void
ccdbg_cmd_write(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len);

uint8_t
ccdbg_cmd_write_read8(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len);

void
ccdbg_cmd_write_queue8(struct ccdbg *dbg, uint8_t cmd,
		       uint8_t *data, int len, uint8_t *reply);

uint16_t
ccdbg_cmd_write_read16(struct ccdbg *dbg, uint8_t cmd, uint8_t *data, int len);

void
ccdbg_send_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes);

void
ccdbg_recv_bytes(struct ccdbg *dbg, uint8_t *bytes, int nbytes);

void
ccdbg_sync(struct ccdbg *dbg);

/* ccdbg-manual.c */

void
ccdbg_manual(struct ccdbg *dbg, FILE *input);

/* ccdbg-memory.c */
uint8_t
ccdbg_write_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes);

uint8_t
ccdbg_read_memory(struct ccdbg *dbg, uint16_t addr, uint8_t *bytes, int nbytes);

uint8_t
ccdbg_write_uint8(struct ccdbg *dbg, uint16_t addr, uint8_t byte);

uint8_t
ccdbg_write_hex_image(struct ccdbg *dbg, struct ao_hex_image *image, uint16_t offset);

struct ao_hex_image *
ccdbg_read_hex_image(struct ccdbg *dbg, uint16_t address, uint16_t length);

uint8_t
ccdbg_read_sfr(struct ccdbg *dbg, uint8_t addr, uint8_t *bytes, int nbytes);

uint8_t
ccdbg_write_sfr(struct ccdbg *dbg, uint8_t addr, uint8_t *bytes, int nbytes);

/* ccdbg-rom.c */
uint8_t
ccdbg_set_rom(struct ccdbg *dbg, struct ao_hex_image *rom);

uint8_t
ccdbg_rom_contains(struct ccdbg *dbg, uint16_t addr, int nbytes);

uint8_t
ccdbg_rom_replace_xmem(struct ccdbg *dbg,
		       uint16_t addrp, uint8_t *bytesp, int nbytes);

/* ccdbg-state.c */
uint8_t
ccdbg_state_save(struct ccdbg *dbg, struct ccstate *state, unsigned int mask);

uint8_t
ccdbg_state_restore(struct ccdbg *dbg, struct ccstate *state);

void
ccdbg_state_replace_xmem(struct ccdbg *dbg, struct ccstate *state,
			 uint16_t addr, uint8_t *bytes, int nbytes);

void
ccdbg_state_replace_sfr(struct ccdbg *dbg, struct ccstate *state,
			uint8_t addr, uint8_t *bytes, int nbytes);

#endif /* _CCDBG_H_ */
