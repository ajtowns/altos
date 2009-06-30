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

static struct {
	char		*name;
	char		*initial_value;
	GtkLabel	*widget;
} label_widgets[] = {
	{ "height_label", "Height", NULL },
	{ "state_label", "State", NULL },
	{ "rssi_label", "RSSI", NULL },
	{ "height_value", "0m", NULL },
	{ "state_value", "pad", NULL },
	{ "rssi_value", "-50dBm", NULL },
};

static void
aoview_label_assign(GtkLabel *widget, char *value)
{
	char	*markup;

	markup = g_markup_printf_escaped("<span font_weight=\"bold\" size=\"xx-large\">%s</span>", value);
	gtk_label_set_markup(widget, markup);
	g_free(markup);
}

void
aoview_label_show(struct aostate *state)
{
	char	line[1024];
	sprintf(line, "%dm", state->height);
	aoview_label_assign(label_widgets[3].widget, line);

	aoview_label_assign(label_widgets[4].widget, state->state);

	sprintf(line, "%ddBm", state->rssi);
	aoview_label_assign(label_widgets[5].widget, line);
}

void
aoview_label_init(GladeXML *xml)
{
	int i;
	for (i = 0; i < sizeof(label_widgets)/sizeof(label_widgets[0]); i++) {
		label_widgets[i].widget = GTK_LABEL(glade_xml_get_widget(xml, label_widgets[i].name));
		aoview_label_assign(label_widgets[i].widget, label_widgets[i].initial_value);
		assert(label_widgets[i].widget);
	}
}
