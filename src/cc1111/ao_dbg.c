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
#include "ao_pins.h"

static void
ao_dbg_send_bits(uint8_t msk, uint8_t val) __reentrant
{
	DBG_PORT = (DBG_PORT & ~msk) | (val & msk);
	_asm
		nop
		nop
	_endasm;
}

void
ao_dbg_send_byte(uint8_t byte)
{
	__pdata uint8_t	b, d;

	DBG_PORT |= DBG_DATA;
	DBG_PORT_DIR |= DBG_DATA;
	for (b = 0; b < 8; b++) {
		d = 0;
		if (byte & 0x80)
			d = DBG_DATA;
		byte <<= 1;
		ao_dbg_send_bits(DBG_CLOCK|DBG_DATA, DBG_CLOCK|d);
		ao_dbg_send_bits(DBG_CLOCK|DBG_DATA,     0    |d);
	}
	DBG_PORT_DIR &= ~DBG_DATA;
}

uint8_t
ao_dbg_recv_byte(void)
{
	__pdata uint8_t	byte, b;

	byte = 0;
	for (b = 0; b < 8; b++) {
		byte = byte << 1;
		ao_dbg_send_bits(DBG_CLOCK, DBG_CLOCK);
		if (DBG_DATA_PIN)
			byte |= 1;
		ao_dbg_send_bits(DBG_CLOCK, 0);
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

__pdata uint8_t	save_acc;
__pdata uint8_t save_psw;
__pdata uint8_t save_dpl0;
__pdata uint8_t save_dph0;
__pdata uint8_t save_dpl1;
__pdata uint8_t save_dph1;

static uint8_t
ao_dbg_inst1(uint8_t a) __reentrant
{
	ao_dbg_send_byte(DEBUG_INSTR(1));
	ao_dbg_send_byte(a);
	return ao_dbg_recv_byte();
}

static uint8_t
ao_dbg_inst2(uint8_t a, uint8_t b) __reentrant
{
	ao_dbg_send_byte(DEBUG_INSTR(2));
	ao_dbg_send_byte(a);
	ao_dbg_send_byte(b);
	return ao_dbg_recv_byte();
}

static uint8_t
ao_dbg_inst3(uint8_t a, uint8_t b, uint8_t c) __reentrant
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
	/* Make the DBG pins GPIOs. On TeleMetrum, this will
	 * disable the SPI link, so don't expect SPI to work after
	 * using the debugger.
	 */
	DBG_PORT_SEL &= ~(DBG_CLOCK|DBG_DATA|DBG_RESET_N);

	/* make DBG_DATA tri-state */
	DBG_PORT_INP |= DBG_DATA;

	/* Raise RESET_N and CLOCK */
	DBG_PORT |= DBG_RESET_N | DBG_CLOCK;

	/* RESET_N and CLOCK are outputs now */
	DBG_PORT_DIR |= DBG_RESET_N | DBG_CLOCK;
	DBG_PORT_DIR &= ~DBG_DATA;
}

static void
ao_dbg_long_delay(void)
{
	uint8_t	n;

	for (n = 0; n < 20; n++)
		_asm nop _endasm;
}

#define AO_RESET_LOW_DELAY	AO_MS_TO_TICKS(100)
#define AO_RESET_HIGH_DELAY	AO_MS_TO_TICKS(100)

void
ao_dbg_debug_mode(void)
{
	ao_dbg_set_pins();
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|    0    );
	ao_delay(AO_RESET_LOW_DELAY);
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N,     0    |DBG_DATA|DBG_RESET_N);
	ao_delay(AO_RESET_HIGH_DELAY);
}

void
ao_dbg_reset(void)
{
	ao_dbg_set_pins();
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_delay(AO_RESET_LOW_DELAY);
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|    0    );
	ao_dbg_long_delay();
	ao_dbg_send_bits(DBG_CLOCK|DBG_DATA|DBG_RESET_N, DBG_CLOCK|DBG_DATA|DBG_RESET_N);
	ao_delay(AO_RESET_HIGH_DELAY);
}

static void
debug_enable(void)
{
	ao_dbg_debug_mode();
}

static void
debug_reset(void)
{
	ao_dbg_reset();
}

static void
debug_put(void)
{
	for (;;) {
		ao_cmd_white ();
		if (ao_cmd_lex_c == '\n')
			break;
		ao_cmd_hex();
		if (ao_cmd_status != ao_cmd_success)
			break;
		ao_dbg_send_byte(ao_cmd_lex_i);
	}
}

static void
debug_get(void)
{
	__pdata uint16_t count;
	__pdata uint16_t i;
	__pdata uint8_t byte;
	ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	count = ao_cmd_lex_i;
	if (count > 256) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	for (i = 0; i < count; i++) {
		if (i && (i & 7) == 0)
			putchar('\n');
		byte = ao_dbg_recv_byte();
		ao_cmd_put8(byte);
		putchar(' ');
	}
	putchar('\n');
}

static uint8_t
getnibble(void)
{
	__pdata char	c;

	c = getchar();
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - ('a' - 10);
	if ('A' <= c && c <= 'F')
		return c - ('A' - 10);
	ao_cmd_status = ao_cmd_lex_error;
	return 0;
}

static void
debug_input(void)
{
	__pdata uint16_t count;
	__pdata uint16_t addr;
	__pdata uint8_t b;
	__pdata uint8_t	i;

	ao_cmd_hex();
	count = ao_cmd_lex_i;
	ao_cmd_hex();
	addr = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_dbg_start_transfer(addr);
	i = 0;
	while (count--) {
		if (!(i++ & 7))
			putchar('\n');
		b = ao_dbg_read_byte();
		ao_cmd_put8(b);
	}
	ao_dbg_end_transfer();
	putchar('\n');
}

static void
debug_output(void)
{
	__pdata uint16_t count;
	__pdata uint16_t addr;
	__pdata uint8_t b;

	ao_cmd_hex();
	count = ao_cmd_lex_i;
	ao_cmd_hex();
	addr = ao_cmd_lex_i;
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_dbg_start_transfer(addr);
	while (count--) {
		b = getnibble() << 4;
		b |= getnibble();
		if (ao_cmd_status != ao_cmd_success)
			return;
		ao_dbg_write_byte(b);
	}
	ao_dbg_end_transfer();
}

__code struct ao_cmds ao_dbg_cmds[7] = {
	{ debug_enable,	"D\0Enable debug" },
	{ debug_get,	"G <count>\0Get data" },
	{ debug_input,	"I <count> <addr>\0Input <count> at <addr>" },
	{ debug_output,	"O <count> <addr>\0Output <count> at <addr>" },
	{ debug_put,	"P <byte> ...\0Put data" },
	{ debug_reset,	"R\0Reset" },
	{ 0, NULL },
};

void
ao_dbg_init(void)
{
	ao_cmd_register(&ao_dbg_cmds[0]);
}
