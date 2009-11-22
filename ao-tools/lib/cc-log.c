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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <gconf/gconf-client.h>
#include "cc.h"

static char *cc_file_dir;

#define ALTOS_DIR_PATH	"/apps/aoview/log_dir"
#define DEFAULT_DIR	"AltOS"

static void
cc_file_save_conf(void)
{
	GConfClient	*gconf_client;

	g_type_init();
	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		gconf_client_set_string(gconf_client,
					ALTOS_DIR_PATH,
					cc_file_dir,
					NULL);
		g_object_unref(G_OBJECT(gconf_client));
	}
}

static void
cc_file_load_conf(void)
{
	char *file_dir;
	GConfClient	*gconf_client;

	g_type_init();
	gconf_client = gconf_client_get_default();
	if (gconf_client)
	{
		file_dir = gconf_client_get_string(gconf_client,
						   ALTOS_DIR_PATH,
						   NULL);
		g_object_unref(G_OBJECT(gconf_client));
		if (file_dir)
			cc_file_dir = strdup(file_dir);
	}
}

void
cc_set_log_dir(char *dir)
{
	cc_file_dir = strdup(dir);
	cc_file_save_conf();
}

char *
cc_get_log_dir(void)
{
	cc_file_load_conf();
	if (!cc_file_dir) {
		cc_file_dir = cc_fullname(getenv("HOME"), DEFAULT_DIR);
		cc_file_save_conf();
	}
	return cc_file_dir;
}

char *
cc_make_filename(int serial, int flight, char *ext)
{
	char		base[50];
	char		seq[20];
	struct tm	tm;
	time_t		now;
	char		*full;
	int		r;
	int		sequence;

	now = time(NULL);
	(void) localtime_r(&now, &tm);
	cc_mkdir(cc_get_log_dir());
	sequence = 0;
	for (;;) {
		if (sequence)
			snprintf(seq, sizeof(seq), "-seq-%03d", sequence);
		else
			seq[0] = '\0';

		snprintf(base, sizeof (base), "%04d-%02d-%02d-serial-%03d-flight-%03d%s.%s",
			 tm.tm_year + 1900,
			 tm.tm_mon + 1,
			 tm.tm_mday,
			 serial,
			 flight,
			 seq,
			 ext);
		full = cc_fullname(cc_get_log_dir(), base);
		r = access(full, F_OK);
		if (r < 0)
			return full;
		free(full);
		sequence++;
	}

}
