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

#include <stdio.h>
#include <flite/flite.h>

cst_voice *register_cmu_us_kal();

int
main(int argc, char **argv)
{
	char		line[1024];
	cst_voice	*v;

	flite_init();
	v = register_cmu_us_kal();
	if (!v) {
		perror("register voice");
		exit(1);
	}
	while (fgets(line, sizeof (line) - 1, stdin) != NULL) {
		flite_text_to_speech(line, v, "play");
	}
	exit (0);
}
