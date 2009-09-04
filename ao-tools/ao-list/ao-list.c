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
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "cc.h"

int
main (int argc, char **argv)
{
	struct cc_usbdevs	*devs;
	struct cc_usbdev	*dev;
	int			i;

	devs = cc_usbdevs_scan();
	if (devs) {
		for (i = 0; i < devs->ndev; i++) {
			dev = devs->dev[i];
			printf ("%-20.20s %6d %s\n",
				dev->product, dev->serial, dev->tty);
		}
		cc_usbdevs_free(devs);
	}
	return 0;
}
