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

static struct aoview_file	*aoview_log;

void
aoview_log_new(void)
{
	aoview_file_finish(aoview_log);
	aoview_state_new();
}

void
aoview_log_set_serial(int serial)
{
	aoview_file_set_serial(aoview_log, serial);
}

int
aoview_log_get_serial(void)
{
	return aoview_file_get_serial(aoview_log);
}

void
aoview_log_set_flight(int flight)
{
	aoview_file_set_flight(aoview_log, flight);
}

int
aoview_log_get_flight(void)
{
	return aoview_file_get_flight(aoview_log);
}

void
aoview_log_printf(char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	aoview_file_vprintf(aoview_log, format, ap);
	va_end(ap);
}

static void
aoview_log_new_item(GtkWidget *widget, gpointer data)
{
	aoview_file_finish(aoview_log);
}

void
aoview_log_init(GladeXML *xml)
{
	GtkWidget	*log_new;

	aoview_log = aoview_file_new("telem");
	assert(aoview_log);

	log_new = glade_xml_get_widget(xml, "log_new");
	assert(log_new);
	g_signal_connect(G_OBJECT(log_new), "activate",
			 G_CALLBACK(aoview_log_new_item),
			 NULL);
}
