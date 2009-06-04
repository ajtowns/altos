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

void
ao_beep(uint8_t beep)
{
	if (beep == 0) {
		P2_0 = 0;
		P2SEL = (P2SEL & ~P2SEL_SELP2_0_MASK) | P2SEL_SELP2_0_GPIO;
		T4CTL = 0;
	} else {
		P2SEL = (P2SEL & ~P2SEL_SELP2_0_MASK) | P2SEL_SELP2_0_PERIPHERAL;
		T4CC0 = beep;
		T4CTL = TxCTL_DIV_32 | TxCTL_MODE_MODULO | TxCTL_START;
	}
}

void
ao_beep_for(uint8_t beep, uint16_t ticks) __reentrant
{
	ao_beep(beep);
	ao_delay(ticks);
	ao_beep(0);
}

void
ao_beep_init(void)
{
	/* Our beeper is on P2_0, which is hooked to timer 4 using
	 * configuration alternative 2
	 */
	P2_0 = 0;
	P2SEL = (P2SEL & ~P2SEL_SELP2_0_MASK) | P2SEL_SELP2_0_GPIO;
	PERCFG = (PERCFG & ~PERCFG_T4CFG_ALT_MASK) | PERCFG_T4CFG_ALT_2;
	T4CCTL0 = TxCCTLy_CMP_TOGGLE|TxCCTLy_CMP_MODE_ENABLE;
}
