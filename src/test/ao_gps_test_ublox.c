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
#include <unistd.h>
#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_NEW_DATA		1
#define AO_GPS_NEW_TRACKING	2

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

struct ao_telemetry_location {
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
	uint8_t			pdop;		/* * 5 */
	uint8_t			hdop;		/* * 5 */
	uint8_t			vdop;		/* * 5 */
	int16_t			climb_rate;	/* cm/s */
	uint16_t		h_error;	/* m */
	uint16_t		v_error;	/* m */
};

#define UBLOX_SAT_STATE_ACQUIRED		(1 << 0)
#define UBLOX_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define UBLOX_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define UBLOX_SAT_SUBFRAME_SYNC_COMPLETE	(1 << 3)
#define UBLOX_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define UBLOX_SAT_CODE_LOCKED			(1 << 5)
#define UBLOX_SAT_ACQUISITION_FAILED		(1 << 6)
#define UBLOX_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct ao_telemetry_satellite_info {
	uint8_t		svid;
	uint8_t		c_n_1;
};

#define AO_TELEMETRY_SATELLITE_MAX_SAT	12

struct ao_telemetry_satellite {
	uint8_t					channels;
	struct ao_telemetry_satellite_info	sats[AO_TELEMETRY_SATELLITE_MAX_SAT];
};

#define ao_gps_orig ao_telemetry_location
#define ao_gps_tracking_orig ao_telemetry_satellite
#define ao_gps_sat_orig ao_telemetry_satellite_info

extern __xdata struct ao_telemetry_location	ao_gps_data;
extern __xdata struct ao_telemetry_satellite	ao_gps_tracking_data;

uint8_t ao_gps_mutex;

void
ao_mutex_get(uint8_t *mutex)
{
}

void
ao_mutex_put(uint8_t *mutex)
{
}

static int ao_gps_fd;
static FILE *ao_gps_file;

#if 0
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
#endif

#include <sys/time.h>

