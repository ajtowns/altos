/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cc.h"
#include "cc-usb.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "frequency", .has_arg = 1, .val = 'F' },
	{ .name = "realtime", .has_arg = 0, .val = 'R' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>] [--frequency <kHz>] [--realtime] file.telem ...\n", program);
	exit(1);
}

#define bool(b)	((b) ? "true" : "false")

struct ao_telem_list {
	struct ao_telem_list	*next;
	union ao_telemetry_all	telem;
};

static struct ao_telem_list	*telem_list, **telem_last;

static void
trim_telem(uint16_t time)
{
	while (telem_list && (int16_t) (time - telem_list->telem.generic.tick) > 0) {
		struct ao_telem_list	*next = telem_list->next;
		free(telem_list);
		telem_list = next;
	}
	if (!telem_list)
		telem_last = &telem_list;
}

static void
add_telem(union ao_telemetry_all *telem)
{
	struct ao_telem_list	*new = malloc (sizeof (struct ao_telem_list));
	trim_telem((uint16_t) (telem->generic.tick - 20 * 100));
	new->telem = *telem;
	new->next = 0;
	*telem_last = new;
	telem_last = &new->next;
}

static enum ao_flight_state	cur_state = ao_flight_invalid;
static enum ao_flight_state	last_state = ao_flight_invalid;

static enum ao_flight_state
packet_state(union ao_telemetry_all *telem)
{
	switch (telem->generic.type) {
	case AO_TELEMETRY_SENSOR_TELEMETRUM:
	case AO_TELEMETRY_SENSOR_TELEMINI:
	case AO_TELEMETRY_SENSOR_TELENANO:
		cur_state = telem->sensor.state;
		break;
	case AO_TELEMETRY_MEGA_DATA:
		cur_state = telem->mega_data.state;
		break;
	}
	return cur_state;
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

static void
send_telem(struct cc_usb *cc, union ao_telemetry_all *telem)
{
	int 	rssi = (int8_t) telem->generic.rssi / 2 - 74;
	int	i;
	uint8_t	*b;

	packet_state(telem);
	if (cur_state != last_state) {
		if (0 <= cur_state && cur_state < sizeof(state_names) / sizeof (state_names[0]))
			printf ("%s\n", state_names[cur_state]);
		last_state = cur_state;
	}
	cc_usb_printf(cc, "S 20\n");
	b = (uint8_t *) telem;
	for (i = 0; i < 0x20; i++)
		cc_usb_printf(cc, "%02x", b[i]);
	cc_usb_sync(cc);
}	

static void
do_delay(uint16_t now, uint16_t then)
{
	int16_t	delay = (int16_t) (now - then);

	if (delay > 0 && delay < 1000)
		usleep(delay * 10 * 1000);
}

static uint16_t
send_queued(struct cc_usb *cc, int pause)
{
	struct ao_telem_list	*next;
	uint16_t		tick = 0;
	int			started = 0;

	while (telem_list) {
		if (started && pause)
			do_delay(telem_list->telem.generic.tick, tick);
		tick = telem_list->telem.generic.tick;
		started = 1;
		send_telem(cc, &telem_list->telem);

		next = telem_list->next;
		free(telem_list);
		telem_list = next;
	}
	return tick;
}

int
main (int argc, char **argv)
{
	struct cc_usb	*cc;
	char		*tty = NULL;
	char		*device = NULL;
	char		line[80];
	int 		c, i, ret = 0;
	int		freq = 434550;
	char 		*s;
	FILE 		*file;
	int 		serial;
	uint16_t	last_tick;
	int		started;
	int		realtime = 0;
      

	while ((c = getopt_long(argc, argv, "RT:D:F:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'F':
			freq = atoi(optarg);
			break;
		case 'R':
			realtime = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TeleDongle");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";
	cc = cc_usb_open(tty);
	if (!cc)
		exit (1);

	cc_usb_printf(cc, "m 0\n");
	cc_usb_printf(cc, "c F %d\n", freq);
	for (i = optind; i < argc; i++) {
		file = fopen(argv[i], "r");
		if (!file) {
			perror(argv[i]);
			ret++;
			continue;
		}
		started = 0;
		last_tick = 0;
		while (fgets(line, sizeof (line), file)) {
			union ao_telemetry_all telem;

			if (cc_telemetry_parse(line, &telem)) {
				/*
				 * Skip packets with CRC errors.
				 */
				if ((telem.generic.status & (1 << 7)) == 0)
					continue;

				if (started) {
					do_delay(telem.generic.tick, last_tick);
					last_tick = telem.generic.tick;
					send_telem(cc, &telem);
				} else {
					enum ao_flight_state state = packet_state(&telem);
					add_telem(&telem);
					if (ao_flight_pad < state && state < ao_flight_landed) {
						printf ("started\n");
						started = 1;
						last_tick = send_queued(cc, realtime);
					}
				}
			}
		}
		fclose (file);

	}
	return ret;
}
