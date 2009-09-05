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
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	struct cc_usb	*cc;
	char		*tty = NULL;
	char		*device = NULL;
	int		c;
	char		line[8192];
	FILE		*out;
	char		*filename;
	int		serial_number;
	char		cmd;
	int		tick, a, b;

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
		tty = cc_usbdevs_find_by_arg(device, "TeleMetrum");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";
	cc = cc_usb_open(tty);
	if (!cc)
		exit(1);
	/* send a 'version' command followed by a 'log' command */
	cc_usb_printf(cc, "v\nl\n");
	out = NULL;
	for (;;) {
		cc_usb_getline(cc, line, sizeof (line));
		if (!strcmp (line, "end"))
			break;
		if (sscanf(line, "serial-number %u", &serial_number) == 1) {
			filename = cc_make_filename(serial_number, "eeprom");
			out = fopen (filename, "w");
			if (!out) {
				perror(filename);
			}
		} else if (sscanf(line, "%c %x %x %x", &cmd, &tick, &a, &b) == 4) {
			if (out) {
				fprintf(out, "%s\n", line);
				if (cmd == 'S' && a == 8) {
					fclose(out);
					out = NULL;
				}
			}
		}
	}
	if (out)
		fclose (out);
	cc_usb_close(cc);
	exit (0);
}
