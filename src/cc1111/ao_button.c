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

static __code struct {
	uint8_t	mask;
	uint8_t	reg;
} ao_buttons[] = {
#ifdef BUTTON_1_MASK
	{ BUTTON_1_MASK, BUTTON_1_REG },
#endif
#ifdef BUTTON_2_MASK
	{ BUTTON_2_MASK, BUTTON_2_REG },
#endif
#ifdef BUTTON_3_MASK
	{ BUTTON_3_MASK, BUTTON_3_REG },
#endif
};

#define NUM_BUTTONS	((sizeof ao_buttons) / sizeof (ao_buttons[0]))

static __xdata uint16_t ao_button_tick[NUM_BUTTONS];

static void
ao_button_insert(char n)
{
	uint16_t	now = ao_time();
	if ((now - ao_button_tick[n]) > 20) {
		ao_button_tick[n] = now;
		ao_fifo_insert(ao_button_fifo, n);
		ao_wakeup(&ao_button_fifo);
	}
}

static void
ao_button_isr(uint8_t flag, uint8_t reg)
{
	uint8_t	b;

	for (b = 0; b < NUM_BUTTONS; b++)
		if (ao_buttons[b].reg == reg && (ao_buttons[b].mask & flag))
			ao_button_insert(b + 1);
}

static uint8_t
ao_button_mask(uint8_t reg)
{
	uint8_t	b;
	uint8_t mask = 0;

	for (b = 0; b < NUM_BUTTONS; b++)
		if (ao_buttons[b].reg == reg)
			mask |= ao_buttons[b].mask;
	return mask;
}

char
ao_button_get(void) __critical
{
	char	b;

	while (ao_fifo_empty(ao_button_fifo))
		if (ao_sleep(&ao_button_fifo))
			return 0;
	ao_fifo_remove(ao_button_fifo, b);
	return b;
}

void
ao_button_clear(void) __critical
{
	char b;

	while (!ao_fifo_empty(ao_button_fifo))
		ao_fifo_remove(ao_button_fifo, b);
}

void
ao_p0_isr(void) ao_arch_interrupt(13)
{
	P0IF = 0;
	ao_button_isr(P0IFG, 0);
	P0IFG = 0;
}

void
ao_p1_isr(void) ao_arch_interrupt(15)
{
	P1IF = 0;
	ao_button_isr(P1IFG, 1);
	P1IFG = 0;
}

/* Shared with USB */
void
ao_p2_isr(void)
{
	ao_button_isr(P2IFG, 2);
	P2IFG = 0;
}

void
ao_button_init(void)
{
	uint8_t	mask;

	/* Pins are configured as inputs with pull-up by default */

	/* Enable interrupts for P0 inputs */
	mask = ao_button_mask(0);
	if (mask) {
		if (mask & 0x0f)
			PICTL |= PICTL_P0IENL;
		if (mask & 0xf0)
			PICTL |= PICTL_P0IENH;
		P0IFG = 0;
		P0IF = 0;
		IEN1 |= IEN1_P0IE;
		PICTL |= PICTL_P0ICON;
	}

	/* Enable interrupts for P1 inputs */
	mask = ao_button_mask(1);
	if (mask) {
		P1IEN |= mask;
		P1IFG = 0;
		P1IF = 0;
		IEN2 |= IEN2_P1IE;
		PICTL |= PICTL_P1ICON;
	}

	/* Enable interrupts for P2 inputs */
	mask = ao_button_mask(2);
	if (mask) {
		PICTL |= PICTL_P2IEN;
		P2IFG = 0;
		P2IF = 0;
		IEN2 |= IEN2_P2IE;
		PICTL |= PICTL_P2ICON;
	}
}
