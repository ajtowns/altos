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


#define NUM_CHANNEL	10

static GtkRadioMenuItem *channel_item[NUM_CHANNEL];

int
aoview_channel_current(void)
{
	int	c;

	for (c = 0; c < NUM_CHANNEL; c++)
		if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(channel_item[c])))
			return c;
	return -1;
}

static void
aoview_channel_notify(int channel)
{
	if (0 <= channel && channel < NUM_CHANNEL)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(channel_item[channel]), TRUE);
}

#define ALTOS_CHANNEL_PATH	"/apps/aoview/channel"

static void
aoview_channel_change(GtkWidget *widget, gpointer data)
{
	gboolean	enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	int		c = (int) data;
	GConfClient	*gconf_client;
	GError		*error;

	if (enabled) {
		aoview_monitor_set_channel(c);
		gconf_client = gconf_client_get_default();
		gconf_client_set_int(gconf_client, ALTOS_CHANNEL_PATH, c, &error);
	}
}

void
aoview_channel_init(GladeXML *xml)
{
	int	c;
	GConfClient	*gconf_client;

	for (c = 0; c < NUM_CHANNEL; c++) {
		char	name[32];

		sprintf(name, "channel_%d", c);
		channel_item[c] = GTK_RADIO_MENU_ITEM(glade_xml_get_widget(xml, name));
		assert(channel_item[c]);
		g_signal_connect(G_OBJECT(channel_item[c]), "toggled",
				 G_CALLBACK(aoview_channel_change),
				 (gpointer) c);
	}
	gconf_client = gconf_client_get_default();
	c = 0;
	if (gconf_client)
	{
		GError	*error;

		error = NULL;
		c = gconf_client_get_int(gconf_client,
					 ALTOS_CHANNEL_PATH,
					 &error);
		if (error)
			c = 0;
	}
	aoview_channel_notify(c);
}
