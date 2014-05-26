/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "ccdbg-debug.h"
#include "cc-usb.h"


#define CC_NUM_HEX_READ		64
/*
 * AltOS has different buffer sizes for in/out packets
 */
#define CC_IN_BUF		65536
#define CC_OUT_BUF		64
#define DEFAULT_TTY		"/dev/ttyACM0"

struct cc_hex_read {
	uint8_t	*buf;
	int	len;
};

struct cc_usb {
	int			fd;
	uint8_t			in_buf[CC_IN_BUF];
	int			in_pos;
	int			in_count;
	uint8_t			out_buf[CC_OUT_BUF];
	int			out_count;

	struct cc_hex_read	hex_buf[CC_NUM_HEX_READ];
	int			hex_count;
	int			show_input;

	int			remote;
};

#define NOT_HEX	0xff

static uint8_t
cc_hex_nibble(uint8_t c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	return NOT_HEX;
}

/*
 * Take raw input bytes, parse them as hex
 * and write them to the waiting buffer
 */
static void
cc_handle_hex_read(struct cc_usb *cc)
{
	uint8_t	h, l;
	int	hex_pos;

	hex_pos = 0;
	while (hex_pos < cc->hex_count && cc->in_pos < cc->in_count) {
		/*
		 * Skip to next hex character
		 */
		while (cc->in_pos < cc->in_count &&
		       cc_hex_nibble(cc->in_buf[cc->in_pos]) == NOT_HEX)
			cc->in_pos++;
		/*
		 * Make sure we have two characters left
		 */
		if (cc->in_count - cc->in_pos < 2)
			break;
		/*
		 * Parse hex number
		 */
		h = cc_hex_nibble(cc->in_buf[cc->in_pos]);
		l = cc_hex_nibble(cc->in_buf[cc->in_pos+1]);
		if (h == NOT_HEX || l == NOT_HEX) {
			fprintf(stderr, "hex read error\n");
			break;
		}
		cc->in_pos += 2;
		/*
		 * Store hex number
		 */
		*cc->hex_buf[hex_pos].buf++ = (h << 4) | l;
		if (--cc->hex_buf[hex_pos].len <= 0)
			hex_pos++;
	}

	/* Move pending hex reads to the start of the array */
	if (hex_pos) {
		memmove(cc->hex_buf, cc->hex_buf + hex_pos,
			(cc->hex_count - hex_pos) * sizeof (cc->hex_buf[0]));
		cc->hex_count -= hex_pos;
	}
}

static void
cc_usb_dbg(int indent, uint8_t *bytes, int len)
{
	static int	eol = 1;
	int	i;
	uint8_t	c;
	ccdbg_debug(CC_DEBUG_BITBANG, "<<<%d bytes>>>", len);
	while (len--) {
		c = *bytes++;
		if (eol) {
			for (i = 0; i < indent; i++)
				ccdbg_debug(CC_DEBUG_BITBANG, " ");
			eol = 0;
		}
		switch (c) {
		case '\r':
			ccdbg_debug(CC_DEBUG_BITBANG, "\\r");
			break;
		case '\n':
			eol = 1;
			ccdbg_debug(CC_DEBUG_BITBANG, "\\n\n");
			break;
		default:
			if (c < ' ' || c > '~')
				ccdbg_debug(CC_DEBUG_BITBANG, "\\%02x", c);
			else
				ccdbg_debug(CC_DEBUG_BITBANG, "%c", c);
		}
	}
}

/*
 * Flush pending writes, fill pending reads
 */

