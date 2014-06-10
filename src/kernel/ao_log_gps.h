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

#include "ao.h"
#include "ao_telemetry.h"

#ifndef _AO_LOG_GPS_H_
#define _AO_LOG_GPS_H_

uint8_t
ao_log_gps_should_log(int32_t lat, int32_t lon, int16_t alt);

void
ao_log_gps_flight(void);

void
ao_log_gps_data(uint16_t tick, struct ao_telemetry_location *gps_data);

#endif /* _AO_LOG_GPS_H_ */
