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

#if HAS_EXTI_0
__xdata void 	(*ao_int_callback)(void);

void
ao_p0_isr(void) __interrupt(13)
{
	if (P0IF && (P0IFG & (AO_MS5607_MISO_MASK))) {
		(*ao_int_callback)();
	}
	P0IFG = 0;
	P0IF = 0;
}
#endif
