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

static GtkFileChooser	*replay_dialog;
static GtkWidget	*replay_ok;
static FILE		*replay_file;
static int		replay_tick;

static int
find_tick(char *line, gboolean *is_pad)
{
	char	*state = strstr(line, "STATE");
	if (!state)
		return -1;
	state = strchr(state, ' ');
	if (!state)
		return -1;
	while (*state == ' ')
		state++;
	*is_pad = strncmp(state, "pad", 3) == 0;
	while (*state && !isdigit(*state))
		state++;
	return atoi(state);
}

static void
aoview_replay_close(void)
{
	if (replay_file) {
		fclose(replay_file);
		replay_file = NULL;
	}
}

static char	replay_line[1024];

static gboolean
aoview_replay_read(gpointer data);

static gboolean
aoview_replay_execute(gpointer data)
{
	aoview_monitor_parse(replay_line);
	g_idle_add(aoview_replay_read, NULL);
	return FALSE;
}

static gboolean
aoview_replay_read(gpointer data)
{
	int		tick;
	gboolean	is_pad;

	if (!replay_file)
		return FALSE;
	if (fgets(replay_line, sizeof (replay_line), replay_file)) {
		tick = find_tick(replay_line, &is_pad);
		if (tick >= 0 && replay_tick >= 0 && !is_pad) {
			while (tick < replay_tick)
				tick += 65536;
			g_timeout_add((tick - replay_tick) * 10,
				      aoview_replay_execute,
				      NULL);
		} else {
			aoview_replay_execute(NULL);
		}
		replay_tick = tick;
	} else {
		aoview_replay_close();
	}
	return FALSE;
}

static void
aoview_replay_open(GtkWidget *widget, gpointer data)
{
	char		*replay_file_name;
	GtkWidget	*dialog;

	aoview_replay_close();
	replay_file_name = gtk_file_chooser_get_filename(replay_dialog);
	replay_file = fopen(replay_file_name, "r");
	if (!replay_file) {
		dialog = gtk_message_dialog_new(GTK_WINDOW(replay_dialog),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						"Error loading file '%s': %s",
						replay_file_name, g_strerror(errno));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	} else {
		replay_tick = -1;
		aoview_state_reset();
		aoview_replay_read(NULL);
	}
	gtk_widget_hide(GTK_WIDGET(replay_dialog));
}

void
aoview_replay_init(GladeXML *xml)
{
	GtkFileFilter	*telem_filter;
	GtkFileFilter	*all_filter;
	GtkFileFilter	*log_filter;

	telem_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(telem_filter, "*.telem");
	gtk_file_filter_set_name(telem_filter, "Telemetry Files");

	log_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(log_filter, "*.log");
	gtk_file_filter_set_name(log_filter, "Log Files");

	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*");
	gtk_file_filter_set_name(all_filter, "All Files");

	replay_dialog = GTK_FILE_CHOOSER(glade_xml_get_widget(xml, "ao_replay_dialog"));
	assert(replay_dialog);
	gtk_file_chooser_set_current_folder(replay_dialog, aoview_file_dir);
	gtk_file_chooser_add_filter(replay_dialog, telem_filter);
	gtk_file_chooser_add_filter(replay_dialog, log_filter);
	gtk_file_chooser_add_filter(replay_dialog, all_filter);

	replay_ok = glade_xml_get_widget(xml, "ao_replay_ok");
	assert(replay_ok);
	g_signal_connect(G_OBJECT(replay_ok), "clicked",
			 G_CALLBACK(aoview_replay_open),
			 replay_dialog);
}
