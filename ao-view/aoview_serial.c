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
#include <termios.h>

#define AOVIEW_SERIAL_IN_BUF	64
#define AOVIEW_SERIAL_OUT_BUF	64

struct aoview_buf {
	char		*buf;
	int		off;
	int		count;
	int		size;
};

static int
aoview_buf_write(struct aoview_buf *buf, char *data, int len)
{
	if (buf->count + len > buf->size) {
		int	new_size = buf->size * 2;
		if (new_size == 0)
			new_size = 1024;
		if (buf->buf)
			buf->buf = realloc (buf->buf, new_size);
		else
			buf->buf = malloc (new_size);
		buf->size = new_size;
	}
	memcpy(buf->buf + buf->count, data, len);
	buf->count += len;
	return len;
}

static int
aoview_buf_read(struct aoview_buf *buf, char *data, int len)
{
	if (len > buf->count - buf->off)
		len = buf->count - buf->off;
	memcpy (data, buf->buf + buf->off, len);
	buf->off += len;
	if (buf->off == buf->count)
		buf->off = buf->count = 0;
	return len;
}

static int
aoview_buf_getc(struct aoview_buf *buf)
{
	char	b;
	int	r;

	r = aoview_buf_read(buf, &b, 1);
	if (r == 1)
		return (int) b;
	return -1;
}

static void
aoview_buf_flush(struct aoview_buf *buf, int fd)
{
	int	ret;

	if (buf->count > buf->off) {
		ret = write(fd, buf->buf + buf->off, buf->count - buf->off);
		if (ret > 0) {
			buf->off += ret;
			if (buf->off == buf->count)
				buf->off = buf->count = 0;
		}
	}
}

static void
aoview_buf_fill(struct aoview_buf *buf, int fd)
{
	int ret;

	while (buf->count >= buf->size) {
		int new_size = buf->size * 2;
		buf->buf = realloc (buf->buf, new_size);
		buf->size = new_size;
	}

	ret = read(fd, buf->buf + buf->count, buf->size - buf->count);
	if (ret > 0)
		buf->count += ret;
}

static void
aoview_buf_init(struct aoview_buf *buf)
{
	buf->buf = malloc (buf->size = 1024);
	buf->count = 0;
}

static void
aoview_buf_fini(struct aoview_buf *buf)
{
	free(buf->buf);
}

struct aoview_serial {
	GSource			source;
	int			fd;
	struct termios		save_termios;
	struct aoview_buf	in_buf;
	struct aoview_buf	out_buf;
	GPollFD			poll_fd;
};


void
aoview_serial_printf(struct aoview_serial *serial, char *format, ...)
{
	char	buf[1024];
	va_list	ap;
	int	ret;

	/* sprintf to a local buffer */
	va_start(ap, format);
	ret = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	if (ret > sizeof(buf)) {
		fprintf(stderr, "printf overflow for format %s\n",
			format);
	}

	/* flush local buffer to the wire */
	aoview_buf_write(&serial->out_buf, buf, ret);
	aoview_buf_flush(&serial->out_buf, serial->fd);
}

int
aoview_serial_read(struct aoview_serial *serial, char *buf, int len)
{
	return aoview_buf_read(&serial->in_buf, buf, len);
}

int
aoview_serial_getc(struct aoview_serial *serial)
{
	return aoview_buf_getc(&serial->in_buf);
}

static gboolean
serial_prepare(GSource *source, gint *timeout)
{
	struct aoview_serial *serial = (struct aoview_serial *) source;
	*timeout = -1;

	if (serial->out_buf.count)
		serial->poll_fd.events |= G_IO_OUT;
	else
		serial->poll_fd.events &= ~G_IO_OUT;
	return FALSE;
}

static gboolean
serial_check(GSource *source)
{
	struct aoview_serial *serial = (struct aoview_serial *) source;
	gint revents = serial->poll_fd.revents;

	if (revents & G_IO_NVAL)
		return FALSE;
	if (revents & G_IO_IN)
		return TRUE;
	if (revents & G_IO_OUT)
		return TRUE;
	return FALSE;
}

static gboolean
serial_dispatch(GSource *source,
		GSourceFunc callback,
		gpointer user_data)
{
	struct aoview_serial *serial = (struct aoview_serial *) source;
	aoview_serial_callback func = (aoview_serial_callback) callback;
	gint revents = serial->poll_fd.revents;

	if (revents & G_IO_IN)
		aoview_buf_fill(&serial->in_buf, serial->fd);

	if (revents & G_IO_OUT)
		aoview_buf_flush(&serial->out_buf, serial->fd);

	if (func)
		(*func)(user_data, serial, revents);
	return TRUE;
}

static void
serial_finalize(GSource *source)
{
	struct aoview_serial *serial = (struct aoview_serial *) source;

	aoview_buf_fini(&serial->in_buf);
	aoview_buf_fini(&serial->out_buf);
	tcsetattr(serial->fd, TCSAFLUSH, &serial->save_termios);
	close (serial->fd);
}

static GSourceFuncs serial_funcs = {
	serial_prepare,
	serial_check,
	serial_dispatch,
	serial_finalize
};

struct aoview_serial *
aoview_serial_open(const char *tty)
{
	struct aoview_serial	*serial;
	struct termios	termios;

	serial = (struct aoview_serial *) g_source_new(&serial_funcs, sizeof (struct aoview_serial));
	aoview_buf_init(&serial->in_buf);
	aoview_buf_init(&serial->out_buf);
	serial->fd = open (tty, O_RDWR | O_NONBLOCK);
	if (serial->fd < 0) {
		g_source_destroy(&serial->source);
		return NULL;
	}
	tcgetattr(serial->fd, &termios);
	serial->save_termios = termios;
	cfmakeraw(&termios);
	tcsetattr(serial->fd, TCSAFLUSH, &termios);

	aoview_serial_printf(serial, "E 0\n");
	tcdrain(serial->fd);
	usleep(15*1000);
	tcflush(serial->fd, TCIFLUSH);
	serial->poll_fd.fd = serial->fd;
	serial->poll_fd.events = G_IO_IN | G_IO_OUT | G_IO_HUP | G_IO_ERR;
	g_source_attach(&serial->source, NULL);
	g_source_add_poll(&serial->source,&serial->poll_fd);
	aoview_serial_set_callback(serial, NULL);
	return serial;
}

void
aoview_serial_close(struct aoview_serial *serial)
{
	g_source_remove_poll(&serial->source, &serial->poll_fd);
	close(serial->fd);
	g_source_destroy(&serial->source);
}

void
aoview_serial_set_callback(struct aoview_serial *serial,
			   aoview_serial_callback func)
{
	g_source_set_callback(&serial->source, (GSourceFunc) func, serial, NULL);
}
