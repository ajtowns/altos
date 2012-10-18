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

#ifndef _AO_LOG_MICRO_H_
#define _AO_LOG_MICRO_H_

#define AO_LOG_MICRO_GROUND	(0l << 24)
#define AO_LOG_MICRO_DATA	(1l << 24)
#define AO_LOG_MICRO_DONE	(0xaal << 24)
#define AO_LOG_MICRO_MASK	(0xffl << 24)

void
ao_log_micro_data(uint32_t data);

extern uint32_t	ao_log_last_ground;
extern uint32_t	ao_log_last_done;

uint8_t
ao_log_micro_scan(void);

void
ao_log_micro_dump(void);

#endif /* _AO_LOG_MICRO_H_ */
