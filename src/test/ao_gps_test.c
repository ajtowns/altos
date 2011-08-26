/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#define AO_GPS_TEST
#include "ao_host.h"
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

struct ao_gps_orig {
	uint8_t			year;
	uint8_t			month;
	uint8_t			day;
	uint8_t			hour;
	uint8_t			minute;
	uint8_t			second;
	uint8_t			flags;
	int32_t			latitude;	/* degrees * 10⁷ */
	int32_t			longitude;	/* degrees * 10⁷ */
	int16_t			altitude;	/* m */
	uint16_t		ground_speed;	/* cm/s */
	uint8_t			course;		/* degrees / 2 */
	uint8_t			hdop;		/* * 5 */
	int16_t			climb_rate;	/* cm/s */
	uint16_t		h_error;	/* m */
	uint16_t		v_error;	/* m */
};

#define SIRF_SAT_STATE_ACQUIRED			(1 << 0)
#define SIRF_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define SIRF_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define SIRF_SAT_SUBFRAME_SYNC_COMPLETE		(1 << 3)
#define SIRF_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define SIRF_SAT_CODE_LOCKED			(1 << 5)
#define SIRF_SAT_ACQUISITION_FAILED		(1 << 6)
#define SIRF_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct ao_gps_sat_orig {
	uint8_t		svid;
	uint8_t		c_n_1;
};

#define AO_MAX_GPS_TRACKING	12

struct ao_gps_tracking_orig {
	uint8_t			channels;
	struct ao_gps_sat_orig	sats[AO_MAX_GPS_TRACKING];
};

#define ao_telemetry_location ao_gps_orig
#define ao_telemetry_satellite ao_gps_tracking_orig
#define ao_telemetry_satellite_info ao_gps_sat_orig

void
ao_mutex_get(uint8_t *mutex)
{
}

void
ao_mutex_put(uint8_t *mutex)
{
}

static int
ao_gps_fd;

static void
ao_dbg_char(char c)
{
	char	line[128];
	line[0] = '\0';
	if (c < ' ') {
		if (c == '\n')
			sprintf (line, "\n");
		else
			sprintf (line, "\\%02x", ((int) c) & 0xff);
	} else {
		sprintf (line, "%c", c);
	}
	write(1, line, strlen(line));
}

#define QUEUE_LEN	4096

static char	input_queue[QUEUE_LEN];
int		input_head, input_tail;

#include <sys/time.h>

