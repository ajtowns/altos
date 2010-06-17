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

gboolean
aoview_monitor_parse(const char *input_line)
{
	struct cc_telem	telem;

	if (!cc_telem_parse(input_line, &telem))
		return FALSE;
	aoview_state_notify(&telem);
	return TRUE;
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
						aoview_log_set_serial(aostate.data.serial);
						aoview_log_set_flight(aostate.data.flight);
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

void
aoview_monitor_set_channel(int channel)
{
	if (monitor_serial) {
		aoview_serial_printf(monitor_serial, "m 0\n");
		aoview_serial_printf(monitor_serial, "c r %d\n", channel);
		aoview_serial_printf(monitor_serial, "m 1\n");
	}
}

gboolean
aoview_monitor_connect(char *tty)
{
	int	channel;
	aoview_monitor_disconnect();
	monitor_serial = aoview_serial_open(tty);
	if (!monitor_serial)
		return FALSE;
	aoview_table_clear();
	aoview_state_reset();
	channel = aoview_channel_current();
	aoview_monitor_set_channel(channel);
	aoview_serial_set_callback(monitor_serial,
				   aoview_monitor_callback);
	return TRUE;
}
