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

static inline double sqr(double a) { return a * a; };

static void
aoview_great_circle (double start_lat, double start_lon,
		     double end_lat, double end_lon,
		     double *dist, double *bearing)
{
	const double rad = M_PI / 180;
	const double earth_radius = 6371.2 * 1000;	/* in meters */
	double lat1 = rad * start_lat;
	double lon1 = rad * -start_lon;
	double lat2 = rad * end_lat;
	double lon2 = rad * -end_lon;

	double d_lat = lat2 - lat1;
	double d_lon = lon2 - lon1;

	/* From http://en.wikipedia.org/wiki/Great-circle_distance */
	double vdn = sqrt(sqr(cos(lat2) * sin(d_lon)) +
			  sqr(cos(lat1) * sin(lat2) -
			      sin(lat1) * cos(lat2) * cos(d_lon)));
	double vdd = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(d_lon);
	double d = atan2(vdn,vdd);
	double course;

	if (cos(lat1) < 1e-20) {
		if (lat1 > 0)
			course = M_PI;
		else
			course = -M_PI;
	} else {
		if (d < 1e-10)
			course = 0;
		else
			course = acos((sin(lat2)-sin(lat1)*cos(d)) /
				      (sin(d)*cos(lat1)));
		if (sin(lon2-lon1) > 0)
			course = 2 * M_PI-course;
	}
	*dist = d * earth_radius;
	*bearing = course * 180/M_PI;
}

static void
aoview_state_add_deg(char *label, double deg, char pos, char neg)
{
	double	int_part;
	double	min;
	char	sign = pos;

	if (deg < 0) {
		deg = -deg;
		sign = neg;
	}
	int_part = floor (deg);
	min = (deg - int_part) * 60.0;
	aoview_table_add_row(label, "%d°%lf'%c",
			     (int) int_part, min, sign);

}

static char *ascent_states[] = {
	"boost",
	"fast",
	"coast",
	0,
};

/*
 * Fill out the derived data fields
 */
static void
aoview_state_derive(struct aostate *state)
{
	int	i;

	state->ground_altitude = aoview_pres_to_altitude(state->ground_pres);
	state->height = aoview_pres_to_altitude(state->flight_pres) - state->ground_altitude;
	state->acceleration = (state->ground_accel - state->flight_accel) / 27.0;
	state->speed = state->flight_vel / 2700.0;
	state->temperature = ((state->temp / 32767.0 * 3.3) - 0.5) / 0.01;
	state->drogue_sense = state->drogue / 32767.0 * 15.0;
	state->main_sense = state->main / 32767.0 * 15.0;
	state->battery = state->batt / 32767.0 * 5.0;
	if (!strcmp(state->state, "pad")) {
		if (state->locked && state->nsat > 4) {
			state->npad++;
			state->pad_lat_total += state->lat;
			state->pad_lon_total += state->lon;
			state->pad_alt_total += state->alt;
			state->pad_lat = state->pad_lat_total / state->npad;
			state->pad_lon = state->pad_lon_total / state->npad;
			state->pad_alt = state->pad_alt_total / state->npad;
		}
	}
	state->ascent = FALSE;
	for (i = 0; ascent_states[i]; i++)
		if (!strcmp(state->state, ascent_states[i]))
			state->ascent = TRUE;

	/* Only look at accelerometer data on the way up */
	if (state->ascent && state->acceleration > state->max_acceleration)
		state->max_acceleration = state->acceleration;
	if (state->ascent && state->speed > state->max_speed)
		state->max_speed = state->speed;

	if (state->height > state->max_height)
		state->max_height = state->height;
	aoview_great_circle(state->pad_lat, state->pad_lon, state->lat, state->lon,
			    &state->distance, &state->bearing);
	if (state->npad) {
		state->gps_height = state->alt - state->pad_alt;
	} else {
		state->gps_height = 0;
	}
}

