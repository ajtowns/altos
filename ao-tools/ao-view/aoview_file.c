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

char *aoview_file_dir;

#define ALTOS_DIR_PATH	"/apps/aoview/log_dir"
#define DEFAULT_DIR	"AltOS"

struct aoview_file {
	char	*ext;
	FILE	*file;
	char	*name;
	int	failed;
	int	serial;
	int	flight;
	int	sequence;
};

static void
aoview_file_save_conf(void)
{
	GConfClient	*gconf_client;

	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		gconf_client_set_string(gconf_client,
					ALTOS_DIR_PATH,
					aoview_file_dir,
					NULL);
		g_object_unref(G_OBJECT(gconf_client));
	}
}

static void
aoview_file_configure(GtkWidget *widget, gpointer data)
{
	GtkFileChooser *chooser = data;
	aoview_file_dir = gtk_file_chooser_get_filename(chooser);
	aoview_file_save_conf();
	gtk_widget_hide(GTK_WIDGET(chooser));
}

void
aoview_file_finish(struct aoview_file *file)
{
	if (file->file) {
		fclose(file->file);
		file->file = NULL;
		free(file->name);
		file->name = NULL;
	}
	file->failed = 0;
}

const char *
aoview_file_name(struct aoview_file *file)
{
	return file->name;
}

static GtkMessageDialog *file_fail_dialog;

static void
aoview_file_open_failed(char *name)
{
	char	*utf8_file;
	utf8_file = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
	if (!utf8_file)
		utf8_file = name;
	gtk_message_dialog_format_secondary_text(file_fail_dialog,
						 "\"%s\"", utf8_file);
	if (utf8_file != name)
		g_free(utf8_file);
	gtk_widget_show(GTK_WIDGET(file_fail_dialog));
}

gboolean
aoview_file_start(struct aoview_file *file)
{
	char		base[50];
	char		seq[20];
	struct tm	tm;
	time_t		now;
	char		*full;
	int		r;

	if (file->file)
		return TRUE;

	if (file->failed)
		return FALSE;

	full = cc_make_filename(file->serial, file->flight, file->ext);
	file->file = fopen(full, "w");
	if (!file->file) {
		aoview_file_open_failed(full);
		free(full);
		file->failed = 1;
		return FALSE;
	} else {
		setlinebuf(file->file);
		file->name = full;
		return TRUE;
	}
}

void
aoview_file_vprintf(struct aoview_file *file, char *format, va_list ap)
{
	if (!aoview_file_start(file))
		return;
	vfprintf(file->file, format, ap);
}

void
aoview_file_printf(struct aoview_file *file, char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	aoview_file_vprintf(file, format, ap);
	va_end(ap);
}

struct aoview_file *
aoview_file_new(char *ext)
{
	struct aoview_file	*file;

	file = calloc (1, sizeof (struct aoview_file));
	if (!file)
		return NULL;
	file->ext = strdup(ext);
	if (!file->ext) {
		free(file);
		return NULL;
	}
	return file;
}

void
aoview_file_destroy(struct aoview_file *file)
{
	if (file->file)
		fclose(file->file);
	if (file->name)
		free(file->name);
	free(file->ext);
	free(file);
}

void
aoview_file_set_serial(struct aoview_file *file, int serial)
{
	if (serial != file->serial)
		aoview_file_finish(file);
	file->serial = serial;
}

int
aoview_file_get_serial(struct aoview_file *file)
{
	return file->serial;
}

void
aoview_file_set_flight(struct aoview_file *file, int flight)
{
	if (flight != file->flight)
		aoview_file_finish(file);
	file->flight = flight;
}

int
aoview_file_get_flight(struct aoview_file *file)
{
	return file->flight;
}

void
aoview_file_init(GladeXML *xml)
{
	GConfClient	*gconf_client;
	char		*file_dir = NULL;
	GtkFileChooser	*file_chooser_dialog;
	GtkWidget	*file_configure_ok;

	g_type_init();
	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		file_dir = gconf_client_get_string(gconf_client,
						   ALTOS_DIR_PATH,
						   NULL);
		g_object_unref(G_OBJECT(gconf_client));
	}
	if (!file_dir) {
		aoview_file_dir = aoview_fullname(getenv("HOME"), DEFAULT_DIR);
		aoview_file_save_conf();
	} else {
		aoview_file_dir = strdup(file_dir);
	}

	file_chooser_dialog = GTK_FILE_CHOOSER(glade_xml_get_widget(xml, "file_chooser_dialog"));
	assert(file_chooser_dialog);
	gtk_file_chooser_set_filename(file_chooser_dialog, aoview_file_dir);

	file_configure_ok = glade_xml_get_widget(xml, "file_configure_ok");
	assert(file_configure_ok);

	g_signal_connect(G_OBJECT(file_configure_ok), "clicked",
			 G_CALLBACK(aoview_file_configure),
			 file_chooser_dialog);


	file_fail_dialog = GTK_MESSAGE_DIALOG(glade_xml_get_widget(xml, "file_fail_dialog"));
	assert(file_fail_dialog);
}
