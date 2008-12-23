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

#include "s51.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int s51_port = 0;
static char *cpu = "8051";
static double freq = 11059200;
char *s51_prompt = "> ";
struct ccdbg *s51_dbg;

static void
usage(void)
{
	fprintf(stderr, "You're doing it wrong.\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int flags, opt;
	FILE *console_in = stdin;
	FILE *console_out = stdout;
	char *endptr;

	while ((opt = getopt(argc, argv, "PVvHht:X:c:r:Z:s:S:p:")) != -1) {
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
			FILE *client_in, *client_out;
			
			s = accept(l, (struct sockaddr *)
				   &client_addr, &client_len);
			if (s < 0) {
				perror("accept");
				exit(1);
			}
			client_in = fdopen(s, "r");
			client_out = fdopen(s, "w");
			if (!client_in || !client_out) {
				perror("fdopen");
				exit(1);
			}
			command_read(client_in, client_out);
			fclose(client_in);
			fclose(client_out);
		}
	} else
		command_read(console_in, console_out);
	exit(0);
}
