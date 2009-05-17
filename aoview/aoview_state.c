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
	double lat1 = rad * start_lat;
	double lon1 = -rad * start_lon;
	double lat2 = rad * end_lat;
	double lon2 = -rad * end_lon;

	double d = acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2));
	double argacos = (sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1));
	double crs;
	if (sin(lon2-lon1) < 0)
		crs = acos(argacos);
	else
		crs = 2 * M_PI - acos(argacos);
	*dist = d * earth_radius;
	*bearing = crs * 180/M_PI;
}

static void
aoview_state_add_deg(char *label, double deg)
{
	double	int_part;
	double	min;

	int_part = floor (deg);
	min = (deg - int_part) * 60.0;
	aoview_table_add_row(label, "%d°%lf'",
			     (int) int_part, min);

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
	double	temp;
	double	battery;
	double	drogue_sense, main_sense;

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
	temp = ((state->temp / 32767.0 * 3.3) - 0.5) / 0.01;
	battery = (state->batt / 32767.0 * 5.0);
	drogue_sense = (state->drogue / 32767.0 * 15.0);
	main_sense = (state->main / 32767.0 * 15.0);

	prev_accel = accel;
	prev_tick = state->tick;
	aoview_table_start();
	aoview_table_add_row("RSSI", "%ddB", state->rssi);
	aoview_table_add_row("Height", "%dm", altitude);
	aoview_table_add_row("Acceleration", "%gm/s²", accel);
	aoview_table_add_row("Velocity", "%gm/s", velocity);
	aoview_table_add_row("Temperature", "%g°C", temp);
	aoview_table_add_row("Battery", "%gV", battery);
	aoview_table_add_row("Drogue", "%gV", drogue_sense);
	aoview_table_add_row("Main", "%gV", main_sense);
	aoview_table_add_row("Pad altitude", "%dm", aoview_pres_to_altitude(pad_pres));
	aoview_table_add_row("Satellites", "%d", state->nsat);
	if (state->locked) {
		aoview_state_add_deg("Latitude", state->lat);
		aoview_state_add_deg("Longitude", state->lon);
		aoview_table_add_row("GPS alt", "%d", state->alt);
		aoview_table_add_row("GPS time", "%02d:%02d:%02d",
				     state->gps_time.hour,
				     state->gps_time.minute,
				     state->gps_time.second);
		aoview_great_circle(pad_lat, pad_lon, state->lat, state->lon,
				    &dist, &bearing);
		aoview_table_add_row("Distance from pad", "%gm", dist * 1000);
		aoview_table_add_row("Direction from pad", "%g°", bearing);
	} else {
		aoview_table_add_row("GPS", "unlocked");
	}
	aoview_table_finish();
}

void
aoview_state_init(GladeXML *xml)
{
}
