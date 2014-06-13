/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_TRACKER_H_
#define _AO_TRACKER_H_

/* Any motion more than this will result in a log entry */

#define AO_TRACKER_MOTION_DEFAULT	10
#define AO_TRACKER_INTERVAL_DEFAULT	1

#define AO_TRACKER_MOTION_COUNT		10

void
ao_tracker_init(void);

void
ao_tracker_erase_start(uint16_t flight);

void
ao_tracker_erase_end(void);

#endif /* _AO_TRACKER_H_ */
