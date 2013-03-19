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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "cc-usb.h"
#include "cc.h"

#define NUM_BLOCK	512

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>]\n", program);
	exit(1);
}

static uint8_t
log_checksum(int d[8])
{
	uint8_t	sum = 0x5a;
	int	i;

	for (i = 0; i < 8; i++)
		sum += (uint8_t) d[i];
	return -sum;
}

static int get_nonwhite(struct cc_usb *cc, int timeout)
{
	int	c;

	for (;;) {
		c = cc_usb_getchar_timeout(cc, timeout);
		putchar(c);
		if (!isspace(c))
			return c;
	}
}

static uint8_t
get_hexc(struct cc_usb *cc)
{
	int	c = get_nonwhite(cc, 1000);

	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	fprintf(stderr, "Non-hex char '%c'\n", c);
	exit(1);
}

static int file_crc;

static const int POLY = 0x8408;

static int
log_crc(int crc, int b)
{
	int	i;

	for (i = 0; i < 8; i++) {
		if (((crc & 0x0001) ^ (b & 0x0001)) != 0)
			crc = (crc >> 1) ^ POLY;
		else
			crc = crc >> 1;
		b >>= 1;
	}
	return crc & 0xffff;
}

static uint8_t
get_hex(struct cc_usb *cc)
{
	int	a = get_hexc(cc);
	int	b = get_hexc(cc);
	int	h = (a << 4) + b;

	file_crc = log_crc(file_crc, h);
	return h;
}

static int get_32(struct cc_usb *cc)
{
	int	v = 0;
	int	i;
	for (i = 0; i < 4; i++) {
		v += get_hex(cc) << (i * 8);
	}
	return v;
}

static int get_16(struct cc_usb *cc)
{
	int	v = 0;
	int	i;
	for (i = 0; i < 2; i++) {
		v += get_hex(cc) << (i * 8);
	}
	return v;
}

static int swap16(int i)
{
	return ((i << 8) & 0xff00) | ((i >> 8) & 0xff);
}

static int find_header(struct cc_usb *cc)
{
	for (;;) {
		if (get_nonwhite(cc, 0) == 'M' && get_nonwhite(cc, 1000) == 'P')
			return 1;
	}
}

static const char *state_names[] = {
	"startup",
	"idle",
	"pad",
	"boost",
	"fast",
	"coast",
	"drogue",
	"main",
	"landed",
	"invalid"
};


int
main (int argc, char **argv)
{
	struct cc_usb	*cc;
	char		*tty = NULL;
	char		*device = NULL;
	int		c;
	char		line[8192];
	int		nsamples;
	int		i;
	int		crc;
	int		current_crc;

	while ((c = getopt_long(argc, argv, "T:D:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "FT230X Basic UART");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyUSB0";
	cc = cc_usb_open(tty);
	if (!cc)
		exit(1);
	find_header(cc);
	file_crc = 0xffff;
	get_32(cc);	/* ground pressure */
	get_32(cc);	/* min pressure */
	nsamples = get_16(cc);	/* nsamples */
	for (i = 0; i < nsamples; i++)
		get_16(cc);	/* sample i */
	current_crc = swap16(~file_crc & 0xffff);
	crc = get_16(cc);	/* crc */
	putchar ('\n');
	if (crc == current_crc)
		printf("CRC valid\n");
	else
		printf("CRC invalid\n");
	cc_usb_close(cc);
	exit (0);
}
