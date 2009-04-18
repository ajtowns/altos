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

#define DBG_CLOCK	(1 << 3)
#define DBG_DATA	(1 << 4)
#define DBG_RESET_N	(1 << 5)

#define DBG_CLOCK_PIN	(P0_3)
#define DBG_DATA_PIN	(P0_4)
#define DBG_RESET_N_PIN	(P0_5)

static void
ao_dbg_send_bits(uint8_t msk, uint8_t val)
{
	P0 = (P0 & ~msk) | (val & msk);
}

#define ao_dbg_pause() do { _asm nop; nop; nop _endasm; } while (0);

void
ao_dbg_send_byte(uint8_t byte)
{
	__xdata uint8_t	b, d;

	P0 |= DBG_DATA;
	P0DIR |= DBG_DATA;
	for (b = 0; b < 8; b++) {
		d = 0;
		if (byte & 0x80)
			d = DBG_DATA;
		byte <<= 1;
		ao_dbg_send_bits(DBG_CLOCK|DBG_DATA, DBG_CLOCK|d);
		ao_dbg_pause();
		ao_dbg_send_bits(DBG_CLOCK|DBG_DATA,     0    |d);
		ao_dbg_pause();
	}
	P0DIR &= ~DBG_DATA;
}

uint8_t
ao_dbg_recv_byte(void)
{
	__xdata uint8_t	byte, b;

	byte = 0;
	for (b = 0; b < 8; b++) {
		byte = byte << 1;
		ao_dbg_send_bits(DBG_CLOCK, DBG_CLOCK);
		ao_dbg_pause();
		if (DBG_DATA_PIN)
			byte |= 1;
		ao_dbg_send_bits(DBG_CLOCK, 0);
		ao_dbg_pause();
	}
	return byte;
}

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

#define DEBUG_INSTR(l)		(0x54 | (l))

#define SFR_PSW			0xD0
#define SFR_DPL0		0x82
#define SFR_DPH0		0x83
#define SFR_DPL1		0x84
#define SFR_DPH1		0x85

__xdata uint8_t	save_acc;
__xdata uint8_t save_psw;
__xdata uint8_t save_dpl0;
__xdata uint8_t save_dph0;
__xdata uint8_t save_dpl1;
__xdata uint8_t save_dph1;

static uint8_t
ao_dbg_inst1(uint8_t a)
{
	ao_dbg_send_byte(DEBUG_INSTR(1));
	ao_dbg_send_byte(a);
	return ao_dbg_recv_byte();
}

static uint8_t
ao_dbg_inst2(uint8_t a, uint8_t b)
{
	ao_dbg_send_byte(DEBUG_INSTR(2));
	ao_dbg_send_byte(a);
	ao_dbg_send_byte(b);
	return ao_dbg_recv_byte();
}

static uint8_t
ao_dbg_inst3(uint8_t a, uint8_t b, uint8_t c)
{
	ao_dbg_send_byte(DEBUG_INSTR(3));
	ao_dbg_send_byte(a);
	ao_dbg_send_byte(b);
	ao_dbg_send_byte(c);
	return ao_dbg_recv_byte();
}

void
ao_dbg_start_transfer(uint16_t addr)
{
	save_acc  = ao_dbg_inst1(NOP);
	save_psw  = ao_dbg_inst2(MOV_A_direct, SFR_PSW);
	save_dpl0 = ao_dbg_inst2(MOV_A_direct, SFR_DPL0);
	save_dph0 = ao_dbg_inst2(MOV_A_direct, SFR_DPH0);
	save_dpl1 = ao_dbg_inst2(MOV_A_direct, SFR_DPL1);
	save_dph1 = ao_dbg_inst2(MOV_A_direct, SFR_DPH1);
	ao_dbg_inst3(MOV_DPTR_data16, addr >> 8, addr);
}

void
ao_dbg_end_transfer(void)
{
	ao_dbg_inst3(MOV_direct_data, SFR_DPL0, save_dpl0);
	ao_dbg_inst3(MOV_direct_data, SFR_DPH0, save_dph0);
	ao_dbg_inst3(MOV_direct_data, SFR_DPL1, save_dpl1);
	ao_dbg_inst3(MOV_direct_data, SFR_DPH1, save_dph1);
	ao_dbg_inst3(MOV_direct_data, SFR_PSW, save_psw);
	ao_dbg_inst2(MOV_A_data, save_acc);
}

void
ao_dbg_write_byte(uint8_t byte)
{
	ao_dbg_inst2(MOV_A_data, byte);
	ao_dbg_inst1(MOVX_atDPTR_A);
	ao_dbg_inst1(INC_DPTR);
}

uint8_t
ao_dbg_read_byte(void)
{
	ao_dbg_inst1(MOVX_A_atDPTR);
	return ao_dbg_inst1(INC_DPTR);
}

static void
ao_dbg_set_pins(void)
{
	/* Disable peripheral use of P0 */
	ADCCFG = 0;
	P0SEL = 0;
	
	
	/* make P0_4 tri-state */
	P0INP = DBG_DATA;
	P2INP &= ~(P2INP_PDUP0_PULL_DOWN);
	
	/* Raise RESET_N and CLOCK */
	P0 = DBG_RESET_N | DBG_CLOCK;

	/* RESET_N and CLOCK are outputs now */
	P0DIR = DBG_RESET_N | DBG_CLOCK;
}

void
ao_dbg_debug_mode(void)
{
	ao_dbg_set_pins();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|DBG_RESET_N);
}

void
ao_dbg_reset(void)
{
	ao_dbg_set_pins();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
}

