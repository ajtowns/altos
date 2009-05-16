/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "aoview.h"
#include <math.h>

static int	pad_pres;
static int	pad_accel;

static int	pad_pres_total;
static int	pad_accel_total;
static double	pad_lat_total;
static double	pad_lon_total;
static int	pad_alt_total;
static int	npad;
static int	prev_tick;
static double	prev_accel;
static double	velocity;
static double	pad_lat;
static double	pad_lon;
static double	pad_alt;

#define NUM_PAD_SAMPLES	50



static void
aoview_great_circle (double start_lat, double start_lon,
		     double end_lat, double end_lon,
		     double *dist, double *bearing)
{
	double rad = M_PI / 180;
	double earth_radius = 6371.2;
	double a = (90 - start_lat) * rad;
	double b = (90 - end_lat) * rad;
	double phi = (end_lon - start_lon) * rad;
	double cosr = cos(a) * cos(b) + sin(a) * sin(b) * cos(phi);
	double r = acos(cosr);
	double rdist = earth_radius * r;
	double sinth = sin(phi) * sin(b) / sin(r);
	double th = asin(sinth) / rad;
	*dist = rdist;
	*bearing = th;
}

void
aoview_state_notify(struct aostate *state)
{
	int	altitude;
	double	accel;
	double	velocity_change;
	int	ticks;
	double	dist;
	double	bearing;

	if (!strcmp(state->state, "pad")) {
		if (npad < NUM_PAD_SAMPLES) {
			pad_accel_total += state->accel;
			pad_pres_total += state->pres;
			pad_lat_total += state->lat;
			pad_lon_total += state->lon;
			pad_alt_total += state->alt;
			npad++;
			velocity = 0;
		}
		if (npad <= NUM_PAD_SAMPLES) {
			pad_pres = pad_pres_total / npad;
			pad_accel = pad_accel_total / npad;
			pad_lat = pad_lat_total / npad;
			pad_lon = pad_lon_total / npad;
			pad_alt = pad_alt_total / npad;
		}
	}
	altitude = aoview_pres_to_altitude(state->pres) - aoview_pres_to_altitude(pad_pres);
	accel = (pad_accel - state->accel) / 264.8 *  9.80665;
	velocity_change = (accel + prev_accel) / 2.0;
	ticks = state->tick - prev_tick;
	velocity -= velocity_change * (ticks / 100.0);

	prev_accel = accel;
	prev_tick = state->tick;
	printf ("Pad altitude: %dm\n", aoview_pres_to_altitude(pad_pres));
	printf ("AGL: %dm\n", altitude);
	printf ("Acceleration: %gm/s²\n", accel);
	printf ("Velocity: %gm/s\n", velocity);
	printf ("Lat: %g\n", state->lat);
	printf ("Lon: %g\n", state->lon);
	printf ("GPS alt: %d\n", state->alt);
	aoview_great_circle(pad_lat, pad_lon, state->lat, state->lon,
			    &dist, &bearing);
	printf ("Course: %gkm %g°\n", dist, bearing);
}

void
aoview_state_init(GladeXML *xml)
{
}
