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


#define CC_NUM_READ		16
/*
 * AltOS has different buffer sizes for in/out packets
 */
#define CC_IN_BUF		256
#define CC_OUT_BUF		64
#define DEFAULT_TTY		"/dev/ttyACM0"

struct cc_read {
	uint8_t	*buf;
	int	len;
};

struct cc_usb {
	int		fd;
	uint8_t		in_buf[CC_IN_BUF];
	int		in_count;
	uint8_t		out_buf[CC_OUT_BUF];
	int		out_count;
	struct cc_read	read_buf[CC_NUM_READ];
	int		read_count;
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
cc_handle_in(struct cc_usb *cc)
{
	uint8_t	h, l;
	int	in_pos;
	int	read_pos;

	in_pos = 0;
	read_pos = 0;
	while (read_pos < cc->read_count && in_pos < cc->in_count) {
		/*
		 * Skip to next hex character
		 */
		while (in_pos < cc->in_count &&
		       cc_hex_nibble(cc->in_buf[in_pos]) == NOT_HEX)
			in_pos++;
		/*
		 * Make sure we have two characters left
		 */
		if (cc->in_count - in_pos < 2)
			break;
		/*
		 * Parse hex number
		 */
		h = cc_hex_nibble(cc->in_buf[in_pos]);
		l = cc_hex_nibble(cc->in_buf[in_pos+1]);
		if (h == NOT_HEX || l == NOT_HEX) {
			fprintf(stderr, "hex read error\n");
			break;
		}
		in_pos += 2;
		/*
		 * Store hex number
		 */
		*cc->read_buf[read_pos].buf++ = (h << 4) | l;
		if (--cc->read_buf[read_pos].len <= 0)
			read_pos++;
	}

	/* Move remaining bytes to the start of the input buffer */
	if (in_pos) {
		memmove(cc->in_buf, cc->in_buf + in_pos,
			cc->in_count - in_pos);
		cc->in_count -= in_pos;
	}

	/* Move pending reads to the start of the array */
	if (read_pos) {
		memmove(cc->read_buf, cc->read_buf + read_pos,
			(cc->read_count - read_pos) * sizeof (cc->read_buf[0]));
		cc->read_count -= read_pos;
	}

	/* Once we're done reading, flush any pending input */
	if (cc->read_count == 0)
		cc->in_count = 0;
}

static void
cc_usb_dbg(int indent, uint8_t *bytes, int len)
{
	int	eol = 1;
	int	i;
	uint8_t	c;
	while (len--) {
		c = *bytes++;
		if (eol) {
			for (i = 0; i < indent; i++)
				ccdbg_debug(CC_DEBUG_BITBANG, " ");
			eol = 0;
		}
		switch (c) {
		case '\r':
			ccdbg_debug(CC_DEBUG_BITBANG, "^M");
			break;
		case '\n':
			eol = 1;
		default:
			ccdbg_debug(CC_DEBUG_BITBANG, "%c", c);
		}
	}
}

/*
 * Flush pending writes, fill pending reads
 */
void
cc_usb_sync(struct cc_usb *cc)
{
	int		ret;
	struct pollfd	fds;
	int		timeout;

	fds.fd = cc->fd;
	for (;;) {
		if (cc->read_count || cc->out_count)
			timeout = -1;
		else
			timeout = 0;
		fds.events = 0;
		if (cc->in_count < CC_IN_BUF)
			fds.events |= POLLIN;
		if (cc->out_count)
			fds.events |= POLLOUT;
		ret = poll(&fds, 1, timeout);
		if (ret == 0)
			break;
		if (ret < 0) {
			perror("poll");
			break;
		}
		if (fds.revents & POLLIN) {
			ret = read(cc->fd, cc->in_buf + cc->in_count,
				   CC_IN_BUF - cc->in_count);
			if (ret > 0) {
				cc_usb_dbg(24, cc->in_buf + cc->in_count, ret);
				cc->in_count += ret;
				cc_handle_in(cc);
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
	struct cc_read	*read_buf;
	while (cc->read_count >= CC_NUM_READ)
		cc_usb_sync(cc);
	read_buf = &cc->read_buf[cc->read_count++];
	read_buf->buf = buf;
	read_buf->len = len;
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

static struct termios	save_termios;

struct cc_usb *
cc_usb_open(char *tty)
{
	struct cc_usb	*cc;
	struct termios	termios;

	if (!tty)
		tty = DEFAULT_TTY;
	cc = calloc (sizeof (struct cc_usb), 1);
	if (!cc)
		return NULL;
	cc->fd = open(tty, O_RDWR | O_NONBLOCK);
	if (cc->fd < 0) {
		perror(tty);
		free (cc);
		return NULL;
	}
	tcgetattr(cc->fd, &termios);
	save_termios = termios;
	cfmakeraw(&termios);
	tcsetattr(cc->fd, TCSAFLUSH, &termios);
	cc_usb_printf(cc, "E 0\nm 0\n");
	cc_usb_sync(cc);
	sleep(1);
	cc_usb_sync(cc);
	return cc;
}

void
cc_usb_close(struct cc_usb *cc)
{
	tcsetattr(cc->fd, TCSAFLUSH, &save_termios);
	close (cc->fd);
	free (cc);
}
