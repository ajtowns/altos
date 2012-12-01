/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include "sky_flash.h"
#include <stdio.h>
#include <string.h>

static const char loader_start[] = "$LOADER DOWNLOAD";

int
skytraq_send_srec(int fd, const char *filename)
{
	FILE	*file;
	int	ret;
	char	line[1024];

	file = fopen(filename, "r");
	if (!file) {
		perror(filename);
		return -1;
	}

	ret = skytraq_cmd_wait(fd, loader_start, strlen(loader_start) + 1, "OK", 1000);
	if (ret)
		return ret;

	for (;;) {
		char	*s;
		int	len;

		s = fgets(line, sizeof(line), file);
		if (!s)
			break;
		len = strlen(s);
		if (len < 3)		/* Terminated with \r\n */
			break;
		s[len-2] = '\n';	/* Smash \r */
		s[len-1] = '\0';	/* Smash \n */
		skytraq_cmd_nowait(fd, s, len);
	}
	fclose(file);

	ret = skytraq_waitstatus(fd, "END", 10000);
	skytraq_dbg_newline();
	return ret;
}
