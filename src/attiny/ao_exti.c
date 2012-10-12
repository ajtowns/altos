/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_exti.h>

static void 	(*pcint_callback)(void);
static uint8_t	pcint_mask;

ISR(PCINT0_vect)
{
	if (PINB & pcint_mask)
		(*pcint_callback)();
}

void
ao_exti_setup_port(uint8_t pin, uint8_t mode, void (*callback)(void))
{
	pcint_callback = callback;
	pcint_mask = (1 << pin);
	ao_exti_disable(PORTB, pin);
	GIMSK |= (1 << PCIE);
}
