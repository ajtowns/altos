/*
 * Copyright © 2008 Keith Packard <keithp@keithp.com>
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

#include <unistd.h>
#include <getopt.h>
#include "ccdbg.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>] file.ihx\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	pc;
	struct hex_file	*hex;
	struct hex_image *image;
	char		*filename;
	FILE		*file;
	char		*tty = NULL;
	char		*device = NULL;
	int		c;

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
	filename = argv[optind];
	if (filename == NULL) {
		fprintf(stderr, "usage: %s <filename.ihx>\n", argv[0]);
		exit(1);
	}
	file = fopen(filename, "r");
	if (!file) {
		perror(filename);
		exit(1);
	}
	hex = ccdbg_hex_file_read(file, filename);
	fclose(file);
	if (!hex)
		exit (1);
	image = ccdbg_hex_image_create(hex);
	if (!image) {
		fprintf(stderr, "image create failed\n");
		exit (1);
	}

	ccdbg_hex_file_free(hex);
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TIDongle");
	dbg = ccdbg_open(tty);
	if (!dbg)
		exit (1);

	ccdbg_add_debug(CC_DEBUG_FLASH);

	ccdbg_debug_mode(dbg);
	ccdbg_halt(dbg);
	if (image->address == 0xf000) {
		printf("Loading %d bytes to execute from RAM\n",
		       image->length);
		ccdbg_write_hex_image(dbg, image, 0);
	} else if (image->address == 0x0000) {
		printf("Loading %d bytes to execute from FLASH\n",
		       image->length);
		ccdbg_flash_hex_image(dbg, image);
	} else {
		printf("Cannot load code to 0x%04x\n",
		       image->address);
		ccdbg_hex_image_free(image);
		ccdbg_close(dbg);
		exit(1);
	}
	ccdbg_set_pc(dbg, image->address);
	ccdbg_resume(dbg);
	ccdbg_close(dbg);
	exit (0);
}
