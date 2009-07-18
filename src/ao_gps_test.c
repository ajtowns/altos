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

struct ao_gps_data {
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
	if (encoded_len != len - 8) {
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
	if (id == 41) {
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
		printf ("\tWeek %d\n", week);
		printf ("\tTOW %d\n", tow);
		printf ("\tyear %d\n", year);
		printf ("\tmonth %d\n", month);
		printf ("\tday %d\n", day);
		printf ("\thour %d\n", hour);
		printf ("\tminute %d\n", minute);
		printf ("\tsecond %g\n", second / 1000.0);
		printf ("\tsats: %08x\n", sat_list);
		printf ("\tlat: %g\n", lat / 1.0e7);
		printf ("\tlon: %g\n", lon / 1.0e7);
		printf ("\talt_ell: %g\n", alt_ell / 100.0);
		printf ("\talt_msll: %g\n", alt_msl / 100.0);
		printf ("\tdatum: %d\n", datum);
		printf ("\tground speed: %g\n", sog / 100.0);
		printf ("\tcourse: %g\n", cog / 100.0);
		printf ("\tclimb: %g\n", climb_rate / 100.0);
		printf ("\theading rate: %g\n", heading_rate / 100.0);
		printf ("\th error: %g\n", h_error / 100.0);
		printf ("\tv error: %g\n", v_error / 100.0);
		printf ("\tt error: %g\n", t_error / 100.0);
		printf ("\th vel error: %g\n", h_v_error / 100.0);
	} else {
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

#include "ao_gps_print.c"
#include "ao_gps.c"

void
ao_dump_state(void)
{
	double	lat, lon;
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

int
main (int argc, char **argv)
{
	char	*gps_file = "/dev/ttyUSB0";

	ao_gps_fd = ao_gps_open(gps_file);
	if (ao_gps_fd < 0) {
		perror (gps_file);
		exit (1);
	}
	ao_gps_setup();
	ao_gps();
}
