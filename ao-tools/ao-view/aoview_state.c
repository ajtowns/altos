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
aoview_state_add_deg(int column, char *label, double deg, char pos, char neg)
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
	aoview_table_add_row(column, label, "%d°%lf'%c",
			     (int) int_part, min, sign);

}

static char *ascent_states[] = {
	"boost",
	"fast",
	"coast",
	0,
};

static double
aoview_time(void)
{
	struct timespec	now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (double) now.tv_sec + (double) now.tv_nsec / 1.0e9;
}

/*
 * Fill out the derived data fields
 */
static void
aoview_state_derive(struct cc_telem *data, struct aostate *state)
{
	int	i;
	double	new_height;
	double	height_change;
	double	time_change;
	double	accel_counts_per_mss;
	int	tick_count;

	state->report_time = aoview_time();

	state->prev_data = state->data;
	state->prev_npad = state->npad;
	state->data = *data;
	tick_count = data->tick;
	if (tick_count < state->prev_data.tick)
		tick_count += 65536;
	time_change = (tick_count - state->prev_data.tick) / 100.0;

	state->ground_altitude = aoview_pres_to_altitude(data->ground_pres);
	new_height = aoview_pres_to_altitude(data->flight_pres) - state->ground_altitude;
	height_change = new_height - state->height;
	state->height = new_height;
	if (time_change)
		state->baro_speed = (state->baro_speed * 3 + (height_change / time_change)) / 4.0;
	accel_counts_per_mss = ((data->accel_minus_g - data->accel_plus_g) / 2.0) / 9.80665;
	state->acceleration = (data->ground_accel - data->flight_accel) / accel_counts_per_mss;
	state->speed = data->flight_vel / (accel_counts_per_mss * 100.0);
	state->temperature = cc_thermometer_to_temperature(data->temp);
	state->drogue_sense = cc_ignitor_to_voltage(data->drogue);
	state->main_sense = cc_ignitor_to_voltage(data->main);
	state->battery = cc_battery_to_voltage(data->batt);
	if (!strcmp(data->state, "pad")) {
		if (data->gps.gps_locked && data->gps.nsat >= 4) {
			state->npad++;
			state->pad_lat_total += data->gps.lat;
			state->pad_lon_total += data->gps.lon;
			state->pad_alt_total += data->gps.alt;
			if (state->npad > 1) {
				state->pad_lat = (state->pad_lat * 31 + data->gps.lat) / 32.0;
				state->pad_lon = (state->pad_lon * 31 + data->gps.lon) / 32.0;
				state->pad_alt = (state->pad_alt * 31 + data->gps.alt) / 32.0;
			} else {
				state->pad_lat = data->gps.lat;
				state->pad_lon = data->gps.lon;
				state->pad_alt = data->gps.alt;
			}
		}
	}
	state->ascent = FALSE;
	for (i = 0; ascent_states[i]; i++)
		if (!strcmp(data->state, ascent_states[i]))
			state->ascent = TRUE;

	/* Only look at accelerometer data on the way up */
	if (state->ascent && state->acceleration > state->max_acceleration)
		state->max_acceleration = state->acceleration;
	if (state->ascent && state->speed > state->max_speed)
		state->max_speed = state->speed;

	if (state->height > state->max_height)
		state->max_height = state->height;
	state->gps.gps_locked = data->gps.gps_locked;
	state->gps.gps_connected = data->gps.gps_connected;
	if (data->gps.gps_locked) {
		state->gps = data->gps;
		state->gps_valid = 1;
		if (state->npad)
			aoview_great_circle(state->pad_lat, state->pad_lon, state->gps.lat, state->gps.lon,
					    &state->distance, &state->bearing);
	}
	if (data->gps_tracking.channels)
		state->gps_tracking = data->gps_tracking;
	if (state->npad) {
		state->gps_height = state->gps.alt - state->pad_alt;
	} else {
		state->gps_height = 0;
	}
}

void
aoview_speak_state(struct aostate *state)
{
	if (strcmp(state->data.state, state->prev_data.state)) {
		aoview_voice_speak("%s\n", state->data.state);
		if (!strcmp(state->data.state, "drogue"))
			aoview_voice_speak("apogee %d meters\n",
					   (int) state->max_height);
		if (!strcmp(state->prev_data.state, "boost"))
			aoview_voice_speak("max speed %d meters per second\n",
					   (int) state->max_speed);
	}
	if (state->prev_npad < MIN_PAD_SAMPLES && state->npad >= MIN_PAD_SAMPLES)
		aoview_voice_speak("g p s ready\n");
}

void
aoview_speak_height(struct aostate *state)
{
	aoview_voice_speak("%d meters\n", state->height);
}

struct aostate aostate;

static guint aostate_timeout;

#define COMPASS_LIMIT(n)	((n * 22.5) + 22.5/2)

static char *compass_points[] = {
	"north",
	"north north east",
	"north east",
	"east north east",
	"east",
	"east south east",
	"south east",
	"south south east",
	"south",
	"south south west",
	"south west",
	"west south west",
	"west",
	"west north west",
	"north west",
	"north north west",
};

static char *
aoview_compass_point(double bearing)
{
	int	i;
	while (bearing < 0)
		bearing += 360.0;
	while (bearing >= 360.0)
		bearing -= 360.0;

	i = floor ((bearing - 22.5/2) / 22.5 + 0.5);
	if (i < 0) i = 0;
	if (i >= sizeof (compass_points) / sizeof (compass_points[0]))
		i = 0;
	return compass_points[i];
}

