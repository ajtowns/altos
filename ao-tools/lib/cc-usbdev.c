/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#define _GNU_SOURCE
#include "cc.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
load_string(char *dir, char *file)
{
	char	*full = cc_fullname(dir, file);
	char	line[4096];
	char	*r;
	FILE	*f;
	int	rlen;

	f = fopen(full, "r");
	free(full);
	if (!f)
		return NULL;
	r = fgets(line, sizeof (line), f);
	fclose(f);
	if (!r)
		return NULL;
	rlen = strlen(r);
	if (r[rlen-1] == '\n')
		r[rlen-1] = '\0';
	return strdup(r);
}

static int
load_hex(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 16);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
load_dec(char *dir, char *file)
{
	char	*line;
	char	*end;
	long	i;

	line = load_string(dir, file);
	if (!line)
		return -1;
	i = strtol(line, &end, 10);
	free(line);
	if (end == line)
		return -1;
	return i;
}

static int
dir_filter_tty_colon(const struct dirent *d)
{
	return strncmp(d->d_name, "tty:", 4) == 0;
}

static int
dir_filter_tty(const struct dirent *d)
{
	return strncmp(d->d_name, "tty", 3) == 0;
}

static char *
usb_tty(char *sys)
{
	char *base;
	int num_configs;
	int config;
	struct dirent **namelist;
	int interface;
	int num_interfaces;
	char endpoint_base[20];
	char *endpoint_full;
	char *tty_dir;
	int ntty;
	char *tty;

	base = cc_basename(sys);
	num_configs = load_hex(sys, "bNumConfigurations");
	num_interfaces = load_hex(sys, "bNumInterfaces");
	for (config = 1; config <= num_configs; config++) {
		for (interface = 0; interface < num_interfaces; interface++) {
			sprintf(endpoint_base, "%s:%d.%d",
				base, config, interface);
			endpoint_full = cc_fullname(sys, endpoint_base);

			/* Check for tty:ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty_colon,
				       alphasort);
			if (ntty > 0) {
				free(endpoint_full);
				tty = cc_fullname("/dev", namelist[0]->d_name + 4);
				free(namelist);
				return tty;
			}

			/* Check for tty/ttyACMx style names
			 */
			tty_dir = cc_fullname(endpoint_full, "tty");
			ntty = scandir(tty_dir, &namelist,
				       dir_filter_tty,
				       alphasort);
			free (tty_dir);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(endpoint_full);
				free(namelist);
				return tty;
			}

			/* Check for ttyACMx style names
			 */
			ntty = scandir(endpoint_full, &namelist,
				       dir_filter_tty,
				       alphasort);
			free(endpoint_full);
			if (ntty > 0) {
				tty = cc_fullname("/dev", namelist[0]->d_name);
				free(namelist);
				return tty;
			}
		}
	}
	return NULL;
}

static struct cc_usbdev *
usb_scan_device(char *sys)
{
	struct cc_usbdev *usbdev;

	usbdev = calloc(1, sizeof (struct cc_usbdev));
	if (!usbdev)
		return NULL;
	usbdev->sys = strdup(sys);
	usbdev->manufacturer = load_string(sys, "manufacturer");
	usbdev->product = load_string(sys, "product");
	usbdev->serial = load_dec(sys, "serial");
	usbdev->idProduct = load_hex(sys, "idProduct");
	usbdev->idVendor = load_hex(sys, "idVendor");
	usbdev->tty = usb_tty(sys);
	return usbdev;
}

static void
usbdev_free(struct cc_usbdev *usbdev)
{
	free(usbdev->sys);
	free(usbdev->manufacturer);
	free(usbdev->product);
	/* this can get used as a return value */
	if (usbdev->tty)
		free(usbdev->tty);
	free(usbdev);
}

#define USB_DEVICES	"/sys/bus/usb/devices"

static int
dir_filter_dev(const struct dirent *d)
{
	const char	*n = d->d_name;
	char	c;

	while ((c = *n++)) {
		if (isdigit(c))
			continue;
		if (c == '-')
			continue;
		if (c == '.' && n != d->d_name + 1)
			continue;
		return 0;
	}
	return 1;
}

static int
is_am(int idVendor, int idProduct) {
	if (idVendor == 0xfffe)
		return 1;
	if (idVendor == 0x0403 && idProduct == 0x6015)
		return 1;
	return 0;
}

struct cc_usbdevs *
cc_usbdevs_scan(void)
{
	int			e;
	struct dirent		**ents;
	char			*dir;
	struct cc_usbdev	*dev;
	struct cc_usbdevs	*devs;
	int			n;

	devs = calloc(1, sizeof (struct cc_usbdevs));
	if (!devs)
		return NULL;

	n = scandir (USB_DEVICES, &ents,
		     dir_filter_dev,
		     alphasort);
	if (!n)
		return 0;
	for (e = 0; e < n; e++) {
		dir = cc_fullname(USB_DEVICES, ents[e]->d_name);
		dev = usb_scan_device(dir);
		free(dir);
		if (is_am(dev->idVendor, dev->idProduct) && dev->tty) {
			if (devs->dev)
				devs->dev = realloc(devs->dev,
						    (devs->ndev + 1) * sizeof (struct usbdev *));
			else
				devs->dev = malloc (sizeof (struct usbdev *));
			devs->dev[devs->ndev++] = dev;
		}
	}
	free(ents);
	return devs;
}

void
cc_usbdevs_free(struct cc_usbdevs *usbdevs)
{
	int	i;

	if (!usbdevs)
		return;
	for (i = 0; i < usbdevs->ndev; i++)
		usbdev_free(usbdevs->dev[i]);
	free(usbdevs);
}

static char *
match_dev(char *product, int serial)
{
	struct cc_usbdevs	*devs;
	struct cc_usbdev	*dev;
	int			i;
	char			*tty = NULL;

	devs = cc_usbdevs_scan();
	if (!devs)
		return NULL;
	for (i = 0; i < devs->ndev; i++) {
		dev = devs->dev[i];
		if (product && strncmp (product, dev->product, strlen(product)) != 0)
			continue;
		if (serial && serial != dev->serial)
			continue;
		break;
	}
	if (i < devs->ndev) {
		tty = devs->dev[i]->tty;
		devs->dev[i]->tty = NULL;
	}
	cc_usbdevs_free(devs);
	return tty;
}

char *
cc_usbdevs_find_by_arg(char *arg, char *default_product)
{
	char	*product;
	int	serial;
	char	*end;
	char	*colon;
	char	*tty;

	if (arg)
	{
		/* check for <serial> */
		serial = strtol(arg, &end, 0);
		if (end != arg) {
			if (*end != '\0')
				return NULL;
			product = NULL;
		} else {
			/* check for <product>:<serial> */
			colon = strchr(arg, ':');
			if (colon) {
				product = strndup(arg, colon - arg);
				serial = strtol(colon + 1, &end, 0);
				if (*end != '\0')
					return NULL;
			} else {
				product = arg;
				serial = 0;
			}
		}
	} else {
		product = NULL;
		serial = 0;
	}
	tty = NULL;
	if (!product && default_product)
		tty = match_dev(default_product, serial);
	if (!tty)
		tty = match_dev(product, serial);
	if (product && product != arg)
		free(product);
	return tty;
}
