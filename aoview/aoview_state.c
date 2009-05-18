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

static double	pad_lat_total;
static double	pad_lon_total;
static int	pad_alt_total;
static int	npad_gps;
static int	prev_tick;
static double	prev_accel;
static double	pad_lat;
static double	pad_lon;
static double	pad_alt;
static double	min_pres;
static double	min_accel;

#define NUM_PAD_SAMPLES	10

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
	int	ticks;
	double	dist;
	double	bearing;
	double	temp;
	double	velocity;
	double	battery;
	double	drogue_sense, main_sense;
	double	max_accel;

	if (!strcmp(state->state, "pad")) {
		if (state->locked && npad_gps < NUM_PAD_SAMPLES) {
			pad_lat_total += state->lat;
			pad_lon_total += state->lon;
			pad_alt_total += state->alt;
			npad_gps++;
		}
		if (state->locked && npad_gps <= NUM_PAD_SAMPLES) {
			pad_lat = pad_lat_total / npad_gps;
			pad_lon = pad_lon_total / npad_gps;
			pad_alt = pad_alt_total / npad_gps;
		}
		min_pres = state->ground_pres;
		min_accel = state->ground_accel;
	}
	if (state->flight_pres < min_pres)
		min_pres = state->flight_pres;
	if (state->flight_accel < min_accel)
		min_accel = state->flight_accel;
	altitude = aoview_pres_to_altitude(state->flight_pres) - aoview_pres_to_altitude(state->ground_pres);
	accel = (state->ground_accel - state->flight_accel) / 27.0;
	velocity = state->flight_vel / 2700.0;
	max_accel = (state->ground_accel - min_accel) / 27.0;
	ticks = state->tick - prev_tick;
	temp = ((state->temp / 32767.0 * 3.3) - 0.5) / 0.01;
	battery = (state->batt / 32767.0 * 5.0);
	drogue_sense = (state->drogue / 32767.0 * 15.0);
	main_sense = (state->main / 32767.0 * 15.0);

	prev_accel = accel;
	prev_tick = state->tick;
	aoview_table_start();

	if (npad_gps >= NUM_PAD_SAMPLES)
		aoview_table_add_row("Ground state", "ready");
	else
		aoview_table_add_row("Ground state", "waiting for gps (%d)",
				     NUM_PAD_SAMPLES - npad_gps);
	aoview_table_add_row("Rocket state", "%s", state->state);
	aoview_table_add_row("Callsign", "%s", state->callsign);
	aoview_table_add_row("Rocket serial", "%d", state->serial);

	aoview_table_add_row("RSSI", "%ddBm", state->rssi);
	aoview_table_add_row("Height", "%dm", altitude);
	aoview_table_add_row("Max height", "%dm",
			     aoview_pres_to_altitude(min_pres) -
			     aoview_pres_to_altitude(state->ground_pres));
	aoview_table_add_row("Acceleration", "%gm/s²", accel);
	aoview_table_add_row("Max acceleration", "%gm/s²", max_accel);
	aoview_table_add_row("Velocity", "%gm/s", velocity);
	aoview_table_add_row("Temperature", "%g°C", temp);
	aoview_table_add_row("Battery", "%gV", battery);
	aoview_table_add_row("Drogue", "%gV", drogue_sense);
	aoview_table_add_row("Main", "%gV", main_sense);
	aoview_table_add_row("Pad altitude", "%dm", aoview_pres_to_altitude(state->ground_pres));
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
	if (npad_gps) {
		aoview_state_add_deg("Pad latitude", pad_lat);
		aoview_state_add_deg("Pad longitude", pad_lon);
		aoview_table_add_row("Pad GPS alt", "%gm", pad_alt);
	}
	aoview_table_finish();
}

void
aoview_state_new(void)
{
	pad_lat_total = 0;
	pad_lon_total = 0;
	pad_alt_total = 0;
	npad_gps = 0;
	prev_tick = 0;
	prev_accel = 0;
	pad_lat = 0;
	pad_lon = 0;
	pad_alt = 0;
	min_pres = 32767;
	min_accel = 32767;
}

void
aoview_state_init(GladeXML *xml)
{
	aoview_state_new();
}
