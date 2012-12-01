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

#include "sky_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <getopt.h>
#include "cc.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "loader", .has_arg = 1, .val = 'l' },
	{ .name = "firmware", .has_arg = 1, .val = 'f' },
	{ .name = "query", .has_arg = 0, .val = 'q' },
	{ .name = "raw", .has_arg = 0, .val = 'r' },
	{ .name = "quiet", .has_arg = 0, .val = 'Q' },
	{ 0, 0, 0, 0},
};

static uint8_t	query_version[] = {
	0xa0, 0xa1, 0x00, 0x02, 0x02, 0x01, 0x03, 0x0d, 0x0a
};

static void
usage(char *program)
{
	fprintf(stderr,
		"usage: %s [--tty <tty-name>]\n"
		"          [--device <device-name>]\n"
		"          [--loader <srec bootloader file>]\n"
		"          [--firmware <binary firmware file>]\n"
		"          [--query]\n"
		"          [--quiet]\n"
		"          [--raw]\n", program);
	exit(1);
}

int
skytraq_expect(int fd, uint8_t want, int timeout) {
	int	c;

	c = skytraq_waitchar(fd, timeout);
	if (c < 0)
		return -1;
	if (c == want)
		return 1;
	return 0;
}

int
skytraq_wait_reply(int fd, uint8_t reply, uint8_t *buf, uint8_t reply_len) {

	for(;;) {
		uint8_t	a, b;
		uint8_t	cksum_computed, cksum_read;
		int	len;
		switch (skytraq_expect(fd, 0xa0, 10000)) {
		case -1:
			return -1;
		case 0:
			continue;
		case 1:
			break;
		}
		switch (skytraq_expect(fd, 0xa1, 1000)) {
		case -1:
			return -1;
		case 0:
			continue;
		}
		a = skytraq_waitchar(fd, 1000);
		b = skytraq_waitchar(fd, 1000);
		switch (skytraq_expect(fd, reply, 1000)) {
		case -1:
			return -1;
		case 0:
			continue;
		}
		len = (a << 16) | b;
		if (len != reply_len)
			continue;
		*buf++ = reply;
		len--;
		cksum_computed = reply;
		while (len--) {
			a = skytraq_waitchar(fd, 1000);
			if (a < 0)
				return a;
			cksum_computed ^= a;
			*buf++ = a;
		}
		switch (skytraq_expect(fd, cksum_computed, 1000)) {
		case -1:
			return -1;
		case 0:
			continue;
		}
		switch (skytraq_expect(fd, 0x0d, 1000)) {
		case -1:
			return -1;
		case 0:
			continue;
		}
		switch (skytraq_expect(fd, 0x0a, 1000)) {
		case -1:
			return -1;
		case 0:
			continue;
		}
		break;
	}
	return 0;
}

int
main(int argc, char **argv)
{
	int	fd;
	char	buf[512];
	int	ret;
	FILE	*input;
	long	size;
	unsigned char	cksum;
	int	c;
	char	message[1024];
	char	*tty = NULL;
	char	*device = NULL;
	char	*loader = "srec_115200.bin";
	char	*file = NULL;
	int	query = 0;
	int	raw = 0;

	while ((c = getopt_long(argc, argv, "T:D:l:f:qQr", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'l':
			loader = optarg;
			break;
		case 'f':
			file = optarg;
			break;
		case 'q':
			query = 1;
			break;
		case 'Q':
			skytraq_verbose = 0;
			break;
		case 'r':
			raw = 1;
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
	fd = skytraq_open(tty);
	if (fd < 0)
		exit(1);

	if (raw) {
		/* Set the baud rate to 115200 */
		skytraq_setcomm(fd, 115200);
		sleep(1);
		skytraq_setspeed(fd, 115200);
	} else {
		/* Connect TM to the device */
		skytraq_write(fd, "U\n", 2);
	}

	/* Wait for the device to stabilize after baud rate changes */
	for (c = 0; c < 6; c++) {
		skytraq_flush(fd);
		sleep(1);
	}

	if (query) {
		uint8_t	query_reply[14];

		uint8_t		software_type;
		uint32_t	kernel_version;
		uint32_t	odm_version;
		uint32_t	revision;

		skytraq_write(fd, query_version, 9);
		if (skytraq_wait_reply(fd, 0x80, query_reply, sizeof (query_reply)) != 0) {
			fprintf(stderr, "query reply failed\n");
			exit(1);
		}

#define i8(o)	query_reply[(o)-1]
#define i32(o)	((i8(o) << 24) | (i8(o+1) << 16) | (i8(o+2) << 8) | (i8(o+3)))
		software_type = i8(2);
		kernel_version = i32(3);
		odm_version = i32(7);
		revision = i32(11);
		skytraq_dbg_printf(0, "\n");
		printf ("Software Type %d. Kernel Version %d.%d.%d. ODM Version %d.%d.%d. Revision %d.%d.%d.\n",
			software_type,
			kernel_version >> 16 & 0xff,
			kernel_version >> 8 & 0xff,
			kernel_version >> 0 & 0xff,
			odm_version >> 16 & 0xff,
			odm_version >> 8 & 0xff,
			odm_version >> 0 & 0xff,
			revision >> 16 & 0xff,
			revision >> 8 & 0xff,
			revision >> 0 & 0xff);
		exit(0);
	}

	if (!file)
		usage(argv[0]);

	ret = skytraq_send_srec(fd, "srec_115200.bin");
	skytraq_dbg_printf (0, "srec ret %d\n", ret);
	if (ret < 0)
		exit(1);

	sleep(2);

//	ret = skytraq_send_bin(fd, "STI_01.04.42-01.10.23_4x_9600_Bin_20100901.bin");
	ret = skytraq_send_bin(fd, "STI_01.06.10-01.07.23_balloon_CRC_7082_9600_20120913.bin");

	printf ("bin ret %d\n", ret);
	if (ret < 0)
		exit(1);

	return 0;
}
