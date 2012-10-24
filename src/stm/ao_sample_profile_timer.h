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

#ifndef _AO_SAMPLE_PROFILE_TIMER_H_
#define _AO_SAMPLE_PROFILE_TIMER_H_

uint16_t
ao_sample_profile_timer_start(void);

void
ao_sample_profile_timer_stop(void);

void
ao_sample_profile_timer_init(void);

#define ao_sample_profile_timer_value()	((uint16_t) stm_tim11.cnt)

#endif /* _AO_SAMPLE_PROFILE_TIMER_H_ */