int
get_millis(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static uint8_t	in_message[4096];
static int	in_len;
static uint16_t	recv_len;

static void check_ublox_message(char *which, uint8_t *msg);

char
ao_gps_getchar(void)
{
	char	c;
	uint8_t	uc;
	int	i;

	i = getc(ao_gps_file);
	if (i == EOF) {
		perror("getchar");
		exit(1);
	}
	c = i;
	uc = (uint8_t) c;
	if (in_len || uc == 0xb5) {
		in_message[in_len++] = c;
		if (in_len == 6) {
			recv_len = in_message[4] | (in_message[5] << 8);
		} else if (in_len > 6 && in_len == recv_len + 8) {
			check_ublox_message("recv", in_message + 2);
			in_len = 0;
		}
		
	}
	return c;
}

#define MESSAGE_LEN	4096

static uint8_t	message[MESSAGE_LEN];
static int	message_len;
static uint16_t	send_len;

void
ao_gps_putchar(char c)
{
	int	i;
	uint8_t	uc = (uint8_t) c;

	if (message_len || uc == 0xb5) {
		if (message_len < MESSAGE_LEN)
			message[message_len++] = uc;
		if (message_len == 6) {
			send_len = message[4] | (message[5] << 8);
		} else if (message_len > 6 && message_len == send_len + 8) {
			check_ublox_message("send", message + 2);
			message_len = 0;
		}
	}

	for (;;) {
		i = write(ao_gps_fd, &c, 1);
		if (i == 1)
			break;
		if (i < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		perror("putchar");
		exit(1);
	}
}

#define AO_SERIAL_SPEED_4800	0
#define AO_SERIAL_SPEED_9600	1
#define AO_SERIAL_SPEED_57600	2
#define AO_SERIAL_SPEED_115200	3

static void
ao_gps_set_speed(uint8_t speed)
{
	int	fd = ao_gps_fd;
	struct termios	termios;

	printf ("\t\tset speed %d\n", speed);
	tcdrain(fd);
	tcgetattr(fd, &termios);
	switch (speed) {
	case AO_SERIAL_SPEED_4800:
		cfsetspeed(&termios, B4800);
		break;
	case AO_SERIAL_SPEED_9600:
		cfsetspeed(&termios, B9600);
		break;
	case AO_SERIAL_SPEED_57600:
		cfsetspeed(&termios, B57600);
		break;
	case AO_SERIAL_SPEED_115200:
		cfsetspeed(&termios, B115200);
		break;
	}
	tcsetattr(fd, TCSAFLUSH, &termios);
	tcflush(fd, TCIFLUSH);
}

#define ao_time() 0

uint8_t	ao_task_minimize_latency;

#define ao_usb_getchar()	0

#include "ao_gps_print.c"
#include "ao_gps_show.c"
#include "ao_gps_ublox.c"

static void
check_ublox_message(char *which, uint8_t *msg)
{
	uint8_t	class = msg[0];
	uint8_t	id = msg[1];
	uint16_t len = msg[2] | (msg[3] << 8);
	uint16_t i;
	struct ao_ublox_cksum	cksum_msg = { .a = msg[4 + len],
					      .b = msg[4 + len + 1] };
	struct ao_ublox_cksum	cksum= { 0, 0 };

	for (i = 0; i < 4 + len; i++) {
		add_cksum(&cksum, msg[i]);
	}
	if (cksum.a != cksum_msg.a || cksum.b != cksum_msg.b) {
		printf ("\t%s: cksum mismatch %02x,%02x != %02x,%02x\n",
			which,
			cksum_msg.a & 0xff,
			cksum_msg.b & 0xff,
			cksum.a & 0xff,
			cksum.b & 0xff);
		return;
	}
	switch (class) {
	case UBLOX_NAV:
		switch (id) {
		case UBLOX_NAV_DOP: ;
			struct ublox_nav_dop	*nav_dop = (void *) msg;
			printf ("\tnav-dop    iTOW %9u gDOP %5u dDOP %5u tDOP %5u vDOP %5u hDOP %5u nDOP %5u eDOP %5u\n",
				nav_dop->itow,
				nav_dop->gdop,
				nav_dop->ddop,
				nav_dop->tdop,
				nav_dop->vdop,
				nav_dop->hdop,
				nav_dop->ndop,
				nav_dop->edop);
			return;
		case UBLOX_NAV_POSLLH: ;
			struct ublox_nav_posllh	*nav_posllh = (void *) msg;
			printf ("\tnav-posllh iTOW %9u lon %12.7f lat %12.7f height %10.3f hMSL %10.3f hAcc %10.3f vAcc %10.3f\n",
				nav_posllh->itow,
				nav_posllh->lon / 1e7,
				nav_posllh->lat / 1e7,
				nav_posllh->height / 1e3,
				nav_posllh->hmsl / 1e3,
				nav_posllh->hacc / 1e3,
				nav_posllh->vacc / 1e3);
			return;
		case UBLOX_NAV_SOL: ;
			struct ublox_nav_sol	*nav_sol = (struct ublox_nav_sol *) msg;
			printf ("\tnav-sol    iTOW %9u fTOW %9d week %5d gpsFix %2d flags %02x\n",
				nav_sol->itow, nav_sol->ftow, nav_sol->week,
				nav_sol->gpsfix, nav_sol->flags);
			return;
		case UBLOX_NAV_SVINFO: ;
			struct ublox_nav_svinfo	*nav_svinfo = (struct ublox_nav_svinfo *) msg;
			printf ("\tnav-svinfo iTOW %9u numCH %3d globalFlags %02x\n",
				nav_svinfo->itow, nav_svinfo->numch, nav_svinfo->globalflags);
			int i;
			for (i = 0; i < nav_svinfo->numch; i++) {
				struct ublox_nav_svinfo_block *nav_svinfo_block = (void *) (msg + 12 + 12 * i);
				printf ("\t\tchn %3u svid %3u flags %02x quality %3u cno %3u elev %3d azim %6d prRes %9d\n",
					nav_svinfo_block->chn,
					nav_svinfo_block->svid,
					nav_svinfo_block->flags,
					nav_svinfo_block->quality,
					nav_svinfo_block->cno,
					nav_svinfo_block->elev,
					nav_svinfo_block->azim,
					nav_svinfo_block->prres);
			}
			return;
		case UBLOX_NAV_VELNED: ;
			struct ublox_nav_velned *nav_velned = (void *) msg;
			printf ("\tnav-velned iTOW %9u velN %10.2f velE %10.2f velD %10.2f speed %10.2f gSpeed %10.2f heading %10.5f sAcc %10.2f cAcc %10.5f\n",
				nav_velned->itow,
				nav_velned->veln / 1e2,
				nav_velned->vele / 1e2,
				nav_velned->veld / 1e2,
				nav_velned->speed / 1e2,
				nav_velned->gspeed / 1e2,
				nav_velned->heading / 1e5,
				nav_velned->sacc / 1e5,
				nav_velned->cacc / 1e6);
			return;
		case UBLOX_NAV_TIMEUTC:;
			struct ublox_nav_timeutc *nav_timeutc = (void *) msg;
			printf ("\tnav-timeutc iTOW %9u tAcc %5u nano %5d %4u-%2d-%2d %2d:%02d:%02d flags %02x\n",
				nav_timeutc->itow,
				nav_timeutc->tacc,
				nav_timeutc->nano,
				nav_timeutc->year,
				nav_timeutc->month,
				nav_timeutc->day,
				nav_timeutc->hour,
				nav_timeutc->min,
				nav_timeutc->sec,
				nav_timeutc->valid);
			return;
		}
		break;
	}
#if 1
	printf ("\t%s: class %02x id %02x len %d:", which, class & 0xff, id & 0xff, len & 0xffff);
	for (i = 0; i < len; i++)
		printf (" %02x", msg[4 + i]);
	printf (" cksum %02x %02x", cksum_msg.a & 0xff, cksum_msg.b & 0xff);
#endif
	printf ("\n");
}

void
ao_dump_state(void *wchan)
{
	if (wchan == &ao_gps_new)
		ao_gps_show();
	return;
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
	ao_gps_file = fdopen(ao_gps_fd, "r");
	ao_gps();
	return 0;
}
