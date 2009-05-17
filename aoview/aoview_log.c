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

#define LOG_DIR_PATH	"/apps/aoview/log_dir"
#define DEFAULT_LOG	"AltOS"

static char *aoview_log_dir;
static FILE *aoview_log_file;
static int aoview_log_serial;
static int aoview_log_sequence;
static GtkMessageDialog *log_fail_dialog;
static int aoview_log_failed;

static void
aoview_log_save_conf(void)
{
	GConfClient	*gconf_client;

	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		gconf_client_set_string(gconf_client,
					LOG_DIR_PATH,
					aoview_log_dir,
					NULL);
		g_object_unref(G_OBJECT(gconf_client));
	}
}

static void
aoview_log_configure(GtkWidget *widget, gpointer data)
{
	GtkFileChooser *chooser = data;
	aoview_log_dir = gtk_file_chooser_get_filename(chooser);
	aoview_log_save_conf();
	gtk_widget_hide(GTK_WIDGET(chooser));
}

void
aoview_log_new(void)
{
	if (aoview_log_file) {
		fclose(aoview_log_file);
		aoview_log_file = NULL;
	}
	aoview_log_failed = 0;
	aoview_state_new();
}

static void
aoview_log_new_item(GtkWidget *widget, gpointer data)
{
	aoview_log_new();
}

void
aoview_log_set_serial(int serial)
{
	aoview_log_serial = serial;
}

int
aoview_log_get_serial(void)
{
	return aoview_log_serial;
}

static void
aoview_log_open_failed(char *name)
{
	char	*utf8_file;
	utf8_file = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
	if (!utf8_file)
		utf8_file = name;
	gtk_message_dialog_format_secondary_text(log_fail_dialog,
						 "\"%s\"", utf8_file);
	if (utf8_file != name)
		g_free(utf8_file);
	gtk_widget_show(GTK_WIDGET(log_fail_dialog));
	aoview_log_failed = 1;
}

static void
aoview_log_start(void)
{
	if (!aoview_log_file) {
		char		base[50];
		struct tm	tm;
		time_t		now;
		char		*full;
		int		r;

		now = time(NULL);
		(void) localtime_r(&now, &tm);
		aoview_mkdir(aoview_log_dir);
		for (;;) {
			sprintf(base, "%04d-%02d-%02d-serial-%03d-flight-%03d.log",
				tm.tm_year + 1900,
				tm.tm_mon + 1,
				tm.tm_mday,
				aoview_log_serial,
				aoview_log_sequence);
			full = aoview_fullname(aoview_log_dir, base);
			r = access(full, F_OK);
			if (r < 0) {
				aoview_log_file = fopen(full, "w");
				if (!aoview_log_file)
					aoview_log_open_failed(full);
				else
					setlinebuf(aoview_log_file);
				free(full);
				break;
			}
			free (full);
			aoview_log_sequence++;
		}
	}
}

void
aoview_log_printf(char *format, ...)
{
	va_list	ap;

	if (aoview_log_failed)
		return;
	aoview_log_start();
	va_start(ap, format);
	vfprintf(aoview_log_file, format, ap);
	va_end(ap);
}

void
aoview_log_init(GladeXML *xml)
{
	GConfClient	*gconf_client;
	char		*log_dir = NULL;
	GtkFileChooser	*log_chooser_dialog;
	GtkWidget	*log_configure_ok;
	GtkWidget	*log_new;

	g_type_init();
	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		log_dir = gconf_client_get_string(gconf_client,
						  LOG_DIR_PATH,
						  NULL);
		g_object_unref(G_OBJECT(gconf_client));
	}
	if (!log_dir) {
		aoview_log_dir = aoview_fullname(getenv("HOME"), DEFAULT_LOG);
		aoview_log_save_conf();
	} else {
		aoview_log_dir = strdup(log_dir);
	}

	log_chooser_dialog = GTK_FILE_CHOOSER(glade_xml_get_widget(xml, "log_chooser_dialog"));
	assert(log_chooser_dialog);
	gtk_file_chooser_set_filename(log_chooser_dialog, aoview_log_dir);

	log_configure_ok = glade_xml_get_widget(xml, "log_configure_ok");
	assert(log_configure_ok);

	g_signal_connect(G_OBJECT(log_configure_ok), "clicked",
			 G_CALLBACK(aoview_log_configure),
			 log_chooser_dialog);

	log_new = glade_xml_get_widget(xml, "ao_log_new");
	assert(log_new);
	g_signal_connect(G_OBJECT(log_new), "activate",
			 G_CALLBACK(aoview_log_new_item),
			 NULL);

	log_fail_dialog = GTK_MESSAGE_DIALOG(glade_xml_get_widget(xml, "log_fail_dialog"));
	assert(log_fail_dialog);
}
