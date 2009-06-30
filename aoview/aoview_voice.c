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
	if (!aoview_flite)
		aoview_flite = popen("aoview_flite", "w");
}

void aoview_voice_close(void)
{
	if (aoview_flite) {
		pclose(aoview_flite);
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

static void
aoview_voice_enable(GtkWidget *widget, gpointer data)
{
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
		aoview_voice_open();
		aoview_voice_speak("enable voice system\n");
	} else {
		aoview_voice_speak("disable voice system\n");
		aoview_voice_close();
	}
}

void
aoview_voice_init(GladeXML *xml)
{
	aoview_voice_open();

	voice_enable = GTK_CHECK_MENU_ITEM(glade_xml_get_widget(xml, "voice_enable"));
	assert(voice_enable);

	g_signal_connect(G_OBJECT(voice_enable), "toggled",
			 G_CALLBACK(aoview_voice_enable),
			 voice_enable);
}
