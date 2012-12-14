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

#ifndef _AO_MICROPEAK_H_
#define _AO_MICROPEAK_H_

#define FILTER_SHIFT		3
#define SAMPLE_SLEEP		AO_MS_TO_TICKS(96)

/* 16 sample, or about two seconds worth */
#define GROUND_AVG_SHIFT	4
#define GROUND_AVG		(1 << GROUND_AVG_SHIFT)

/* Pressure change (in Pa) to detect boost */
#define BOOST_DETECT		120	/* 10m at sea level, 12m at 2000m */

/* Wait after power on before doing anything to give the user time to assemble the rocket */
#define BOOST_DELAY		AO_SEC_TO_TICKS(30)

/* Pressure change (in Pa) to detect landing */
#define LAND_DETECT		12	/* 1m at sea level, 1.2m at 2000m */

/* Current sensor pressure value */
extern uint32_t	pa;

/* IIR filtered pressure value */
extern uint32_t	pa_avg;

/* Average pressure value on ground */
extern uint32_t	pa_ground;

/* Minimum recorded filtered pressure value */
extern uint32_t	pa_min;

/* Pressure values converted to altitudes */
extern alt_t	ground_alt, max_alt;

/* max_alt - ground_alt */
extern alt_t	ao_max_height;

#endif

