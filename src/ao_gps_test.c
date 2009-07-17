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
#include <pthread.h>
#include <semaphore.h>
#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)

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

static sem_t input_semaphore;

char
ao_serial_getchar(void)
{
	char	c;
	int	value;
	char	line[100];

	sem_getvalue(&input_semaphore, &value);
//	printf ("ao_serial_getchar %d\n", value);
	sem_wait(&input_semaphore);
	c = input_queue[input_head];
	input_head = (input_head + 1) % QUEUE_LEN;
//	sprintf (line, "%02x\n", ((int) c) & 0xff);
//	write(1, line, strlen(line));
	return c;
}

void *
ao_gps_input(void *arg)
{
	int	i;
	char	c;

	printf("ao_gps_input\n");
	for (;;) {
		i = read(ao_gps_fd, &c, 1);
		if (i == 1) {
			int	v;

			input_queue[input_tail] = c;
			input_tail = (input_tail + 1) % QUEUE_LEN;
			sem_post(&input_semaphore);
			sem_getvalue(&input_semaphore, &v);
//			printf ("ao_gps_input %02x %d\n", ((int) c) & 0xff, v);
			fflush(stdout);
			continue;
		}
		if (i < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		perror("getchar");
		exit(1);
	}
}

void
ao_serial_putchar(char c)
{
	int	i;

	ao_dbg_char(c);
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

static void
ao_serial_set_speed(uint8_t fast)
{
	int	fd = ao_gps_fd;
	struct termios	termios;

	tcdrain(fd);
	tcgetattr(fd, &termios);
	cfsetspeed(&termios, fast ? B9600 : B4800);
	tcsetattr(fd, TCSAFLUSH, &termios);
	tcflush(fd, TCIFLUSH);
}

#include "ao_gps_print.c"
#include "ao_gps.c"

void
ao_dump_state(void)
{
	printf ("dump state\n");
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

pthread_t input_thread;

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
	sem_init(&input_semaphore, 0, 0);
	if (pthread_create(&input_thread, NULL, ao_gps_input, NULL) != 0)
		perror("pthread_create");
	ao_gps();
}
