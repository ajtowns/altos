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

#include "cc.h"
#include <string.h>
#include <stdlib.h>

static void
cc_parse_string(char *target, int len, char *source)
{
	strncpy(target, source, len-1);
	target[len-1] = '\0';
}

static void
cc_parse_int(int *target, char *source)
{
	*target = strtol(source, NULL, 0);
}

static void
cc_parse_hex(int *target, char *source)
{
	*target = strtol(source, NULL, 16);
}

static void
cc_parse_pos(double *target, char *source)
{
	int	deg;
	double	min;
	char	dir;
	double	r;

	if (sscanf(source, "%d°%lf'%c", &deg, &min, &dir) != 3) {
		*target = 0;
		return;
	}
	r = deg + min / 60.0;
	if (dir == 'S' || dir == 'W')
		r = -r;
	*target = r;
}

#define PARSE_MAX_WORDS	512

int
cc_telem_parse(const char *input_line, struct cc_telem *telem)
{
	char *saveptr;
	char *raw_words[PARSE_MAX_WORDS];
	char **words;
	int version = 0;
	int nword;
	char line_buf[8192], *line;
	int	tracking_pos;

	/* avoid smashing our input parameter */
	strncpy (line_buf, input_line, sizeof (line_buf)-1);
	line_buf[sizeof(line_buf) - 1] = '\0';
	line = line_buf;
	for (nword = 0; nword < PARSE_MAX_WORDS; nword++) {
		raw_words[nword] = strtok_r(line, " \t\n", &saveptr);
		line = NULL;
		if (raw_words[nword] == NULL)
			break;
	}
	if (nword < 36)
		return FALSE;
	words = raw_words;
	if (strcmp(words[0], "VERSION") == 0) {
		cc_parse_int(&version, words[1]);
		words += 2;
		nword -= 2;
	}

	if (strcmp(words[0], "CALL") != 0)
		return FALSE;
	cc_parse_string(telem->callsign, sizeof (telem->callsign), words[1]);
	cc_parse_int(&telem->serial, words[3]);

	if (version >= 2) {
		cc_parse_int(&telem->flight, words[5]);
		words += 2;
		nword -= 2;
	} else
		telem->flight = 0;

	cc_parse_int(&telem->rssi, words[5]);
	if (version <= 2) {
		/* Older telemetry versions mis-computed the rssi value */
		telem->rssi = (telem->rssi + 74) / 2 - 74;
	}
	cc_parse_string(telem->state, sizeof (telem->state), words[9]);
	cc_parse_int(&telem->tick, words[10]);
	cc_parse_int(&telem->accel, words[12]);
	cc_parse_int(&telem->pres, words[14]);
	cc_parse_int(&telem->temp, words[16]);
	cc_parse_int(&telem->batt, words[18]);
	cc_parse_int(&telem->drogue, words[20]);
	cc_parse_int(&telem->main, words[22]);
	cc_parse_int(&telem->flight_accel, words[24]);
	cc_parse_int(&telem->ground_accel, words[26]);
	cc_parse_int(&telem->flight_vel, words[28]);
	cc_parse_int(&telem->flight_pres, words[30]);
	cc_parse_int(&telem->ground_pres, words[32]);
	if (version >= 1) {
		cc_parse_int(&telem->accel_plus_g, words[34]);
		cc_parse_int(&telem->accel_minus_g, words[36]);
		words += 4;
		nword -= 4;
	} else {
		telem->accel_plus_g = telem->ground_accel;
		telem->accel_minus_g = telem->ground_accel + 530;
	}
	cc_parse_int(&telem->gps.nsat, words[34]);
	if (strcmp (words[36], "unlocked") == 0) {
		telem->gps.gps_connected = 1;
		telem->gps.gps_locked = 0;
		telem->gps.gps_time.year = telem->gps.gps_time.month = telem->gps.gps_time.day = 0;
		telem->gps.gps_time.hour = telem->gps.gps_time.minute = telem->gps.gps_time.second = 0;
		telem->gps.lat = telem->gps.lon = 0;
		telem->gps.alt = 0;
		tracking_pos = 37;
	} else if (nword >= 40) {
		telem->gps.gps_locked = 1;
		telem->gps.gps_connected = 1;
		if (version >= 2) {
			sscanf(words[36], "%d-%d-%d",
			       &telem->gps.gps_time.year,
			       &telem->gps.gps_time.month,
			       &telem->gps.gps_time.day);
			words += 1;
			nword -= 1;
		} else {
			telem->gps.gps_time.year = telem->gps.gps_time.month = telem->gps.gps_time.day = 0;
		}
		sscanf(words[36], "%d:%d:%d", &telem->gps.gps_time.hour, &telem->gps.gps_time.minute, &telem->gps.gps_time.second);
		cc_parse_pos(&telem->gps.lat, words[37]);
		cc_parse_pos(&telem->gps.lon, words[38]);
		sscanf(words[39], "%dm", &telem->gps.alt);
		tracking_pos = 46;
	} else {
		telem->gps.gps_connected = 0;
		telem->gps.gps_locked = 0;
		telem->gps.gps_time.year = telem->gps.gps_time.month = telem->gps.gps_time.day = 0;
		telem->gps.gps_time.hour = telem->gps.gps_time.minute = telem->gps.gps_time.second = 0;
		telem->gps.lat = telem->gps.lon = 0;
		telem->gps.alt = 0;
		tracking_pos = -1;
	}
	if (nword >= 46) {
		telem->gps.gps_extended = 1;
		sscanf(words[40], "%lfm/s", &telem->gps.ground_speed);
		sscanf(words[41], "%d", &telem->gps.course);
		sscanf(words[42], "%lfm/s", &telem->gps.climb_rate);
		sscanf(words[43], "%lf", &telem->gps.hdop);
		sscanf(words[44], "%d", &telem->gps.h_error);
		sscanf(words[45], "%d", &telem->gps.v_error);
	} else {
		telem->gps.gps_extended = 0;
		telem->gps.ground_speed = 0;
		telem->gps.course = 0;
		telem->gps.climb_rate = 0;
		telem->gps.hdop = 0;
		telem->gps.h_error = 0;
		telem->gps.v_error = 0;
	}
	if (tracking_pos >= 0 && nword >= tracking_pos + 2 && strcmp(words[tracking_pos], "SAT") == 0) {
		int	c, n, pos;
		int	per_sat;
		int	state;

		if (version >= 2)
			per_sat = 2;
		else
			per_sat = 3;
		cc_parse_int(&n, words[tracking_pos + 1]);
		pos = tracking_pos + 2;
		if (nword >= pos + n * per_sat) {
			telem->gps_tracking.channels = n;
			for (c = 0; c < n; c++) {
				cc_parse_int(&telem->gps_tracking.sats[c].svid,
						 words[pos + 0]);
				if (version < 2)
					cc_parse_hex(&state, words[pos + 1]);
				cc_parse_int(&telem->gps_tracking.sats[c].c_n0,
						 words[pos + per_sat - 1]);
				pos += per_sat;
			}
		} else {
			telem->gps_tracking.channels = 0;
		}
	} else {
		telem->gps_tracking.channels = 0;
	}
	return TRUE;
}