static int
_cc_usb_sync(struct cc_usb *cc, int wait_for_input, int write_timeout)
{
	int		ret;
	struct pollfd	fds;
	int		timeout;

	fds.fd = cc->fd;
	for (;;) {
		if (cc->hex_count || cc->out_count)
			timeout = write_timeout;
		else if (wait_for_input && cc->in_pos == cc->in_count)
			timeout = wait_for_input;
		else
			timeout = 0;
		fds.events = 0;
		/* Move remaining bytes to the start of the input buffer */
		if (cc->in_pos) {
			memmove(cc->in_buf, cc->in_buf + cc->in_pos,
				cc->in_count - cc->in_pos);
			cc->in_count -= cc->in_pos;
			cc->in_pos = 0;
		}
		if (cc->in_count < CC_IN_BUF)
			fds.events |= POLLIN;
		if (cc->out_count)
			fds.events |= POLLOUT;
		ret = poll(&fds, 1, timeout);
		if (ret == 0) {
			if (timeout)
				return -1;
			break;
		}
		if (ret < 0) {
			perror("poll");
			return -1;
		}
		if (fds.revents & POLLIN) {
			ret = read(cc->fd, cc->in_buf + cc->in_count,
				   CC_IN_BUF - cc->in_count);
			if (ret > 0) {
				cc_usb_dbg(24, cc->in_buf + cc->in_count, ret);
				cc->in_count += ret;
				if (cc->hex_count)
					cc_handle_hex_read(cc);
				if (cc->show_input && cc->in_count) {
					write(2, cc->in_buf, cc->in_count);
					cc->in_count = 0;
				}
			} else if (ret < 0)
				perror("read");
		}
		if (fds.revents & POLLOUT) {
			ret = write(cc->fd, cc->out_buf,
				    cc->out_count);
			if (ret > 0) {
				cc_usb_dbg(0, cc->out_buf, ret);
				memmove(cc->out_buf,
					cc->out_buf + ret,
					cc->out_count - ret);
				cc->out_count -= ret;
			} else if (ret < 0)
				perror("write");
		}
	}
	return 0;
}

void
cc_usb_sync(struct cc_usb *cc)
{
	if (_cc_usb_sync(cc, 0, 5000) < 0) {
		fprintf(stderr, "USB link timeout\n");
		exit(1);
	}
}

void
cc_usb_printf(struct cc_usb *cc, char *format, ...)
{
	char	buf[1024], *b;
	va_list	ap;
	int	ret, this_time;

	/* sprintf to a local buffer */
	va_start(ap, format);
	ret = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	if (ret > sizeof(buf)) {
		fprintf(stderr, "printf overflow for format %s\n",
			format);
	}

	/* flush local buffer to the wire */
	b = buf;
	while (ret > 0) {
		this_time = ret;
		if (this_time > CC_OUT_BUF - cc->out_count)
			this_time = CC_OUT_BUF - cc->out_count;
		memcpy(cc->out_buf + cc->out_count, b, this_time);
		cc->out_count += this_time;
		ret -= this_time;
		b += this_time;
		while (cc->out_count >= CC_OUT_BUF)
			cc_usb_sync(cc);
	}
}

int
cc_usb_getchar_timeout(struct cc_usb *cc, int timeout)
{
	while (cc->in_pos == cc->in_count) {
		if (_cc_usb_sync(cc, timeout, 5000) < 0) {
			fprintf(stderr, "USB link timeout\n");
			exit(1);
		}
	}
	return cc->in_buf[cc->in_pos++];
}

int
cc_usb_getchar(struct cc_usb *cc)
{
	return cc_usb_getchar_timeout(cc, 5000);
}

void
cc_usb_getline(struct cc_usb *cc, char *line, int max)
{
	int	c;

	while ((c = cc_usb_getchar(cc)) != '\n') {
		switch (c) {
		case '\r':
			break;
		default:
			if (max > 1) {
				*line++ = c;
				max--;
			}
			break;
		}
	}
	*line++ = '\0';
}

int
cc_usb_send_bytes(struct cc_usb *cc, uint8_t *bytes, int len)
{
	int	this_len;
	int	ret = len;

	while (len) {
		this_len = len;
		if (this_len > 8)
			this_len = 8;
		len -= this_len;
		cc_usb_printf(cc, "P");
		while (this_len--)
			cc_usb_printf (cc, " %02x", (*bytes++) & 0xff);
		cc_usb_printf(cc, "\n");
	}
	return ret;
}

void
cc_queue_read(struct cc_usb *cc, uint8_t *buf, int len)
{
	struct cc_hex_read	*hex_buf;

	/* At the start of a command sequence, flush any pending input */
	if (cc->hex_count == 0) {
		cc_usb_sync(cc);
		cc->in_count = 0;
	}
	while (cc->hex_count >= CC_NUM_HEX_READ)
		cc_usb_sync(cc);
	hex_buf = &cc->hex_buf[cc->hex_count++];
	hex_buf->buf = buf;
	hex_buf->len = len;
}

int
cc_usb_recv_bytes(struct cc_usb *cc, uint8_t *buf, int len)
{
	cc_queue_read(cc, buf, len);
	cc_usb_printf(cc, "G %x\n", len);
	return len;
}

