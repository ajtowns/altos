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
	{ .name = "channel", .has_arg = 1, .val = 'C' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>] [--remote] [--channel <radio-channel>]\n", program);
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
	FILE		*out;
	char		*filename;
	int		serial_number = 0;
	int		channel = 0;
	int		flight = 0;
	char		cmd;
	int		tick, a, b;
	int		block;
	int		addr;
	int		received_addr;
	int		data[8];
	int		done;
	int		column;
	int		remote = 0;
	int		any_valid;
	int		invalid;
	char		serial_line[8192];

	while ((c = getopt_long(argc, argv, "T:D:C:R", options, NULL)) != -1) {
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
		case 'C':
			channel = atoi(optarg);
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
		cc_usb_open_remote(cc, channel);
	/* send a 'version' command followed by a 'log' command */
	cc_usb_printf(cc, "v\n");
	out = NULL;
	for (;;) {
		cc_usb_getline(cc, line, sizeof (line));
		if (sscanf(line, "serial-number %u", &serial_number) == 1)
			strcpy(serial_line, line);
		if (!strncmp(line, "software-version", 16))
			break;
	}
	if (!serial_number) {
		fprintf(stderr, "no serial number found\n");
		cc_usb_close(cc);
		exit(1);
	}
	printf ("Serial number: %d\n", serial_number);
	done = 0;
	column = 0;
	for (block = 0; !done && block < 511; block++) {
		cc_usb_printf(cc, "e %x\n", block);
		if (column == 64) {
			putchar('\n');
			column = 0;
		}
		putchar('.'); fflush(stdout); column++;
		any_valid = 0;
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

				if (log_checksum(data) != 0)
					fprintf (stderr, "invalid checksum at 0x%x\n",
						 block * 256 + received_addr);
				else
					any_valid = 1;

				cmd = data[0];
				tick = data[2] + (data[3] << 8);
				a = data[4] + (data[5] << 8);
				b = data[6] + (data[7] << 8);
				if (cmd == 'F') {
					flight = b;
					filename = cc_make_filename(serial_number, flight, "eeprom");
					printf ("Flight:       %d\n", flight);
					printf ("File name:     %s\n", filename);
					out = fopen (filename, "w");
					if (!out) {
						perror(filename);
						exit(1);
					}
					fprintf(out, "%s\n", serial_line);
				}

				if (cmd == 'S' && a <= 8) {
					if (column) putchar('\n');
					printf("%s\n", state_names[a]);
					column = 0;
				}
				if (out) {
					fprintf(out, "%c %4x %4x %4x\n",
						cmd, tick, a, b);
					if (cmd == 'S' && a == 8) {
						fclose(out);
						out = NULL;
						done = 1;
					}
				}
				addr += 8;
			}
		}
		if (!any_valid) {
			fclose(out);
			out = NULL;
			done = 1;
		}
	}
	if (column)
		putchar('\n');
	if (out)
		fclose (out);
	cc_usb_close(cc);
	exit (0);
}
