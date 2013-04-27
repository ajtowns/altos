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

#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "stlink-common.h"
#include "ao-elf.h"
#include "ccdbg.h"
#include "cc-usb.h"
#include "cc.h"
#include "ao-stmload.h"

#define AO_USB_DESC_STRING		3

struct sym ao_symbols[] = {

	{ 0, AO_BOOT_APPLICATION_BASE + 0x100,	"ao_romconfig_version",	1 },
#define AO_ROMCONFIG_VERSION	(ao_symbols[0].addr)

	{ 0, AO_BOOT_APPLICATION_BASE + 0x102,	"ao_romconfig_check",	1 },
#define AO_ROMCONFIG_CHECK	(ao_symbols[1].addr)

	{ 0, AO_BOOT_APPLICATION_BASE + 0x104,	"ao_serial_number", 1 },
#define AO_SERIAL_NUMBER	(ao_symbols[2].addr)

	{ 0, AO_BOOT_APPLICATION_BASE + 0x108,	"ao_radio_cal", 0 },
#define AO_RADIO_CAL		(ao_symbols[3].addr)

	{ 0, AO_BOOT_APPLICATION_BASE + 0x10c,	"ao_usb_descriptors", 0 },
#define AO_USB_DESCRIPTORS	(ao_symbols[4].addr)
};

#define NUM_SYMBOLS		5
#define NUM_REQUIRED_SYMBOLS	3

int ao_num_symbols = NUM_SYMBOLS;
int ao_num_required_symbols = NUM_REQUIRED_SYMBOLS;

/*
 * Edit the to-be-written memory block
 */
static int
rewrite(struct hex_image *load, unsigned address, uint8_t *data, int length)
{
	int 		i;

	if (address < load->address || load->address + load->length < address + length)
		return 0;

	printf("rewrite %04x:", address);
	for (i = 0; i < length; i++)
		printf (" %02x", load->data[address - load->address + i]);
	printf(" ->");
	for (i = 0; i < length; i++)
		printf (" %02x", data[i]);
	printf("\n");
	memcpy(load->data + address - load->address, data, length);
}

/*
 * Read a 16-bit value from the USB target
 */

static uint16_t
get_uint16_cc(struct cc_usb *cc, uint32_t addr)
{
	struct hex_image	*hex = ao_self_read(cc, addr, 2);
	uint16_t		v;
	uint8_t			*data;

	if (!hex)
		return 0;
	data = hex->data + addr - hex->address;
	v = data[0] | (data[1] << 8);
	free(hex);
	return v;
}

static uint32_t
get_uint32_cc(struct cc_usb *cc, uint32_t addr)
{
	struct hex_image	*hex = ao_self_read(cc, addr, 4);
	uint32_t		v;
	uint8_t			*data;

	if (!hex)
		return 0;
	data = hex->data + addr - hex->address;
	v = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	free(hex);
	return v;
}

/*
 * Read a 16-bit value from the target device with arbitrary
 * alignment
 */
static uint16_t
get_uint16_sl(stlink_t *sl, uint32_t addr)
{
	const 		uint8_t *data = sl->q_buf;
	uint32_t	actual_addr;
	int		off;
	uint16_t	result;

	sl->q_len = 0;


	actual_addr = addr & ~3;
	
	stlink_read_mem32(sl, actual_addr, 8);

	if (sl->q_len != 8)
		abort();

	off = addr & 3;
	result = data[off] | (data[off + 1] << 8);
	return result;
}

static uint16_t
get_uint16(stlink_t *sl, struct cc_usb *cc, uint32_t addr)
{
	uint16_t	result;
	if (cc)
		result = get_uint16_cc(cc, addr);
	else
		result = get_uint16_sl(sl, addr);
	printf ("read 0x%08x = 0x%04x\n", addr, result);
	return result;
}

/*
 * Read a 32-bit value from the target device with arbitrary
 * alignment
 */
static uint32_t
get_uint32_sl(stlink_t *sl, uint32_t addr)
{
	const 		uint8_t *data = sl->q_buf;
	uint32_t	actual_addr;
	int		off;
	uint32_t	result;

	sl->q_len = 0;

	printf ("read 0x%x\n", addr);

	actual_addr = addr & ~3;
	
	stlink_read_mem32(sl, actual_addr, 8);

	if (sl->q_len != 8)
		abort();

	off = addr & 3;
	result = data[off] | (data[off + 1] << 8) | (data[off+2] << 16) | (data[off+3] << 24);
	return result;
}

/*
 * Read a 32-bit value from the target device with arbitrary
 * alignment
 */
static uint32_t
get_uint32(stlink_t *sl, struct cc_usb *cc, uint32_t addr)
{
	uint32_t	result;

	if (cc)
		result = get_uint32_cc(cc, addr);
	else
		result = get_uint32_sl(sl, addr);
	printf ("read 0x%08x = 0x%08x\n", addr, result);
	return result;
}

