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

#ifndef _AO_FAST_TIMER_H_
#define _AO_FAST_TIMER_H_

void
ao_fast_timer_init(void);

#ifndef AO_FAST_TIMER_MAX
#define AO_FAST_TIMER_MAX	2
#endif

void
ao_fast_timer_on(void (*callback)(void));

void
ao_fast_timer_off(void (*callback)(void));

#endif /* _AO_FAST_TIMER_H_ */
