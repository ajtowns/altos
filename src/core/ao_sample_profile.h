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

#ifndef _AO_SAMPLE_PROFILE_H_
#define _AO_SAMPLE_PROFILE_H_

#include <ao_sample_profile_timer.h>

void
ao_sample_profile_point(uint32_t pc, uint16_t tick, uint8_t in_isr);

void
ao_sample_profile_init(void);

#endif /* _AO_SAMPLE_PROFILE_H_ */