static gboolean
aoview_state_timeout(gpointer data)
{
	double	now = aoview_time();

	if (strlen(aostate.data.state) > 0 && strcmp(aostate.data.state, "pad") != 0)
		aoview_speak_height(&aostate);
	if (now - aostate.report_time >= 20 || !strcmp(aostate.data.state, "landed")) {
		if (!aostate.ascent) {
			if (fabs(aostate.baro_speed) < 20 && aostate.height < 100)
				aoview_voice_speak("rocket landed safely\n");
			else
				aoview_voice_speak("rocket may have crashed\n");
			if (aostate.gps_valid) {
				aoview_voice_speak("rocket reported %s of pad distance %d meters\n",
						   aoview_compass_point(aostate.bearing),
						   (int) aostate.distance);
			}
		}
		aostate_timeout = 0;
		return FALSE;
	}
	return TRUE;
}

void
aoview_state_reset(void)
{
	memset(&aostate, '\0', sizeof (aostate));
}

void
aoview_state_notify(struct cc_telem *data)
{
	struct aostate *state = &aostate;
	aoview_state_derive(data, state);
	aoview_table_start();

	if (state->npad >= MIN_PAD_SAMPLES)
		aoview_table_add_row(0, "Ground state", "ready");
	else
		aoview_table_add_row(0, "Ground state", "waiting for gps (%d)",
				     MIN_PAD_SAMPLES - state->npad);
	aoview_table_add_row(0, "Rocket state", "%s", state->data.state);
	aoview_table_add_row(0, "Callsign", "%s", state->data.callsign);
	aoview_table_add_row(0, "Rocket serial", "%d", state->data.serial);
	aoview_table_add_row(0, "Rocket flight", "%d", state->data.flight);

	aoview_table_add_row(0, "RSSI", "%6ddBm", state->data.rssi);
	aoview_table_add_row(0, "Height", "%6dm", state->height);
	aoview_table_add_row(0, "Max height", "%6dm", state->max_height);
	aoview_table_add_row(0, "Acceleration", "%7.1fm/s²", state->acceleration);
	aoview_table_add_row(0, "Max acceleration", "%7.1fm/s²", state->max_acceleration);
	aoview_table_add_row(0, "Speed", "%7.1fm/s", state->ascent ? state->speed : state->baro_speed);
	aoview_table_add_row(0, "Max Speed", "%7.1fm/s", state->max_speed);
	aoview_table_add_row(0, "Temperature", "%6.2f°C", state->temperature);
	aoview_table_add_row(0, "Battery", "%5.2fV", state->battery);
	aoview_table_add_row(0, "Drogue", "%5.2fV", state->drogue_sense);
	aoview_table_add_row(0, "Main", "%5.2fV", state->main_sense);
	aoview_table_add_row(0, "Pad altitude", "%dm", state->ground_altitude);
	aoview_table_add_row(1, "Satellites", "%d", state->gps.nsat);
	if (state->gps.gps_locked) {
		aoview_table_add_row(1, "GPS", "locked");
	} else if (state->gps.gps_connected) {
		aoview_table_add_row(1, "GPS", "unlocked");
	} else {
		aoview_table_add_row(1, "GPS", "not available");
	}
	if (state->gps_valid) {
		aoview_state_add_deg(1, "Latitude", state->gps.lat, 'N', 'S');
		aoview_state_add_deg(1, "Longitude", state->gps.lon, 'E', 'W');
		aoview_table_add_row(1, "GPS altitude", "%d", state->gps.alt);
		aoview_table_add_row(1, "GPS height", "%d", state->gps_height);
		aoview_table_add_row(1, "GPS date", "%04d-%02d-%02d",
				     state->gps.gps_time.year,
				     state->gps.gps_time.month,
				     state->gps.gps_time.day);
		aoview_table_add_row(1, "GPS time", "%02d:%02d:%02d",
				     state->gps.gps_time.hour,
				     state->gps.gps_time.minute,
				     state->gps.gps_time.second);
	}
	if (state->gps.gps_extended) {
		aoview_table_add_row(1, "GPS ground speed", "%7.1fm/s %d°",
				     state->gps.ground_speed,
				     state->gps.course);
		aoview_table_add_row(1, "GPS climb rate", "%7.1fm/s",
				     state->gps.climb_rate);
		aoview_table_add_row(1, "GPS precision", "%4.1f(hdop) %3dm(h) %3dm(v)",
				     state->gps.hdop, state->gps.h_error, state->gps.v_error);
	}
	if (state->npad) {
		aoview_table_add_row(1, "Distance from pad", "%5.0fm", state->distance);
		aoview_table_add_row(1, "Direction from pad", "%4.0f°", state->bearing);
		aoview_state_add_deg(1, "Pad latitude", state->pad_lat, 'N', 'S');
		aoview_state_add_deg(1, "Pad longitude", state->pad_lon, 'E', 'W');
		aoview_table_add_row(1, "Pad GPS alt", "%gm", state->pad_alt);
	}
	if (state->gps.gps_connected) {
		int	nsat_vis = 0;
		int	c;

		aoview_table_add_row(2, "Satellites Visible", "%d", state->gps_tracking.channels);
		for (c = 0; c < state->gps_tracking.channels; c++) {
			aoview_table_add_row(2, "Satellite id,C/N0",
					     "%3d,%2d",
					     state->gps_tracking.sats[c].svid,
					     state->gps_tracking.sats[c].c_n0);
		}
	}
	aoview_table_finish();
	aoview_label_show(state);
	aoview_speak_state(state);
	if (!aostate_timeout && strcmp(state->data.state, "pad") != 0)
		aostate_timeout = g_timeout_add_seconds(10, aoview_state_timeout, NULL);
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
