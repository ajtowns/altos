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

#define NCOL	3

static GtkTreeView	*dataview[NCOL];
static GtkListStore	*datalist[NCOL];

void
aoview_table_start(void)
{
	int col;
	for (col = 0; col < NCOL; col++)
		datalist[col] = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
}

void
aoview_table_add_row(int col, char *label, char *format, ...)
{
	char		buf[1024];
	va_list		ap;
	GtkTreeIter	iter;

	va_start(ap, format);
	vsnprintf(buf, sizeof (buf), format, ap);
	va_end(ap);
	gtk_list_store_append(datalist[col], &iter);
	gtk_list_store_set(datalist[col], &iter,
			   0, label,
			   1, buf,
			   -1);
}

void
aoview_table_finish(void)
{
	int	col;
	for (col = 0; col < NCOL; col++) {
		gtk_tree_view_set_model(dataview[col], GTK_TREE_MODEL(datalist[col]));
		g_object_unref(G_OBJECT(datalist[col]));
		gtk_tree_view_columns_autosize(dataview[col]);
	}
}

void
aoview_table_clear(void)
{
	int	col;
	for (col = 0; col < NCOL; col++)
		gtk_tree_view_set_model(dataview[col], NULL);
}

void
aoview_table_init(GladeXML *xml)
{
	int	col;

	for (col = 0; col < NCOL; col++) {
		char	name[32];
		sprintf(name, "dataview_%d", col);
		dataview[col] = GTK_TREE_VIEW(glade_xml_get_widget(xml, name));
		assert(dataview[col]);

		aoview_add_plain_text_column(dataview[col], "Field", 0, 20);
		aoview_add_plain_text_column(dataview[col], "Value", 1, 32);
	}
}
