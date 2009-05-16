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

static struct aoview_serial *monitor_serial;

#define MONITOR_LEN	1024

static char	monitor_line[MONITOR_LEN + 1];
static int	monitor_pos;

void
aoview_monitor_disconnect(void)
{
	if (monitor_serial) {
		aoview_serial_close(monitor_serial);
		monitor_serial = NULL;
	}
}

static void
aoview_parse_string(char *target, int len, char *source)
{
	strncpy(target, source, len-1);
	target[len-1] = '\0';
}

static void
aoview_parse_int(int *target, char *source)
{
	*target = strtol(source, NULL, 0);
}

static void
aoview_parse_pos(double *target, char *source)
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

static void
aoview_monitor_parse(char *line)
{
	char *saveptr;
	char *words[64];
	int nword;
	struct aostate	state;

	printf ("%s\n", line);
	for (nword = 0; nword < 64; nword++) {
		words[nword] = strtok_r(line, " \t\n", &saveptr);
		line = NULL;
		if (words[nword] == NULL)
			break;
	}
	if (nword < 26)
		return;
	if (strcmp(words[0], "CALL") != 0)
		return;
	aoview_parse_string(state.callsign, sizeof (state.callsign), words[1]);
	aoview_parse_int(&state.serial, words[3]);
	aoview_parse_int(&state.rssi, words[5]);
	aoview_parse_string(state.state, sizeof (state.state), words[9]);
	aoview_parse_int(&state.tick, words[10]);
	aoview_parse_int(&state.accel, words[12]);
	aoview_parse_int(&state.pres, words[14]);
	aoview_parse_int(&state.temp, words[16]);
	aoview_parse_int(&state.batt, words[18]);
	aoview_parse_int(&state.drogue, words[20]);
	aoview_parse_int(&state.main, words[22]);
	if (strcmp (words[26], "unlocked") != 0 && nword >= 29) {
		sscanf(words[26], "%d:%d:%d", &state.gps_time.hour, &state.gps_time.minute, &state.gps_time.second);
		aoview_parse_pos(&state.lat, words[27]);
		aoview_parse_pos(&state.lon, words[28]);
		sscanf(words[29], "%dm", &state.alt);
	} else {
		state.gps_time.hour = state.gps_time.minute = state.gps_time.second = 0;
		state.lat = state.lon = 0;
		state.alt = 0;
	}
	aoview_state_notify(&state);
}

static gboolean
aoview_monitor_callback(void *user_data)
{
	int	c;

	if (!monitor_serial)
		return FALSE;

	for (;;) {
		c = aoview_serial_getc(monitor_serial);
		if (c == -1)
			break;
		if (c == '\r')
			continue;
		if (c == '\n') {
			monitor_line[monitor_pos] = '\0';
			if (monitor_pos)
			aoview_monitor_parse(monitor_line);
			monitor_pos = 0;
		} else if (monitor_pos < MONITOR_LEN)
			monitor_line[monitor_pos++] = c;
	}
	return TRUE;
}

void
aoview_monitor_connect(char *tty)
{
	aoview_monitor_disconnect();
	monitor_serial = aoview_serial_open(tty);
	aoview_serial_set_callback(monitor_serial,
				   aoview_monitor_callback,
				   monitor_serial,
				   NULL);
}
