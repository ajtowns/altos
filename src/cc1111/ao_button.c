/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

volatile __xdata struct ao_fifo ao_button_fifo;

#define BUTTON_1_PIN	(P0_4)
#define BUTTON_1_MASK	(1 << 4)	/* P0_4 */

#define BUTTON_2_PIN	(P2_3)
#define BUTTON_2_MASK	(1 << 3)	/* P2_3 */

#define BUTTON_3_PIN	(P2_4)
#define BUTTON_3_MASK	(1 << 4)	/* P2_4 */

static void
ao_button_insert(char n)
{
	ao_fifo_insert(ao_button_fifo, n);
	ao_wakeup(&ao_button_fifo);
}

char
ao_button_get(void) __critical
{
	char	b;

	while (ao_fifo_empty(ao_button_fifo))
		ao_sleep(&ao_button_fifo);
	ao_fifo_remove(ao_button_fifo, b);
	return b;
}

void
ao_p2_isr(void)
{
	if (P2IFG & BUTTON_2_MASK)
		ao_button_insert(2);
	if (P2IFG & BUTTON_3_MASK)
		ao_button_insert(3);
	P2IFG = 0;
}

void
ao_p0_isr(void) ao_arch_interrupt(13)
{
	P0IF = 0;
	if (P0IFG & BUTTON_1_MASK)
		ao_button_insert(1);
	P0IFG = 0;
}

void
ao_button_init(void)
{
	/* Pins are configured as inputs with pull-up by default */

	/* Enable interrupts for P2_0 - P2_4
	 * Enable interrupts for P0_4 - P0_7
	 * Set P2 interrupts to falling edge
	 * Set P0 interrupts to falling edge
	 */
	
	PICTL |= PICTL_P2IEN | PICTL_P0IENH | PICTL_P2ICON | PICTL_P0ICON;

	/* Enable interrupts for P0 inputs */
	IEN1 |= IEN1_P0IE;

	/* Enable interrupts for P2 inputs */
	IEN2 |= IEN2_P2IE;
}
