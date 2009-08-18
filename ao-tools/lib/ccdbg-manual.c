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

#include "ccdbg.h"
#include "cc-bitbang.h"

/*
 * Manual bit-banging to debug the low level protocol
 */

static void
get_bit(char *line, int i, char on, uint8_t bit, uint8_t *bits, uint8_t *masks)
{
	if (line[i] == on) {
		*bits |= bit;
		*masks |= bit;
		return;
	}
	if (line[i] == '.') {
		*masks |= bit;
		return;
	}
	if (line[i] == '-') {
		return;
	}
	fprintf(stderr, "bad line %s\n", line);
	exit (1);
}

void
ccdbg_manual(struct ccdbg *dbg, FILE *input)
{
	char	line[80];
	uint8_t	set, mask;

	if (dbg->bb == NULL) {
		fprintf(stderr, "Must use bitbang API for manual mode\n");
		return;
	}
	while (fgets(line, sizeof line, input)) {
		if (line[0] == '#' || line[0] == '\n') {
			printf ("%s", line);
			continue;
		}
		set = 0;
		mask = 0;
		get_bit(line, 0, 'C', CC_CLOCK, &set, &mask);
		get_bit(line, 2, 'D', CC_DATA, &set, &mask);
		get_bit(line, 4, 'R', CC_RESET_N, &set, &mask);
		if (mask != (CC_CLOCK|CC_DATA|CC_RESET_N)) {
			uint8_t	read;
			cc_bitbang_read(dbg->bb, &read);
			cc_bitbang_sync(dbg->bb);
			cc_bitbang_print("\t%c %c %c", CC_CLOCK|CC_DATA|CC_RESET_N, read);
			if ((set & CC_CLOCK) == 0)
				printf ("\t%d", (read&CC_DATA) ? 1 : 0);
			printf ("\n");
		}
		cc_bitbang_send(dbg->bb, mask, set);
		cc_bitbang_sync(dbg->bb);
	}
}