int
get_millis(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void
check_sirf_message(char *from, uint8_t *msg, int len)
{
	uint16_t	encoded_len, encoded_cksum;
	uint16_t	cksum;
	uint8_t		id;
	int		i;

	if (msg[0] != 0xa0 || msg[1] != 0xa2) {
		printf ("bad header\n");
		return;
	}
	if (len < 7) {
		printf("short\n");
		return;
	}
	if (msg[len-1] != 0xb3 || msg[len-2] != 0xb0) {
		printf ("bad trailer\n");
		return;
	}
	encoded_len = (msg[2] << 8) | msg[3];
	id = msg[4];
/*	printf ("%9d: %3d\n", get_millis(), id); */
	if (encoded_len != len - 8) {
		if (id != 52)
			printf ("length mismatch (got %d, wanted %d)\n",
				len - 8, encoded_len);
		return;
	}
	encoded_cksum = (msg[len - 4] << 8) | msg[len-3];
	cksum = 0;
	for (i = 4; i < len - 4; i++)
		cksum = (cksum + msg[i]) & 0x7fff;
	if (encoded_cksum != cksum) {
		printf ("cksum mismatch (got %04x wanted %04x)\n",
			cksum, encoded_cksum);
		return;
	}
	id = msg[4];
	switch (id) {
	case 41:{
		int	off = 4;

		uint8_t		id;
		uint16_t	nav_valid;
		uint16_t	nav_type;
		uint16_t	week;
		uint32_t	tow;
		uint16_t	year;
		uint8_t		month;
		uint8_t		day;
		uint8_t		hour;
		uint8_t		minute;
		uint16_t	second;
		uint32_t	sat_list;
		int32_t		lat;
		int32_t		lon;
		int32_t		alt_ell;
		int32_t		alt_msl;
		int8_t		datum;
		uint16_t	sog;
		uint16_t	cog;
		int16_t		mag_var;
		int16_t		climb_rate;
		int16_t		heading_rate;
		uint32_t	h_error;
		uint32_t	v_error;
		uint32_t	t_error;
		uint16_t	h_v_error;

#define get_u8(u)	u = (msg[off]); off+= 1
#define get_u16(u)	u = (msg[off] << 8) | (msg[off + 1]); off+= 2
#define get_u32(u)	u = (msg[off] << 24) | (msg[off + 1] << 16) | (msg[off+2] << 8) | (msg[off+3]); off+= 4

		get_u8(id);
		get_u16(nav_valid);
		get_u16(nav_type);
		get_u16(week);
		get_u32(tow);
		get_u16(year);
		get_u8(month);
		get_u8(day);
		get_u8(hour);
		get_u8(minute);
		get_u16(second);
		get_u32(sat_list);
		get_u32(lat);
		get_u32(lon);
		get_u32(alt_ell);
		get_u32(alt_msl);
		get_u8(datum);
		get_u16(sog);
		get_u16(cog);
		get_u16(mag_var);
		get_u16(climb_rate);
		get_u16(heading_rate);
		get_u32(h_error);
		get_u32(v_error);
		get_u32(t_error);
		get_u16(h_v_error);


		printf ("Geodetic Navigation Data (41):\n");
		printf ("\tNav valid %04x\n", nav_valid);
		printf ("\tNav type %04x\n", nav_type);
		printf ("\tWeek %5d", week);
		printf (" TOW %9d", tow);
		printf (" %4d-%2d-%2d %02d:%02d:%07.4f\n",
			year, month, day,
			hour, minute, second / 1000.0);
		printf ("\tsats: %08x\n", sat_list);
		printf ("\tlat: %g", lat / 1.0e7);
		printf (" lon: %g", lon / 1.0e7);
		printf (" alt_ell: %g", alt_ell / 100.0);
		printf (" alt_msll: %g", alt_msl / 100.0);
		printf (" datum: %d\n", datum);
		printf ("\tground speed: %g", sog / 100.0);
		printf (" course: %g", cog / 100.0);
		printf (" climb: %g", climb_rate / 100.0);
		printf (" heading rate: %g\n", heading_rate / 100.0);
		printf ("\th error: %g", h_error / 100.0);
		printf (" v error: %g", v_error / 100.0);
		printf (" t error: %g", t_error / 100.0);
		printf (" h vel error: %g\n", h_v_error / 100.0);
		break;
	}
	case 4: {
		int off = 4;
		uint8_t		id;
		int16_t		gps_week;
		uint32_t	gps_tow;
		uint8_t		channels;
		int		j, k;

		get_u8(id);
		get_u16(gps_week);
		get_u32(gps_tow);
		get_u8(channels);

		printf ("Measured Tracker Data (4):\n");
		printf ("GPS week: %d\n", gps_week);
		printf ("GPS time of week: %d\n", gps_tow);
		printf ("channels: %d\n", channels);
		for (j = 0; j < 12; j++) {
			uint8_t	svid, azimuth, elevation;
			uint16_t state;
			uint8_t	c_n[10];
			get_u8(svid);
			get_u8(azimuth);
			get_u8(elevation);
			get_u16(state);
			for (k = 0; k < 10; k++) {
				get_u8(c_n[k]);
			}
			printf ("Sat %3d:", svid);
			printf (" aziumuth: %6.1f", azimuth * 1.5);
			printf (" elevation: %6.1f", elevation * 0.5);
			printf (" state: 0x%02x", state);
			printf (" c_n:");
			for (k = 0; k < 10; k++)
				printf(" %3d", c_n[k]);
			if (state & SIRF_SAT_STATE_ACQUIRED)
				printf(" acq,");
			if (state & SIRF_SAT_STATE_CARRIER_PHASE_VALID)
				printf(" car,");
			if (state & SIRF_SAT_BIT_SYNC_COMPLETE)
				printf(" bit,");
			if (state & SIRF_SAT_SUBFRAME_SYNC_COMPLETE)
				printf(" sub,");
			if (state & SIRF_SAT_CARRIER_PULLIN_COMPLETE)
				printf(" pullin,");
			if (state & SIRF_SAT_CODE_LOCKED)
				printf(" code,");
			if (state & SIRF_SAT_ACQUISITION_FAILED)
				printf(" fail,");
			if (state & SIRF_SAT_EPHEMERIS_AVAILABLE)
				printf(" ephem,");
			printf ("\n");
		}
		break;
	}
	default:
		return;
		printf ("%s %4d:", from, encoded_len);
		for (i = 4; i < len - 4; i++) {
			if (((i - 4) & 0xf) == 0)
				printf("\n   ");
			printf (" %3d", msg[i]);
		}
		printf ("\n");
	}
}

static uint8_t	sirf_message[4096];
static int	sirf_message_len;
static uint8_t	sirf_in_message[4096];
static int	sirf_in_len;

char
ao_serial_getchar(void)
{
	char	c;
	uint8_t	uc;

	while (input_head == input_tail) {
		for (;;) {
			input_tail = read(ao_gps_fd, input_queue, QUEUE_LEN);
			if (input_tail < 0) {
				if (errno == EINTR || errno == EAGAIN)
					continue;
				perror ("getchar");
				exit (1);
			}
			input_head = 0;
			break;
		}
	}
	c = input_queue[input_head];
	input_head = (input_head + 1) % QUEUE_LEN;
	uc = c;
	if (sirf_in_len || uc == 0xa0) {
		if (sirf_in_len < 4096)
			sirf_in_message[sirf_in_len++] = uc;
		if (uc == 0xb3) {
			check_sirf_message("recv", sirf_in_message, sirf_in_len);
			sirf_in_len = 0;
		}
	}
	return c;
}


void
ao_serial_putchar(char c)
{
	int	i;
	uint8_t	uc = (uint8_t) c;

	if (sirf_message_len || uc == 0xa0) {
		if (sirf_message_len < 4096)
			sirf_message[sirf_message_len++] = uc;
		if (uc == 0xb3) {
			check_sirf_message("send", sirf_message, sirf_message_len);
			sirf_message_len = 0;
		}
	}
	for (;;) {
		i = write(ao_gps_fd, &c, 1);
		if (i == 1) {
			if ((uint8_t) c == 0xb3 || c == '\r') {
				static const struct timespec delay = {
					.tv_sec = 0,
					.tv_nsec = 100 * 1000 * 1000
				};
				tcdrain(ao_gps_fd);
//				nanosleep(&delay, NULL);
			}
			break;
		}
		if (i < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		perror("putchar");
		exit(1);
	}
}

#define AO_SERIAL_SPEED_4800	0
#define AO_SERIAL_SPEED_57600	1

static void
ao_serial_set_speed(uint8_t speed)
{
	int	fd = ao_gps_fd;
	struct termios	termios;

	tcdrain(fd);
	tcgetattr(fd, &termios);
	switch (speed) {
	case AO_SERIAL_SPEED_4800:
		cfsetspeed(&termios, B4800);
		break;
	case AO_SERIAL_SPEED_57600:
		cfsetspeed(&termios, B57600);
		break;
	}
	tcsetattr(fd, TCSAFLUSH, &termios);
	tcflush(fd, TCIFLUSH);
}

#define ao_time() 0

#include "ao_gps_print.c"
#include "ao_gps_sirf.c"

void
ao_dump_state(void *wchan)
{
	double	lat, lon;
	int	i;
	if (wchan == &ao_gps_data)
		ao_gps_print(&ao_gps_data);
	else
		ao_gps_tracking_print(&ao_gps_tracking_data);
	putchar('\n');
	return;
	printf ("%02d:%02d:%02d",
		ao_gps_data.hour, ao_gps_data.minute,
		ao_gps_data.second);
	printf (" nsat %d %svalid",
		ao_gps_data.flags & AO_GPS_NUM_SAT_MASK,
		ao_gps_data.flags & AO_GPS_VALID ? "" : "not ");
	printf (" lat %g lon %g alt %d",
		ao_gps_data.latitude / 1.0e7,
		ao_gps_data.longitude / 1.0e7,
		ao_gps_data.altitude);
	printf (" speed %g climb %g course %d",
		ao_gps_data.ground_speed / 100.0,
		ao_gps_data.climb_rate / 100.0,
		ao_gps_data.course * 2);
	printf (" hdop %g h_error %d v_error %d",
		ao_gps_data.hdop / 5.0,
		ao_gps_data.h_error, ao_gps_data.v_error);
	printf("\n");
	printf ("\t");
	for (i = 0; i < 12; i++)
		printf (" %2d(%02d)",
			ao_gps_tracking_data.sats[i].svid,
			ao_gps_tracking_data.sats[i].c_n_1);
	printf ("\n");
}

int
ao_gps_open(const char *tty)
{
	struct termios	termios;
	int fd;

	fd = open (tty, O_RDWR);
	if (fd < 0)
		return -1;

	tcgetattr(fd, &termios);
	cfmakeraw(&termios);
	cfsetspeed(&termios, B4800);
	tcsetattr(fd, TCSAFLUSH, &termios);

	tcdrain(fd);
	tcflush(fd, TCIFLUSH);
	return fd;
}

#include <getopt.h>

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>]\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	char	*tty = "/dev/ttyUSB0";
	int	c;

	while ((c = getopt_long(argc, argv, "T:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	ao_gps_fd = ao_gps_open(tty);
	if (ao_gps_fd < 0) {
		perror (tty);
		exit (1);
	}
	ao_gps_setup();
	ao_gps();
}
