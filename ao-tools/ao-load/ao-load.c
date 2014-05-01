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

#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include "ccdbg.h"
#include "cc.h"

#define AO_USB_DESC_STRING		3

static struct sym {
	unsigned	addr;
	char		*name;
	int		required;
} ao_symbols[] = {
	{ 0,	"_ao_serial_number", 1 },
#define AO_SERIAL_NUMBER	(ao_symbols[0].addr)
	{ 0,	"_ao_usb_descriptors", 0 },
#define AO_USB_DESCRIPTORS	(ao_symbols[1].addr)
	{ 0,	"_ao_radio_cal", 1 },
#define AO_RADIO_CAL		(ao_symbols[2].addr)
};

#define NUM_SYMBOLS		3
#define NUM_REQUIRED_SYMBOLS	2

static int
find_symbols(FILE *map)
{
	char	line[2048];
	char	*addr, *addr_end;
	char	*name;
	char	*save;
	char	*colon;
	unsigned long	a;
	int	s;
	int	required = 0;

	while (fgets(line, sizeof(line), map) != NULL) {
		line[sizeof(line)-1] = '\0';
		addr = strtok_r(line, " \t\n", &save);
		if (!addr)
			continue;
		name = strtok_r(NULL, " \t\n", &save);
		if (!name)
			continue;
		colon = strchr (addr, ':');
		if (!colon)
			continue;
		a = strtoul(colon+1, &addr_end, 16);
		if (a == ULONG_MAX || addr_end == addr)
			continue;
		for (s = 0; s < NUM_SYMBOLS; s++)
			if (!strcmp(ao_symbols[s].name, name)) {
				ao_symbols[s].addr = (unsigned) a;
				if (ao_symbols[s].required)
					++required;
				break;
			}
	}
	return required >= NUM_REQUIRED_SYMBOLS;
}

static int
rewrite(struct ao_hex_image *image, unsigned addr, char *data, int len)
{
	int i;
	if (addr < image->address || image->address + image->length < addr + len)
		return 0;
	printf("rewrite %04x:", addr);
	for (i = 0; i < len; i++)
		printf (" %02x", image->data[addr - image->address + i]);
	printf(" ->");
	for (i = 0; i < len; i++)
		printf (" %02x", data[i]);
	printf("\n");
	memcpy(image->data + addr - image->address, data, len);
}

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "cal", .has_arg = 1, .val = 'c' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty=<tty-name>] [--device=<device-name>] [--cal=<radio-cal>] file.ihx serial-number\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	pc;
	struct ao_hex_file	*hex;
	struct ao_hex_image *image;
	char		*filename;
	FILE		*file;
	FILE		*map;
	char		*serial_string;
	unsigned int	serial;
	char		*mapname, *dot;
	char		*serial_ucs2;
	int		serial_ucs2_len;
	char		serial_int[2];
	unsigned int	s;
	int		i;
	int		string_num;
	char		*tty = NULL;
	char		*device = NULL;
	uint32_t	cal = 0;
	char		cal_int[4];
	char		*cal_end;
	int		c;

	while ((c = getopt_long(argc, argv, "T:D:c:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'c':
			cal = strtoul(optarg, &cal_end, 10);
			if (cal_end == optarg || *cal_end != '\0')
				usage(argv[0]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	filename = argv[optind];
	if (filename == NULL)
		usage(argv[0]);
	mapname = strdup(filename);
	dot = strrchr(mapname, '.');
	if (!dot || strcmp(dot, ".ihx") != 0)
		usage(argv[0]);
	strcpy(dot, ".map");

	serial_string = argv[optind + 1];
	if (serial_string == NULL)
		usage(argv[0]);

	file = fopen(filename, "r");
	if (!file) {
		perror(filename);
		exit(1);
	}
	map = fopen(mapname, "r");
	if (!map) {
		perror(mapname);
		exit(1);
	}
	if (!find_symbols(map)) {
		fprintf(stderr, "Cannot find symbols in \"%s\"\n", mapname);
		exit(1);
	}
	fclose(map);

	hex = ao_hex_file_read(file, filename);
	fclose(file);
	if (!hex) {
		perror(filename);
		exit (1);
	}
	image = ao_hex_image_create(hex);
	if (!image) {
		fprintf(stderr, "image create failed\n");
		exit (1);
	}
	ao_hex_file_free(hex);

	serial = strtoul(serial_string, NULL, 0);
	if (!serial)
		usage(argv[0]);

	serial_int[0] = serial & 0xff;
	serial_int[1] = (serial >> 8) & 0xff;

	if (!rewrite(image, AO_SERIAL_NUMBER, serial_int, sizeof (serial_int))) {
		fprintf(stderr, "Cannot rewrite serial integer at %04x\n",
			AO_SERIAL_NUMBER);
		exit(1);
	}

	if (AO_USB_DESCRIPTORS) {
		unsigned	usb_descriptors;
		usb_descriptors = AO_USB_DESCRIPTORS - image->address;
		string_num = 0;
		while (image->data[usb_descriptors] != 0 && usb_descriptors < image->length) {
			if (image->data[usb_descriptors+1] == AO_USB_DESC_STRING) {
				++string_num;
				if (string_num == 4)
					break;
			}
			usb_descriptors += image->data[usb_descriptors];
		}
		if (usb_descriptors >= image->length || image->data[usb_descriptors] == 0 ) {
			fprintf(stderr, "Cannot rewrite serial string at %04x\n", AO_USB_DESCRIPTORS);
			exit(1);
		}

		serial_ucs2_len = image->data[usb_descriptors] - 2;
		serial_ucs2 = malloc(serial_ucs2_len);
		if (!serial_ucs2) {
			fprintf(stderr, "Malloc(%d) failed\n", serial_ucs2_len);
			exit(1);
		}
		s = serial;
		for (i = serial_ucs2_len / 2; i; i--) {
			serial_ucs2[i * 2 - 1] = 0;
			serial_ucs2[i * 2 - 2] = (s % 10) + '0';
			s /= 10;
		}
		if (!rewrite(image, usb_descriptors + 2 + image->address, serial_ucs2, serial_ucs2_len))
			usage(argv[0]);
	}

	if (cal) {
		cal_int[0] = cal & 0xff;
		cal_int[1] = (cal >> 8) & 0xff;
		cal_int[2] = (cal >> 16) & 0xff;
		cal_int[3] = (cal >> 24) & 0xff;
		if (!AO_RADIO_CAL) {
			fprintf(stderr, "Cannot find radio calibration location in image\n");
			exit(1);
		}
		if (!rewrite(image, AO_RADIO_CAL, cal_int, sizeof (cal_int))) {
			fprintf(stderr, "Cannot rewrite radio calibration at %04x\n", AO_RADIO_CAL);
			exit(1);
		}
	}
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
		ao_hex_image_free(image);
		ccdbg_close(dbg);
		exit(1);
	}
	ccdbg_set_pc(dbg, image->address);
	ccdbg_resume(dbg);
	ccdbg_close(dbg);
	exit (0);
}
