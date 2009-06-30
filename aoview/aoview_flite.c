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

#include <stdio.h>
#include <flite/flite.h>
#include "aoview.h"

cst_voice *register_cmu_us_kal();
static cst_voice *voice;

static FILE *pipe_write;
static GThread *aoview_flite_thread;

gpointer
aoview_flite_task(gpointer data)
{
	FILE		*input = data;
	char		line[1024];

	while (fgets(line, sizeof (line) - 1, input) != NULL)
		flite_text_to_speech(line, voice, "play");
	return NULL;
}

void
aoview_flite_stop(void)
{
	int status;
	if (pipe_write) {
		fclose(pipe_write);
		pipe_write = NULL;
	}
	if (aoview_flite_thread) {
		g_thread_join(aoview_flite_thread);
		aoview_flite_thread = NULL;
	}
}

FILE *
aoview_flite_start(void)
{
	static once;
	int	p[2];
	GError	*error;
	FILE	*pipe_read;

	if (!once) {
		flite_init();
		voice = register_cmu_us_kal();
		if (!voice) {
			perror("register voice");
			exit(1);
		}
	}
	aoview_flite_stop();
	pipe(p);
	pipe_read = fdopen(p[0], "r");
	pipe_write = fdopen(p[1], "w");
	g_thread_create(aoview_flite_task, pipe_read, TRUE, &error);
	return pipe_write;
}