/*
 * Check to see if the target device has been
 * flashed with a similar firmware image before
 *
 * This is done by looking for the same romconfig version,
 * which should be at the same location as the linker script
 * places this at 0x100 from the start of the rom section
 */
static int
check_flashed(stlink_t *sl, struct cc_usb *cc)
{
	uint16_t	romconfig_version = get_uint16(sl, cc, AO_ROMCONFIG_VERSION);
	uint16_t	romconfig_check = get_uint16(sl, cc, AO_ROMCONFIG_CHECK);

	if (romconfig_version != (uint16_t) ~romconfig_check) {
		fprintf (stderr, "Device has not been flashed before\n");
		return 0;
	}
	return 1;
}

static const struct option options[] = {
	{ .name = "stlink", .has_arg = 0, .val = 'S' },
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "cal", .has_arg = 1, .val = 'c' },
	{ .name = "serial", .has_arg = 1, .val = 's' },
	{ .name = "verbose", .has_arg = 0, .val = 'v' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--stlink] [--verbose] [--device=<device>] [-tty=<tty>] [--cal=<radio-cal>] [--serial=<serial>] file.{elf,ihx}\n", program);
	exit(1);
}

void
done(stlink_t *sl, struct cc_usb *cc, int code)
{
	if (cc) {
/*		cc_usb_printf(cc, "a\n"); */
		cc_usb_close(cc);
	}
	if (sl) {
		stlink_reset(sl);
		stlink_run(sl);
		stlink_exit_debug_mode(sl);
		stlink_close(sl);
	}
	exit (code);
}

