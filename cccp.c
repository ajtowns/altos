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

static void
say(char *name, uint8_t bits)
{
	printf("%s: ", name);
	if (bits & CC_RESET_N)
		printf ("R ");
	else
		printf (". ");
	if (bits & CC_CLOCK)
		printf ("C ");
	else
		printf (". ");
	if (bits & CC_DATA)
		printf ("D\n");
	else
		printf (".\n");
}

static void
_cccp_write(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
	uint16_t	set;
	int		ret;

	set = (mask) | (value << 8);
	dbg->debug_data = (dbg->debug_data & ~mask) | (value & mask);
	ret = ioctl(dbg->fd, CP2101_IOCTL_GPIOSET, &set);
	if (ret < 0)
		perror("CP2101_IOCTL_GPIOSET");
}

void
cccp_write(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
	_cccp_write(dbg, mask, value);
//	say("w", dbg->debug_data);
}

uint8_t
cccp_read_all(struct ccdbg *dbg)
{
	int ret;
	uint8_t	get;
	ret = ioctl(dbg->fd, CP2101_IOCTL_GPIOGET, &get);
	if (ret < 0) {
		perror("CP2101_IOCTL_GPIOGET");
		get = 0;
	}
	return get;
}

uint8_t
cccp_read(struct ccdbg *dbg, uint8_t mask)
{
	uint8_t		pull_up;
	uint8_t		get;

	/* tri-state the bits of interest */
	pull_up = (~dbg->debug_data) & mask;
	if (pull_up)
		_cccp_write(dbg, pull_up, pull_up);
	get = cccp_read_all(dbg);
	say("\t\tr", get);
	return get & mask;
}

void
cccp_init(struct ccdbg *dbg)
{
	/* set all of the GPIOs to a known state */
	cccp_write(dbg, 0xf, 0xf);
}

void
cccp_fini(struct ccdbg *dbg)
{
	/* set all of the GPIOs to a known state */
	cccp_write(dbg, 0xf, 0xf);
	dbg->clock = 1;
}