void
aoview_state_speak(struct aostate *state)
{
	static char	last_state[32];
	int		i;
	gboolean	report = FALSE;
	int		this_tick;
	static int	last_tick;
	static int	last_altitude;
	int		this_altitude;

	if (strcmp(state->state, last_state)) {
		aoview_voice_speak("%s\n", state->state);
		if (!strcmp(state->state, "drogue"))
			aoview_voice_speak("apogee %d meters\n",
					   (int) state->max_height);
		report = TRUE;
		strcpy(last_state, state->state);
	}
	this_altitude = aoview_pres_to_altitude(state->flight_pres) - aoview_pres_to_altitude(state->ground_pres);
	this_tick = state->tick;
	while (this_tick < last_tick)
		this_tick += 65536;
	if (strcmp(state->state, "pad") != 0) {
		if (this_altitude / 1000 != last_altitude / 1000)
			report = TRUE;
		if (this_tick - last_tick >= 10 * 100)
			report = TRUE;
	}
	if (report) {
		aoview_voice_speak("%d meters\n",
				   this_altitude);
		if (state->ascent)
			aoview_voice_speak("%d meters per second\n",
					   state->flight_vel / 2700);
		last_tick = state->tick;
		last_altitude = this_altitude;
	}
}

void
aoview_state_notify(struct aostate *state)
{
	aoview_state_derive(state);
	aoview_table_start();

	if (state->npad >= MIN_PAD_SAMPLES)
		aoview_table_add_row("Ground state", "ready");
	else
		aoview_table_add_row("Ground state", "waiting for gps (%d)",
				     MIN_PAD_SAMPLES - state->npad);
	aoview_table_add_row("Rocket state", "%s", state->state);
	aoview_table_add_row("Callsign", "%s", state->callsign);
	aoview_table_add_row("Rocket serial", "%d", state->serial);

	aoview_table_add_row("RSSI", "%ddBm", state->rssi);
	aoview_table_add_row("Height", "%dm", state->height);
	aoview_table_add_row("Max height", "%dm", state->max_height);
	aoview_table_add_row("Acceleration", "%gm/s²", state->acceleration);
	aoview_table_add_row("Max acceleration", "%gm/s²", state->max_acceleration);
	aoview_table_add_row("Speed", "%gm/s", state->speed);
	aoview_table_add_row("Max Speed", "%gm/s", state->max_speed);
	aoview_table_add_row("Temperature", "%g°C", state->temperature);
	aoview_table_add_row("Battery", "%gV", state->battery);
	aoview_table_add_row("Drogue", "%gV", state->drogue_sense);
	aoview_table_add_row("Main", "%gV", state->main_sense);
	aoview_table_add_row("Pad altitude", "%dm", state->ground_altitude);
	aoview_table_add_row("Satellites", "%d", state->nsat);
	if (state->locked) {
		aoview_state_add_deg("Latitude", state->lat, 'N', 'S');
		aoview_state_add_deg("Longitude", state->lon, 'E', 'W');
		aoview_table_add_row("GPS height", "%d", state->gps_height);
		aoview_table_add_row("GPS time", "%02d:%02d:%02d",
				     state->gps_time.hour,
				     state->gps_time.minute,
				     state->gps_time.second);
		aoview_table_add_row("GPS ground speed", "%fm/s %d°",
				     state->ground_speed,
				     state->course);
		aoview_table_add_row("GPS climb rate", "%fm/s",
				     state->climb_rate);
		aoview_table_add_row("GPS precision", "%f(hdop) %dm(h) %dm(v)\n",
				     state->hdop, state->h_error, state->v_error);
		aoview_table_add_row("Distance from pad", "%gm", state->distance);
		aoview_table_add_row("Direction from pad", "%g°", state->bearing);
	} else {
		aoview_table_add_row("GPS", "unlocked");
	}
	if (state->npad) {
		aoview_state_add_deg("Pad latitude", state->pad_lat, 'N', 'S');
		aoview_state_add_deg("Pad longitude", state->pad_lon, 'E', 'W');
		aoview_table_add_row("Pad GPS alt", "%gm", state->pad_alt);
	}
	aoview_table_finish();
	aoview_label_show(state);
	aoview_state_speak(state);
}

void
aoview_state_new(void)
{
}

void
aoview_state_init(GladeXML *xml)
{
	aoview_state_new();
}
