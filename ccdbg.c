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

static uint8_t
get_bit(char *line, int i, char on, uint8_t bit)
{
	if (line[i] == on)
		return bit;
	if (line[i] == '.')
		return 0;
	fprintf(stderr, "bad line %s\n", line);
	exit (1);
}

static char
is_bit(uint8_t get, char on, uint8_t bit)
{
	if (get&bit)
		return on;
	else
		return '.';
}

static uint8_t
ccdbg_write_read(struct ccdbg *dbg, uint8_t set)
{
	uint8_t	get;

	cccp_write(dbg, CC_DATA|CC_CLOCK|CC_RESET_N, set);
	get = cccp_read_all(dbg);
	printf("%c %c %c -> %c %c %c\n",
	       is_bit(set, 'C', CC_CLOCK),
	       is_bit(set, 'D', CC_DATA),
	       is_bit(set, 'R', CC_RESET_N),
	       is_bit(get, 'C', CC_CLOCK),
	       is_bit(get, 'D', CC_DATA),
	       is_bit(get, 'R', CC_RESET_N));
	ccdbg_half_clock(dbg);
	return get;
}

static void
_ccdbg_debug_mode(struct ccdbg *dbg)
{
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg,          CC_DATA           );
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           );
	ccdbg_write_read(dbg,          CC_DATA           );
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           );
	ccdbg_write_read(dbg,          CC_DATA|CC_RESET_N);
}

static void
_ccdbg_reset(struct ccdbg *dbg)
{
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N);
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           );
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA           );
	ccdbg_write_read(dbg, CC_CLOCK|CC_DATA|CC_RESET_N);
}

static void
ccdbg_manual(struct ccdbg *dbg, FILE *input)
{
	char	line[80];
	uint8_t	set;

	while (fgets(line, sizeof line, input)) {
		if (line[0] == '#' || line[0] == '\n') {
			printf ("%s", line);
			continue;
		}
		set = 0;
		set |= get_bit(line, 0, 'C', CC_CLOCK);
		set |= get_bit(line, 2, 'D', CC_DATA);
		set |= get_bit(line, 4, 'R', CC_RESET_N);
		ccdbg_write_read(dbg, set);
	}
}

int
main (int argc, char **argv)
{
	struct ccdbg	*dbg;
	uint8_t		status;
	uint16_t	chip_id;

	dbg = ccdbg_open("/dev/ttyUSB0");
	if (!dbg)
		exit (1);
	ccdbg_manual(dbg, stdin);
#if 0	
	ccdbg_debug_mode(dbg);
	status = ccdbg_read_status(dbg);
	printf("Status: 0x%02x\n", status);
	chip_id = ccdbg_get_chip_id(dbg);
	printf("Chip id: 0x%04x\n", chip_id);
#endif
	_ccdbg_reset(dbg);
	ccdbg_close(dbg);
	exit (0);
}
