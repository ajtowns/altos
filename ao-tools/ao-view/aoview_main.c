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

static const char aoview_glade[] = {
#include "aoview_glade.h"
};

static void usage(void) {
	printf("aoview [--device|-d device_file]");
	exit(1);
}

static void destroy_event(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

char *aoview_tty = NULL;

int main(int argc, char **argv)
{
	GladeXML *xml = NULL;
	GtkWidget *mainwindow;
	GtkAboutDialog *about_dialog;

	static struct option long_options[] = {
		{ "tty", 1, 0, 'T'},
		{ 0, 0, 0, 0 }
	};
	for (;;) {
		int c, temp;

		c = getopt_long_only(argc, argv, "T:", long_options, &temp);
		if (c == -1)
			break;

		switch (c) {
		case 'T':
			aoview_tty = optarg;
			break;
		default:
			usage();
		}
	}

	g_thread_init(NULL);
	gtk_init(&argc, &argv);
	glade_init();

	xml = glade_xml_new_from_buffer(aoview_glade, sizeof (aoview_glade), NULL, NULL);

	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);

	/* Hook up the close button. */
	mainwindow = glade_xml_get_widget(xml, "aoview");
	assert(mainwindow);

	g_signal_connect (G_OBJECT(mainwindow), "destroy",
	    G_CALLBACK(destroy_event), NULL);

	about_dialog = GTK_ABOUT_DIALOG(glade_xml_get_widget(xml, "about_dialog"));
	assert(about_dialog);
	gtk_about_dialog_set_version(about_dialog, AOVIEW_VERSION);

	aoview_voice_init(xml);

	aoview_channel_init(xml);

	aoview_dev_dialog_init(xml);

	aoview_state_init(xml);

	aoview_file_init(xml);

	aoview_log_init(xml);

	aoview_table_init(xml);

	aoview_eeprom_init(xml);

	aoview_replay_init(xml);

	aoview_label_init(xml);

	if (aoview_tty) {
		if (!aoview_monitor_connect(aoview_tty)) {
			perror(aoview_tty);
			exit(1);
		}
	}
	aoview_voice_speak("rocket flight monitor ready\n");

	gtk_main();

	return 0;
}
