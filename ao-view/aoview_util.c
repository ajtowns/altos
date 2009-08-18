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

char *
aoview_fullname (char *dir, char *file)
{
	char	*new;
	int	dlen = strlen (dir);
	int	flen = strlen (file);
	int	slen = 0;

	if (dir[dlen-1] != '/')
		slen = 1;
	new = malloc (dlen + slen + flen + 1);
	if (!new)
		return 0;
	strcpy(new, dir);
	if (slen)
		strcat (new, "/");
	strcat(new, file);
	return new;
}

char *
aoview_basename(char *file)
{
	char *b;

	b = strrchr(file, '/');
	if (!b)
		return file;
	return b + 1;
}

int
aoview_mkdir(char *dir)
{
	char	*slash;
	char	*d;
	char	*part;

	d = dir;
	for (;;) {
		slash = strchr (d, '/');
		if (!slash)
			slash = d + strlen(d);
		if (!*slash)
			break;
		part = strndup(dir, slash - dir);
		if (!access(part, F_OK))
			if (mkdir(part, 0777) < 0)
				return -errno;
		free(part);
		d = slash + 1;
	}
	return 0;
}

GtkTreeViewColumn *
aoview_add_plain_text_column (GtkTreeView *view, const gchar *title, gint model_column, gint width)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_NONE, NULL);
	g_object_set(renderer, "width-chars", width, NULL);
	column = gtk_tree_view_column_new_with_attributes (title, renderer,
							   "text", model_column,
							   NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (view, column);

	return column;
}