static int
ends_with(char *whole, char *suffix)
{
	int whole_len = strlen(whole);
	int suffix_len = strlen(suffix);

	if (suffix_len > whole_len)
		return 0;
	return strcmp(whole + whole_len - suffix_len, suffix) == 0;
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	char			*filename;
	Elf			*e;
	char			*serial_end;
	unsigned int		serial = 0;
	char			*serial_ucs2;
	int			serial_ucs2_len;
	char			serial_int[2];
	unsigned int		s;
	int			i;
	int			string_num;
	uint32_t		cal = 0;
	char			cal_int[4];
	char			*cal_end;
	int			c;
	stlink_t		*sl = NULL;
	int			was_flashed = 0;
	struct hex_image	*load;
	int			tries;
	struct cc_usb		*cc = NULL;
	int			use_stlink = 0;
	char			*tty = NULL;
	int			success;
	int			verbose = 0;

	while ((c = getopt_long(argc, argv, "T:D:c:s:Sv", options, NULL)) != -1) {
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
		case 's':
			serial = strtoul(optarg, &serial_end, 10);
			if (serial_end == optarg || *serial_end != '\0')
				usage(argv[0]);
			break;
		case 'S':
			use_stlink = 1;
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	ao_self_verbose = verbose;

	if (verbose > 1)
		ccdbg_add_debug(CC_DEBUG_BITBANG);

	filename = argv[optind];
	if (filename == NULL)
		usage(argv[0]);

	if (ends_with (filename, ".elf")) {
		load = ao_load_elf(filename);
	} else if (ends_with (filename, ".ihx")) {
		int	i;
		load = ccdbg_hex_load(filename);
		for (i = 0; i < ao_num_symbols; i++)
			ao_symbols[i].addr = ao_symbols[i].default_addr;
	} else
		usage(argv[0]);

	if (use_stlink) {
		/* Connect to the programming dongle
		 */
	
		for (tries = 0; tries < 3; tries++) {
			if (device) {
				sl = stlink_v1_open(50);
			} else {
				sl = stlink_open_usb(50);
		
			}
			if (!sl) {
				fprintf (stderr, "No STLink devices present\n");
				done (sl, NULL, 1);
			}

			if (sl->chip_id != 0)
				break;
			stlink_reset(sl);
			stlink_close(sl);
			sl = NULL;
		}
		if (!sl) {
			fprintf (stderr, "Debugger connection failed\n");
			exit(1);
		}

		/* Verify that the loaded image fits entirely within device flash
		 */
		if (load->address < sl->flash_base ||
		    sl->flash_base + sl->flash_size < load->address + load->length) {
			fprintf (stderr, "\%s\": Invalid memory range 0x%08x - 0x%08x\n", filename,
				 load->address, load->address + load->length);
			done(sl, NULL, 1);
		}

		/* Enter debugging mode
		 */
		if (stlink_current_mode(sl) == STLINK_DEV_DFU_MODE)
			stlink_exit_dfu_mode(sl);

		if (stlink_current_mode(sl) != STLINK_DEV_DEBUG_MODE)
			stlink_enter_swd_mode(sl);
	} else {
		int	is_loader;
		int	tries;

		for (tries = 0; tries < 3; tries++) {
			char	*this_tty = tty;
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "AltosFlash");
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "MegaMetrum");
			if (!this_tty)
				this_tty = getenv("ALTOS_TTY");
			if (!this_tty)
				this_tty="/dev/ttyACM0";

			cc = cc_usb_open(this_tty);

			if (!cc)
				exit(1);
			cc_usb_printf(cc, "v\n");
			is_loader = 0;
			for (;;) {
				char	line[256];
				cc_usb_getline(cc, line, sizeof(line));
				if (!strncmp(line, "altos-loader", 12))
					is_loader = 1;
				if (!strncmp(line, "software-version", 16))
					break;
			}
			if (is_loader)
				break;
			printf ("rebooting to loader\n");
			cc_usb_printf(cc, "X\n");
			cc_usb_close(cc);
			sleep(1);
			cc = NULL;
		}
		if (!is_loader) {
			fprintf(stderr, "Cannot switch to boot loader\n");
			exit(1);
		}
#if 0
		{
			uint8_t	check[256];
			int	i = 0;

			ao_self_block_read(cc, AO_BOOT_APPLICATION_BASE, check);
			for (;;) {
				uint8_t block[256];
				putchar ('.');
				if (++i == 40) {
					putchar('\n');
					i = 0;
				}
				fflush(stdout);
				ao_self_block_write(cc, AO_BOOT_APPLICATION_BASE, block);
				ao_self_block_read(cc, AO_BOOT_APPLICATION_BASE, block);
				if (memcmp(block, check, 256) != 0) {
					fprintf (stderr, "read differed\n");
					exit(1);
				}
			}
		}
#endif
	}

	/* Go fetch existing config values
	 * if available
	 */
	was_flashed = check_flashed(sl, cc);

	if (!serial) {
		if (!was_flashed) {
			fprintf (stderr, "Must provide serial number\n");
			done(sl, cc, 1);
		}
		serial = get_uint16(sl, cc, AO_SERIAL_NUMBER);
		if (!serial || serial == 0xffff) {
			fprintf (stderr, "Invalid existing serial %d\n", serial);
			done(sl, cc, 1);
		}
	}

	if (!cal && AO_RADIO_CAL && was_flashed) {
		cal = get_uint32(sl, cc, AO_RADIO_CAL);
		if (!cal || cal == 0xffffffff) {
			fprintf (stderr, "Invalid existing rf cal %d\n", cal);
			done(sl, cc, 1);
		}
	}

	/* Write the config values into the flash image
	 */

	serial_int[0] = serial & 0xff;
	serial_int[1] = (serial >> 8) & 0xff;

	if (!rewrite(load, AO_SERIAL_NUMBER, serial_int, sizeof (serial_int))) {
		fprintf(stderr, "Cannot rewrite serial integer at %08x\n",
			AO_SERIAL_NUMBER);
		done(sl, cc, 1);
	}

	if (AO_USB_DESCRIPTORS) {
		uint32_t	usb_descriptors = AO_USB_DESCRIPTORS - load->address;
		string_num = 0;

		while (load->data[usb_descriptors] != 0 && usb_descriptors < load->length) {
			if (load->data[usb_descriptors+1] == AO_USB_DESC_STRING) {
				++string_num;
				if (string_num == 4)
					break;
			}
			usb_descriptors += load->data[usb_descriptors];
		}
		if (usb_descriptors >= load->length || load->data[usb_descriptors] == 0 ) {
			fprintf(stderr, "Cannot rewrite serial string at %08x\n", AO_USB_DESCRIPTORS);
			done(sl, cc, 1);
		}

		serial_ucs2_len = load->data[usb_descriptors] - 2;
		serial_ucs2 = malloc(serial_ucs2_len);
		if (!serial_ucs2) {
			fprintf(stderr, "Malloc(%d) failed\n", serial_ucs2_len);
			done(sl, cc, 1);
		}
		s = serial;
		for (i = serial_ucs2_len / 2; i; i--) {
			serial_ucs2[i * 2 - 1] = 0;
			serial_ucs2[i * 2 - 2] = (s % 10) + '0';
			s /= 10;
		}
		if (!rewrite(load, usb_descriptors + 2 + load->address, serial_ucs2, serial_ucs2_len)) {
			fprintf (stderr, "Cannot rewrite USB descriptor at %08x\n", AO_USB_DESCRIPTORS);
			done(sl, cc, 1);
		}
	}

	if (cal && AO_RADIO_CAL) {
		cal_int[0] = cal & 0xff;
		cal_int[1] = (cal >> 8) & 0xff;
		cal_int[2] = (cal >> 16) & 0xff;
		cal_int[3] = (cal >> 24) & 0xff;

		if (!rewrite(load, AO_RADIO_CAL, cal_int, sizeof (cal_int))) {
			fprintf(stderr, "Cannot rewrite radio calibration at %08x\n", AO_RADIO_CAL);
			exit(1);
		}
	}

	/* And flash the resulting image to the device
	 */
	if (cc)
		success = ao_self_write(cc, load);
	else
		success = (stlink_write_flash(sl, load->address, load->data, load->length) >= 0);
		
	if (!success) {
		fprintf (stderr, "\"%s\": Write failed\n", filename);
		done(sl, cc, 1);
	}

	done(sl, cc, 0);
}
