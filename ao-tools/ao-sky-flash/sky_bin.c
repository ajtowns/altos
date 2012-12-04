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

#define FLASHBYTES	8192

int
skytraq_send_bin(int fd, const char *filename)
{
	FILE		*file;
	char		buf[FLASHBYTES];
	int		count;
	unsigned char	cksum;
	int		c;
	long		size;
	long		pos;
	char		message[1024];
	int		ret;
	
	file = fopen(filename, "r");
	if (!file) {
		perror(filename);
		return -1;
	}

	/* Compute checksum, figure out how long the file */
	cksum = 0;
	while ((c = getc(file)) != EOF)
		cksum += (unsigned char) c;
	size = ftell(file);
	rewind(file);

	sprintf(message, "BINSIZE = %d Checksum = %d Loopnumber = %d ", size, cksum, 1);

	ret = skytraq_cmd_wait(fd, message, strlen(message) + 1, "OK", 20000);
	if (ret < 0)
		printf ("waitstatus failed %d\n", ret);

	pos = 0;
	for (;;) {
		count = fread(buf, 1, sizeof (buf), file);
		if (count < 0) {
			perror("fread");
			fclose(file);
			return -1;
		}
		if (count == 0)
			break;
		skytraq_dbg_printf (0, "%7d of %7d ", pos + count, size);
		pos += count;
		ret = skytraq_cmd_wait(fd, buf, count, "OK", 20000);
		if (ret < 0)
			return ret;
	}
	return skytraq_waitstatus(fd, "END", 30000);
}
