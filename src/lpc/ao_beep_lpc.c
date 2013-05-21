/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
		lpc_ct32b1.tcr = ((0 << LPC_CT32B_TCR_CEN) |
				  (1 << LPC_CT32B_TCR_CRST));
		lpc_scb.sysahbclkctrl &= ~(1 << LPC_SCB_SYSAHBCLKCTRL_CT32B1);
	} else {
		lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_CT32B1);

		/* Set prescaler to match cc1111 clocks
		 */
		lpc_ct32b1.pr = AO_LPC_SYSCLK / 750000 - 1;

		/* Write the desired data in the match registers */

		/* Reset after two time units */
		lpc_ct32b1.mr[0] = beep << 1;

		/* PWM width is half of that */
		lpc_ct32b1.mr[1] = beep;

		/* Flip output 1 on PWM match */
		lpc_ct32b1.emr = (LPC_CT32B_EMR_EMC_TOGGLE << LPC_CT32B_EMR_EMC1);

		/* Reset on match 0 */
		lpc_ct32b1.mcr = (1 << LPC_CT32B_MCR_MR0R);

		/* PWM on match 1 */
		lpc_ct32b1.pwmc = (1 << LPC_CT32B_PWMC_PWMEN1);
		
		/* timer mode */
		lpc_ct32b1.ctcr = 0;

		/* And turn the timer on */
		lpc_ct32b1.tcr = ((1 << LPC_CT32B_TCR_CEN) |
				  (0 << LPC_CT32B_TCR_CRST));
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
	/* Our beeper is on c32b1_mat1
	 * which is on pin pio0_14
	 */

	lpc_ioconf.pio0_14 = ((LPC_IOCONF_FUNC_PIO0_14_CT32B1_MAT1 << LPC_IOCONF_FUNC) |
			      (LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE) |
			      (0 << LPC_IOCONF_HYS) |
			      (0 << LPC_IOCONF_INV) |
			      (1 << LPC_IOCONF_ADMODE) |
			      (0 << LPC_IOCONF_OD));

	lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_CT32B1);

	/* Disable the counter and reset the value */
	lpc_ct32b1.tcr = ((0 << LPC_CT32B_TCR_CEN) |
			  (1 << LPC_CT32B_TCR_CRST));
}
