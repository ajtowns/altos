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
	aoview_log_new();
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

static struct aostate	state;

gboolean
aoview_monitor_parse(const char *input_line)
{
	char *saveptr;
	char *words[64];
	int nword;
	char line_buf[8192], *line;

	/* avoid smashing our input parameter */
	strncpy (line_buf, input_line, sizeof (line_buf)-1);
	line_buf[sizeof(line_buf) - 1] = '\0';
	line = line_buf;
	for (nword = 0; nword < 64; nword++) {
		words[nword] = strtok_r(line, " \t\n", &saveptr);
		line = NULL;
		if (words[nword] == NULL)
			break;
	}
	if (nword < 36)
		return FALSE;
	if (strcmp(words[0], "CALL") != 0)
		return FALSE;
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
	aoview_parse_int(&state.flight_accel, words[24]);
	aoview_parse_int(&state.ground_accel, words[26]);
	aoview_parse_int(&state.flight_vel, words[28]);
	aoview_parse_int(&state.flight_pres, words[30]);
	aoview_parse_int(&state.ground_pres, words[32]);
	aoview_parse_int(&state.nsat, words[34]);
	if (strcmp (words[36], "unlocked") != 0 && nword >= 40) {
		state.locked = 1;
		sscanf(words[36], "%d:%d:%d", &state.gps_time.hour, &state.gps_time.minute, &state.gps_time.second);
		aoview_parse_pos(&state.lat, words[37]);
		aoview_parse_pos(&state.lon, words[38]);
		sscanf(words[39], "%dm", &state.alt);
	} else {
		state.locked = 0;
		state.gps_time.hour = state.gps_time.minute = state.gps_time.second = 0;
		state.lat = state.lon = 0;
		state.alt = 0;
	}
	if (nword >= 46) {
		sscanf(words[40], "%lfm/s", &state.ground_speed);
		sscanf(words[41], "%d", &state.course);
		sscanf(words[42], "%lfm/s", &state.climb_rate);
		sscanf(words[43], "%lf", &state.hdop);
		sscanf(words[44], "%d", &state.h_error);
		sscanf(words[45], "%d", &state.v_error);
	} else {
		state.ground_speed = 0;
		state.course = 0;
		state.climb_rate = 0;
		state.hdop = 0;
		state.h_error = 0;
		state.v_error = 0;
	}
	aoview_state_notify(&state);
	return TRUE;
}

void
aoview_monitor_reset(void)
{
	memset(&state, '\0', sizeof (state));
}

static void
aoview_monitor_callback(gpointer user_data,
			struct aoview_serial *serial,
			gint revents)
{
	int	c;

	if (revents & (G_IO_HUP|G_IO_ERR)) {
		aoview_monitor_disconnect();
		return;
	}
	if (revents & G_IO_IN) {
		for (;;) {
			c = aoview_serial_getc(serial);
			if (c == -1)
				break;
			if (c == '\r')
				continue;
			if (c == '\n') {
				monitor_line[monitor_pos] = '\0';
				if (monitor_pos) {
					if (aoview_monitor_parse(monitor_line)) {
						aoview_log_set_serial(state.serial);
						if (aoview_log_get_serial())
							aoview_log_printf ("%s\n", monitor_line);
					}
				}
				monitor_pos = 0;
			} else if (monitor_pos < MONITOR_LEN)
				monitor_line[monitor_pos++] = c;
		}
	}
}

gboolean
aoview_monitor_connect(char *tty)
{
	aoview_monitor_disconnect();
	monitor_serial = aoview_serial_open(tty);
	if (!monitor_serial)
		return FALSE;
	aoview_table_clear();
	aoview_monitor_reset();
	aoview_serial_set_callback(monitor_serial,
				   aoview_monitor_callback,
				   monitor_serial,
				   NULL);
	return TRUE;
}
