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
	{ .name = "remote", .has_arg = 0, .val = 'R' },
	{ .name = "frequency", .has_arg = 1, .val = 'F' },
	{ .name = "call", .has_arg = 1, .val = 'C' },
	{ .name = "output", .has_arg = 1, .val = 'o' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>] [--remote] [--frequency <radio-frequency>] [--call <radio-callsign>]\n", program);
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
	int		serial_number = 0;
	int		freq = 434550;
	char		*call = "N0CALL";
	int		flight = 0;
	char		cmd;
	int		block;
	int		addr;
	int		received_addr;
	int		data[8];
	int		done;
	int		i;
	int		column;
	int		remote = 0;
	int		any_valid;
	int		invalid;
	int		storage_size = 0;
	char		*out_name;

	while ((c = getopt_long(argc, argv, "T:D:F:C:o:R", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'R':
			remote = 1;
			break;
		case 'F':
			freq = atoi(optarg);
			break;
		case 'C':
			call = optarg;
			break;
		case 'o':
			out_name = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (!tty) {
		if (remote)
			tty = cc_usbdevs_find_by_arg(device, "TeleDongle");
		else
			tty = cc_usbdevs_find_by_arg(device, "TeleMetrum");
	}
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	cc = cc_usb_open(tty);
	if (!cc)
		exit(1);
	if (remote)
		cc_usb_open_remote(cc, freq, call);

	if (out_name) {
		out = fopen(out_name, "w");
		if (!out) {
			perror(out_name);
			cc_usb_close(cc);
			exit(1);
		}
	} else
		out = stdout;

	/* send a 'version' command followed by a 'flash' command */
	cc_usb_printf(cc, "f\nv\n");
	for (;;) {
		cc_usb_getline(cc, line, sizeof (line));
		if (sscanf(line, "serial-number %u", &serial_number) == 1)
			continue;
		if (sscanf(line, "Storage size: %u", &storage_size) == 1)
			continue;
		if (!strncmp(line, "software-version", 16))
			break;
	}
	if (!serial_number) {
		fprintf(stderr, "no serial number found\n");
		cc_usb_close(cc);
		exit(1);
	}
	if (!storage_size) {
		fprintf(stderr, "no storage size found\n");
		cc_usb_close(cc);
		exit(1);
	}
	printf ("Serial number: %d\n", serial_number);
	printf ("Storage size:  %d\n", storage_size);
	fprintf (stderr, "%7d of %7d", 0, storage_size/256);
	for (block = 0; block < storage_size / 256; block++) {
		cc_usb_printf(cc, "e %x\n", block);
		fprintf (stderr, "\r%7d of %7d", block + 1, storage_size/256); fflush(stderr);
		for (addr = 0; addr < 0x100;) {
			cc_usb_getline(cc, line, sizeof (line));
			if (sscanf(line, "00%x %x %x %x %x %x %x %x %x",
					  &received_addr,
					  &data[0], &data[1], &data[2], &data[3],
					  &data[4], &data[5], &data[6], &data[7]) == 9)
			{
				if (received_addr != addr)
					fprintf(stderr, "data out of sync at 0x%x\n",
						block * 256 + received_addr);

				fprintf (out, "%08x", block * 256 + addr);
				for (i = 0; i < 8; i++)
					fprintf (out, " %02x", data[i]);
				fprintf (out, "\n");

				addr += 8;
			}
		}
	}
	fprintf(stderr, "\n");
	cc_usb_close(cc);
	exit (0);
}