int
cc_usb_write_memory(struct cc_usb *cc, uint16_t addr, uint8_t *bytes, int len)
{
	cc_usb_printf(cc, "O %x %x\n", len, addr);
	while (len--)
		cc_usb_printf(cc, "%02x", *bytes++);
	cc_usb_sync(cc);
	return 0;
}

int
cc_usb_read_memory(struct cc_usb *cc, uint16_t addr, uint8_t *bytes, int len)
{
	int	i;
	cc_queue_read(cc, bytes, len);
	cc_usb_printf(cc, "I %x %x\n", len, addr);
	cc_usb_sync(cc);
	for (i = 0; i < len; i++) {
		if ((i & 15) == 0) {
			if (i)
				ccdbg_debug(CC_DEBUG_MEMORY, "\n");
			ccdbg_debug(CC_DEBUG_MEMORY, "\t%04x", addr + i);
		}
		ccdbg_debug(CC_DEBUG_MEMORY, " %02x", bytes[i]);
	}
	ccdbg_debug(CC_DEBUG_MEMORY, "\n");
	return 0;
}

int
cc_usb_debug_mode(struct cc_usb *cc)
{
	cc_usb_sync(cc);
	cc_usb_printf(cc, "D\n");
	return 1;
}

int
cc_usb_reset(struct cc_usb *cc)
{
	cc_usb_sync(cc);
	cc_usb_printf(cc, "R\n");
	return 1;
}

void
cc_usb_open_remote(struct cc_usb *cc, int freq, char *call)
{
	if (!cc->remote) {
		fprintf (stderr, "freq %dkHz\n", freq);
		fprintf (stderr, "call %s\n", call);
		cc_usb_printf(cc, "\nc F %d\nc c %s\np\nE 0\n", freq, call);
		do {
			cc->in_count = cc->in_pos = 0;
			_cc_usb_sync(cc, 100, 5000);
		} while (cc->in_count > 0);
		cc->remote = 1;
	}
}

void
cc_usb_close_remote(struct cc_usb *cc)
{
	if (cc->remote) {
		cc_usb_printf(cc, "~");
		cc->remote = 0;
	}
}

static struct termios	save_termios;

#include <errno.h>

struct cc_usb *
cc_usb_open(char *tty)
{
	struct cc_usb	*cc;
	struct termios	termios;
	int		i;

	if (!tty)
		tty = DEFAULT_TTY;
	cc = calloc (sizeof (struct cc_usb), 1);
	if (!cc)
		return NULL;
	i = 0;
	for (;;) {
		cc->fd = open(tty, O_RDWR | O_NONBLOCK);
		if (cc->fd >= 0)
			break;
		i++;
		if (errno == EBUSY || errno == EPERM || errno == EACCES) {
			fprintf(stderr, "open failed, pausing");
			perror(tty);
			if (i < 20) {
				sleep(3);
				continue;
			}
		}

		perror(tty);
		free (cc);
		return NULL;
	}
	tcgetattr(cc->fd, &termios);
	save_termios = termios;
	cfmakeraw(&termios);
	cfsetospeed(&termios, B9600);
	cfsetispeed(&termios, B9600);
	tcsetattr(cc->fd, TCSAFLUSH, &termios);
	cc_usb_printf(cc, "\nE 0\nm 0\n");
	do {
		cc->in_count = cc->in_pos = 0;
		_cc_usb_sync(cc, 100, 5000);
	} while (cc->in_count > 0);
	return cc;
}

void
cc_usb_close(struct cc_usb *cc)
{
	cc_usb_close_remote(cc);
	cc_usb_sync(cc);
	tcsetattr(cc->fd, TCSAFLUSH, &save_termios);
	close (cc->fd);
	free (cc);
}

int
cc_usb_write(struct cc_usb *cc, void *buf, int c)
{
	uint8_t	*b;
	int this_time;

	b = buf;
	cc->show_input = 1;
	while (c > 0) {
		this_time = c;
		if (this_time > CC_OUT_BUF - cc->out_count)
			this_time = CC_OUT_BUF - cc->out_count;
		memcpy(cc->out_buf + cc->out_count, b, this_time);
		cc->out_count += this_time;
		c -= this_time;
		b += this_time;
		while (cc->out_count >= CC_OUT_BUF) {
			_cc_usb_sync(cc, 0, -1);
		}
	}
	return 1;
}
