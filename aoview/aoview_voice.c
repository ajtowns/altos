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

#if HAVE_FLITE
#include <stdarg.h>

FILE	*aoview_flite;

void aoview_voice_open(void)
{
	int	err;

	if (!aoview_flite)
		aoview_flite = aoview_flite_start();
}

void aoview_voice_close(void)
{
	if (aoview_flite) {
		aoview_flite_stop();
		aoview_flite = NULL;
	}
}

void aoview_voice_speak(char *format, ...)
{
	va_list	ap;

	if (aoview_flite) {
		va_start(ap, format);
		vfprintf(aoview_flite, format, ap);
		fflush(aoview_flite);
		va_end(ap);
	}
}

#else
void aoview_voice_open(void)
{
}

void aoview_voice_close(void)
{
}

void aoview_voice_speak(char *format, ...)
{
}
#endif


static GtkCheckMenuItem	*voice_enable;

#define ALTOS_VOICE_PATH	"/apps/aoview/voice"

static void
aoview_voice_enable(GtkWidget *widget, gpointer data)
{
	gboolean	enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	GError		*error;
	GConfClient	*gconf_client;

	if (enabled) {
		aoview_voice_open();
		aoview_voice_speak("enable voice\n");
	} else {
		aoview_voice_speak("disable voice\n");
		aoview_voice_close();
	}
	gconf_client = gconf_client_get_default();
	gconf_client_set_bool(gconf_client,
			      ALTOS_VOICE_PATH,
			      enabled,
			      &error);
}

void
aoview_voice_init(GladeXML *xml)
{
	gboolean	enabled;
	GConfClient	*gconf_client;

	voice_enable = GTK_CHECK_MENU_ITEM(glade_xml_get_widget(xml, "voice_enable"));
	assert(voice_enable);

	gconf_client = gconf_client_get_default();
	enabled = TRUE;
	if (gconf_client)
	{
		GError	*error;

		error = NULL;
		enabled = gconf_client_get_bool(gconf_client,
						ALTOS_VOICE_PATH,
						&error);
		if (error)
			enabled = TRUE;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(voice_enable), enabled);
	if (enabled)
		aoview_voice_open();

	g_signal_connect(G_OBJECT(voice_enable), "toggled",
			 G_CALLBACK(aoview_voice_enable),
			 voice_enable);
}
