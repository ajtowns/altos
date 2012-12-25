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

#define _BSD_SOURCE
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include "sky_flash.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>

int	skytraq_verbose = 1;

int
skytraq_setspeed(int fd, int baud)
{
	int	b;
	int	ret;
	struct termios	term;

	switch (baud) {
	case 9600:
		b = B9600;
		break;
	case 38400:
		b = B38400;
		break;
	case 115200:
		b = B115200;
		break;
	default:
		fprintf (stderr, "Invalid baudrate %d\n", baud);
		return -1;
	}
	ret = tcgetattr(fd, &term);
	cfmakeraw(&term);
#ifdef USE_POLL
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
#else
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
#endif

	cfsetspeed(&term, b);

	ret = tcsetattr(fd, TCSAFLUSH, &term);
	return ret;
}

int	skytraq_open_time;

int
skytraq_open(const char *path)
{
	int		fd;
	int		ret;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror (path);
		return -1;
	}

	ret = skytraq_setspeed(fd, 9600);
	if (ret < 0) {
		close (fd);
		return -1;
	}
	skytraq_open_time = skytraq_millis();
	return fd;
}


#define BAUD		57600
#define BPS		(BAUD/10 * 9/10)
#define US_PER_CHAR	(1000000 / BPS)

int
skytraq_write(int fd, const char *data, int len)
{
	const char *d = data;
	int		r;
	int		us;

	skytraq_dbg_printf (0, "%4d: ", len);
	if (len < 70)
		skytraq_dbg_buf(0, data, len);
	while (len) {
		int	this_time = len;
		if (this_time > 128)
			this_time = 128;
		skytraq_dbg_printf(0, ".");
		fflush(stdout);
		r = write(fd, data, this_time);
		if (r <= 0)
			return r;
		us = r * US_PER_CHAR;
		usleep(r * US_PER_CHAR);
		data += r;
		len -= r;
	}
	skytraq_dbg_newline();
	return 1;
}

int
skytraq_setcomm(int fd, int baudrate)
{
	uint8_t	msg[11];
	int	i;
	uint8_t	cksum;

	int target_baudrate;
	switch(baudrate)
	{
	case 4800:
		target_baudrate=0;
		break;
	case 9600:
		target_baudrate=1;
		break;
	case 19200:
		target_baudrate=2;
		break;
	case 38400:
		target_baudrate=3;
		break;
	case 57600:
		target_baudrate=4;
		break;
	case 115200:
		target_baudrate=5;
		break;
	case 230400:
		target_baudrate=6;
		break;
	}
	msg[0] = 0xa0;	/* header */
	msg[1] = 0xa1;
	msg[2] = 0x00;	/* length */
	msg[3] = 0x04;
	msg[4] = 0x05;	/* configure serial port */
	msg[5] = 0x00;	/* COM 1 */
	msg[6] = target_baudrate;
	msg[7] = 0x00;	/* update to SRAM only */

	cksum = 0;
	for (i = 4; i < 8; i++)
		cksum ^= msg[i];
	msg[8] = cksum;
	msg[9] = 0x0d;
	msg[10] = 0x0a;
	return skytraq_write(fd, msg, 11);
}

int
skytraq_waitchar(int fd, int timeout)
{
	struct pollfd	fds[1];
	int		ret;
	unsigned char	c;

	for (;;) {
		fds[0].fd = fd;
		fds[0].events = POLLIN;
		ret = poll(fds, 1, timeout);
		if (ret >= 1) {
			if (fds[0].revents & POLLIN) {
				ret = read(fd, &c, 1);
				if (ret == 1) {
					skytraq_dbg_char(1, c);
					return c;
				}
			}
		} else if (ret == 0)
			return -2;
		else {
			perror("poll");
			return -1;
		}
	}
}

int
skytraq_waitstatus(int fd, const char *status, int timeout)
{
	const char	*s;
	int		c;

	for (;;) {
		c = skytraq_waitchar(fd, timeout);
		if (c < 0) {
			skytraq_dbg_newline();
			return c;
		}
		if ((char) c == *status) {
			s = status + 1;
			for (;;) {
				c = skytraq_waitchar(fd, timeout);
				if (c < 0) {
					skytraq_dbg_newline();
					return c;
				}
				if ((char) c != *s)
					break;
				if (!*s) {
					skytraq_dbg_newline();
					return 0;
				}
				s++;
			}
		}
	}
}

void
skytraq_flush(int fd)
{
	while (skytraq_waitchar(fd, 1) >= 0)
		;
}

int
skytraq_cmd_wait(int fd, const char *message, int len, const char *status, int timeout)
{
	skytraq_flush(fd);
	skytraq_write(fd, message, len);
	return skytraq_waitstatus(fd, status, timeout);
}

int
skytraq_cmd_nowait(int fd, const char *message, int len)
{
	skytraq_flush(fd);
	return skytraq_write(fd, message, len);
}
