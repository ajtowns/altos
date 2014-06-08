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

#define AO_CONFIG_DEFAULT_TRACKER_START_HORIZ	1000
#define AO_CONFIG_DEFAULT_TRACKER_START_VERT	100

/* Speeds for the various modes, 2m/s seems reasonable for 'not moving' */
#define AO_TRACKER_NOT_MOVING	200

extern int32_t	ao_tracker_start_latitude;
extern int32_t	ao_tracker_start_longitude;
extern int16_t	ao_tracker_start_altitude;

#define AO_TRACKER_RING	4

struct ao_tracker_data {
	uint16_t			tick;
	uint8_t				new;
	uint8_t				state;
	struct ao_telemetry_location	gps_data;
	struct ao_telemetry_satellite	gps_tracking_data;
};

extern struct ao_tracker_data	ao_tracker_data[AO_TRACKER_RING];
extern uint8_t			ao_tracker_head;

#define ao_tracker_ring_next(n)	(((n) + 1) & (AO_TRACKER_RING-1))
#define ao_tracker_ring_prev(n)	(((n) - 1) & (AO_TRACKER_RING-1))

void
ao_tracker_init(void);

#endif /* _AO_TRACKER_H_ */
