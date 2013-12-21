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

#ifndef _AO_PROFILE_H_
#define _AO_PROFILE_H_

void	ao_profile_init();

static inline uint32_t ao_profile_tick(void) {
	uint16_t	hi, lo, second_hi;

	do {
		hi = stm_tim2.cnt;
		lo = stm_tim4.cnt;
		second_hi = stm_tim2.cnt;
	} while (hi != second_hi);
	return ((uint32_t) hi << 16) | lo;
}

#endif
