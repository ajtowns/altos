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

#define EEPROM_LEN	1024

static struct aoview_file	*eeprom_file;
static char			eeprom_line[EEPROM_LEN + 1];
static int			eeprom_pos;
static GtkMessageDialog		*eeprom_save_done;
static GtkWidget		*eeprom_save_close;
static gboolean			eeprom_save_shown;

static void
aoview_eeprom_disconnect(struct aoview_serial *serial)
{
	aoview_file_finish(eeprom_file);
}

static void
aoview_eeprom_done(struct aoview_serial *serial)
{
	gtk_window_set_title(GTK_WINDOW(eeprom_save_done),
			     "EEPROM data saved");
	gtk_message_dialog_set_markup(eeprom_save_done,
				      "<b>EEPROM data saved as</b>");
	if (!eeprom_save_shown)
		gtk_widget_show(GTK_WIDGET(eeprom_save_done));
	eeprom_save_close = gtk_window_get_default_widget(GTK_WINDOW(eeprom_save_done));
	if (eeprom_save_close)
		gtk_widget_set_sensitive(eeprom_save_close, TRUE);
	aoview_eeprom_disconnect(serial);
}

static gboolean
aoview_eeprom_parse(struct aoview_serial *serial,
		    char *line)
{
	char		cmd;
	int		tick;
	int		a;
	int		b;
	int		serial_number;
	const char	*name;
	char		*utf8_name;

	if (!strcmp(line, "end")) {
		aoview_eeprom_done(serial);
		return FALSE;
	}
	if (sscanf(line, "serial-number %u", &serial_number) == 1) {
		aoview_file_set_serial(eeprom_file, serial_number);
	} else if (sscanf(line, "%c %x %x %x", &cmd, &tick, &a, &b) == 4) {
		if (cmd == 'F')
			aoview_file_set_flight(eeprom_file, b);
		aoview_file_printf(eeprom_file, "%s\n", line);
		if (cmd == 'S' && a == 8) {
			aoview_eeprom_done(serial);
			return FALSE;
		}

		if (!eeprom_save_shown)
		{
			name = aoview_file_name(eeprom_file);
			if (name) {
				utf8_name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
				if (!utf8_name)
					utf8_name = (char *) name;
				gtk_widget_set_sensitive(eeprom_save_close, FALSE);
				gtk_window_set_title(GTK_WINDOW(eeprom_save_done),
						     "Saving EEPROM data");
				gtk_message_dialog_set_markup(eeprom_save_done,
							      "<b>Saving EEPROM data as</b>");
				gtk_message_dialog_format_secondary_text(eeprom_save_done, "%s",
									 utf8_name);
				if (utf8_name != name)
					g_free(utf8_name);
				gtk_container_check_resize(GTK_CONTAINER(eeprom_save_done));
				gtk_widget_show(GTK_WIDGET(eeprom_save_done));
				eeprom_save_shown = TRUE;
				eeprom_save_close = gtk_window_get_default_widget(GTK_WINDOW(eeprom_save_done));
				if (eeprom_save_close)
					gtk_widget_set_sensitive(eeprom_save_close, FALSE);
			}
		}
	}
	return TRUE;
}

static void
aoview_eeprom_callback(gpointer user_data,
		       struct aoview_serial *serial,
		       gint revents)
{
	int	c;

	if (revents & (G_IO_HUP|G_IO_ERR)) {
		aoview_eeprom_disconnect(serial);
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
				eeprom_line[eeprom_pos] = '\0';
				if (eeprom_pos)
				if (!aoview_eeprom_parse(serial, eeprom_line))
					break;
				eeprom_pos = 0;
			} else if (eeprom_pos < EEPROM_LEN)
				eeprom_line[eeprom_pos++] = c;
		}
	}
}

gboolean
aoview_eeprom_save(const char *device)
{
	struct aoview_serial	*serial;

	gtk_widget_hide(GTK_WIDGET(eeprom_save_done));
	eeprom_save_shown = FALSE;
	serial = aoview_serial_open(device);
	if (!serial)
		return FALSE;
	aoview_serial_set_callback(serial, aoview_eeprom_callback);
	aoview_serial_printf(serial, "v\nl\n");
	return TRUE;
}

void
aoview_eeprom_init(GladeXML *xml)
{
	eeprom_file = aoview_file_new("eeprom");
	assert(eeprom_file);

	eeprom_save_done = GTK_MESSAGE_DIALOG(glade_xml_get_widget(xml, "ao_save_done"));
	assert(eeprom_save_done);

}
