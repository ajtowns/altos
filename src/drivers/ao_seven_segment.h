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

#ifndef _AO_SEVEN_SEGMENT_H_
#define _AO_SEVEN_SEGMENT_H_

#define AO_SEVEN_SEGMENT_DECIMAL	0x10

#define AO_SEVEN_SEGMENT_CLEAR		0xff

void
ao_seven_segment_set(uint8_t digit, uint8_t value);

void
ao_seven_segment_clear(void);

void
ao_seven_segment_init(void);

#endif /* _AO_SEVEN_SEGMENT_H_ */
