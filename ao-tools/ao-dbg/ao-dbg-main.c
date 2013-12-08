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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ao-dbg.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <poll.h>
#include <getopt.h>

static int s51_port = 0;
static char *cpu = "8051";
static double freq = 11059200;
char *s51_prompt = "> ";
struct ccdbg *s51_dbg;
int s51_interrupted = 0;
int s51_monitor = 0;
char *s51_tty = NULL;
char *s51_device = NULL;

static FILE *s51_input;
static FILE *s51_output;

static void
usage(void)
{
	fprintf(stderr, "You're doing it wrong.\n");
	exit(1);
}

void s51_sigint()
{
	s51_interrupted = 1;
}

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ 0, 0, 0, 0 },
};

int
main(int argc, char **argv)
{
	int flags, opt;
	char *endptr;
	struct sigvec vec, ovec;

	while ((opt = getopt_long(argc, argv, "PVvHhmt:X:c:r:Z:s:S:p:T:", options, NULL)) != -1) {
		switch (opt) {
		case 't':
			cpu = optarg;
			break;
		case 'X':
			freq = strtod(optarg, &endptr);
			if (endptr == optarg)
				usage();
			if (endptr[0] != '\0') {
				if (!strcmp(endptr, "k"))
					freq *= 1000;
				else if (!strcmp(endptr, "M") )
					freq *= 1000000;
				else
					usage ();
			}
			break;
		case 'c':
			break;
		case 'r':
		case 'Z':
			s51_port = strtol(optarg, &endptr, 0);
			if (endptr == optarg || strlen(endptr) != 0)
				usage();
			break;
		case 's':
			break;
		case 'S':
			break;
		case 'p':
			s51_prompt = optarg;
			break;
		case 'P':
			s51_prompt = NULL;
			break;
		case 'V':
			break;
		case 'v':
			break;
		case 'H':
			exit (0);
			break;
		case 'h':
			usage ();
			break;
		case 'm':
			s51_monitor = 1;
			break;
		case 'T':
			s51_tty = optarg;
			break;
		case 'D':
			s51_device = optarg;
			break;
		}
	}
	if (s51_port) {
		int l, r, one = 1;
		int s;
		struct sockaddr_in in;

		l = socket(AF_INET, SOCK_STREAM, 0);
		if (l < 0) {
			perror ("socket");
			exit(1);
		}
		r = setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &one, sizeof (int));
		if (r) {
			perror("setsockopt");
			exit(1);
		}
		in.sin_family = AF_INET;
		in.sin_port = htons(s51_port);
		in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		r = bind(l, (struct sockaddr *) &in, sizeof (in));
		if (r) {
			perror("bind");
			exit(1);
		}
		r = listen(l, 5);
		if (r) {
			perror("listen");
			exit(1);
		}
		for (;;) {
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof (struct sockaddr_in);

			s = accept(l, (struct sockaddr *)
				   &client_addr, &client_len);
			if (s < 0) {
				perror("accept");
				exit(1);
			}
			s51_input = fdopen(s, "r");
			s51_output = fdopen(s, "w");
			if (!s51_input || !s51_output) {
				perror("fdopen");
				exit(1);
			}
			vec.sv_handler = SIG_IGN;
			vec.sv_mask = 0;
			vec.sv_flags = 0;
			sigvec(SIGINT, &vec, &ovec);
			command_read();
			sigvec(SIGINT, &ovec, NULL);
			fclose(s51_input);
			fclose(s51_output);
		}
	} else {
		s51_input = stdin;
		s51_output = stdout;
		vec.sv_handler = s51_sigint;
		vec.sv_mask = 0;
		vec.sv_flags = 0;
		sigvec(SIGINT, &vec, &ovec);
		command_read();
	}
	exit(0);
}

void
s51_printf(char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	vfprintf(s51_output, format, ap);
	if (s51_monitor)
		vfprintf(stdout, format, ap);
	va_end(ap);
}

void
s51_putc(int c)
{
	putc(c, s51_output);
}

#if HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

int
s51_read_line(char *line, int len)
{
	int ret;
#if HAVE_LIBREADLINE
	if (s51_output == stdout && s51_input == stdin && s51_prompt) {
		char *r;

		r = readline(s51_prompt);
		if (r == NULL)
			return 0;
		strncpy (line, r, len);
		line[len-1] = '\0';
		add_history(r);
		return 1;
	} else
#endif
	{
		if (s51_prompt)
			s51_printf("%s", s51_prompt);
		else
			s51_putc('\0');
		fflush(s51_output);
		ret = fgets(line, len, s51_input) != NULL;
		if (s51_monitor)
			printf("> %s", line);
		fflush(stdout);
	}
	return ret;
}

int
s51_check_input(void)
{
	struct pollfd	input;
	int r;
	int c;

	input.fd = fileno(s51_input);
	input.events = POLLIN;
	r = poll(&input, 1, 0);
	if (r > 0) {
		char line[256];
		(void) s51_read_line(line, sizeof (line));
		return 1;
	}
	return 0;
}
