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

/* sky_serial.c */

extern int	skytraq_open_time;
extern int	skytraq_verbose;

int
skytraq_open(const char *path);

int
skytraq_setspeed(int fd, int baud);

int
skytraq_setcomm(int fd, int baudrate);

int
skytraq_write(int fd, const char *data, int len);

int
skytraq_waitchar(int fd, int timeout);

int
skytraq_waitstatus(int fd, const char *status, int timeout);

void
skytraq_flush(int fd);

int
skytraq_cmd_wait(int fd, const char *message, int len, const char *status, int timeout);

int
skytraq_cmd_nowait(int fd, const char *message, int len);

/* sky_debug.c */

void
skytraq_dbg_printf(int input, const char *fmt, ...);

void
skytraq_dbg_buf(int input, const char *buf, int len);

void
skytraq_dbg_char(int input, char c);

/* sky_srec.c */
int
skytraq_send_srec(int fd, const char *file);

/* sky_bin.c */
int
skytraq_send_bin(int fd, const char *filename);
