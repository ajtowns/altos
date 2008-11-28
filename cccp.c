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

void
cccp_write(struct ccdbg *dbg, uint8_t mask, uint8_t value)
{
	uint16_t	set;
	int		ret;

	set = (mask) | (value << 8);
	dbg->debug_data = (dbg->debug_data & ~mask) | (value & mask);
	printf (" -> %02x\n", dbg->debug_data);
	ret = ioctl(dbg->fd, CP2101_IOCTL_GPIOSET, &set);
	if (ret < 0)
		perror("CP2101_IOCTL_GPIOSET");
}

uint8_t
cccp_read(struct ccdbg *dbg, uint8_t mask)
{
	uint8_t		pull_up;
	int		ret;
	uint8_t		get;

	/* tri-state the bits of interest */
	pull_up = (~dbg->debug_data) & mask;
	if (pull_up) {
		cccp_write(dbg, pull_up, pull_up);
	}
	ret = ioctl(dbg->fd, CP2101_IOCTL_GPIOGET, &get);
	if (ret < 0) {
		perror("CP2101_IOCTL_GPIOGET");
		get = 0;
	}
	printf (" <- %02x\n", get);
	return get & mask;
}

void
cccp_init(struct ccdbg *dbg)
{
	/* set all of the GPIOs to a known state */
	cccp_write(dbg, 0xf, 0xf);
	dbg->clock = 1;
}

void
cccp_fini(struct ccdbg *dbg)
{
	/* set all of the GPIOs to a known state */
	cccp_write(dbg, 0xf, 0xf);
	dbg->clock = 1;
}
