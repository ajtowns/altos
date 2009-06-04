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

static uint8_t save_acc[] = {
	1,	NOP,
	0
};

static uint8_t save_sfr[] = {
	2,	MOV_A_direct,	0,
#define SAVE_SFR_ADDR	2
	0,
};

struct sfr_state {
	uint8_t		address;
	uint16_t	mask;
	char		*name;
};

static struct sfr_state	sfrs[CC_STATE_NSFR] = {
	{ SFR_DPL0,	CC_STATE_DP,	"dpl0" },
	{ SFR_DPH0,	CC_STATE_DP,	"dph0" },
	{ SFR_DPL1,	CC_STATE_DP,	"dpl1" },
	{ SFR_DPH1,	CC_STATE_DP,	"dph1" },
	{ PSW(0),	CC_STATE_PSW,	"psw"  },
};

uint8_t
ccdbg_state_save(struct ccdbg *dbg, struct ccstate *state, unsigned int mask)
{
	int	i;

	mask |= CC_STATE_ACC;
	if (mask & CC_STATE_ACC)
		state->acc = ccdbg_execute(dbg, save_acc);
	for (i = 0; i < CC_STATE_NSFR; i++) {
		if (sfrs[i].mask & mask) {
			save_sfr[SAVE_SFR_ADDR] = sfrs[i].address;
			state->sfr[i] = ccdbg_execute(dbg, save_sfr);
		}
	}
	state->mask = mask;
	return 0;
}

static uint8_t restore_sfr[] = {
	3,	MOV_direct_data,	0,	0,
#define RESTORE_SFR_ADDR	2
#define RESTORE_SFR_DATA	3
	0
};

static uint8_t restore_acc[] = {
	2,	MOV_A_data,	0,
#define RESTORE_ACC_DATA	2
	0
};

uint8_t
ccdbg_state_restore(struct ccdbg *dbg, struct ccstate *state)
{
	int i;
	for (i = CC_STATE_NSFR - 1; i >= 0; i--) {
		if (sfrs[i].mask & state->mask) {
			restore_sfr[RESTORE_SFR_ADDR] = sfrs[i].address;
			restore_sfr[RESTORE_SFR_DATA] = state->sfr[i];
			ccdbg_execute(dbg, restore_sfr);
		}
	}
	if (state->mask & CC_STATE_ACC) {
		restore_acc[RESTORE_ACC_DATA] = state->acc;
		ccdbg_execute(dbg, restore_acc);
	}
	state->mask = 0;
	return 0;
}

static void
ccdbg_state_replace(uint16_t sfr_addr, uint8_t sfr, char *name,
		    uint16_t addr, uint8_t *bytes, int nbytes)
{
	sfr_addr += 0xdf00;

	if (addr <= sfr_addr && sfr_addr < addr + nbytes) {
		fprintf(stderr, "replacing %s at 0x%04x - read 0x%02x saved 0x%02x\n",
			name, sfr_addr, bytes[sfr_addr - addr], sfr);
		bytes[sfr_addr - addr] = sfr;
	}
}

void
ccdbg_state_replace_xmem(struct ccdbg *dbg, struct ccstate *state,
			 uint16_t addr, uint8_t *bytes, int nbytes)
{
	int i;
	if (state->mask & CC_STATE_ACC)
		ccdbg_state_replace(ACC(0), state->acc, "acc",
				    addr, bytes, nbytes);
	for (i = 0; i < CC_STATE_NSFR; i++)
		if (state->mask & sfrs[i].mask)
			ccdbg_state_replace(sfrs[i].address, state->sfr[i],
					    sfrs[i].name, addr, bytes, nbytes);
}

void
ccdbg_state_replace_sfr(struct ccdbg *dbg, struct ccstate *state,
			uint8_t addr, uint8_t *bytes, int nbytes)
{
	ccdbg_state_replace_xmem(dbg, state, (uint16_t) addr + 0xdf00, bytes, nbytes);
}
