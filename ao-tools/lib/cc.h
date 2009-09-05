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

#ifndef _CC_H_
#define _CC_H_

char *
cc_fullname (char *dir, char *file);

char *
cc_basename(char *file);

int
cc_mkdir(char *dir);

struct cc_usbdev {
	char	*sys;
	char	*tty;
	char	*manufacturer;
	char	*product;
	int	serial;	/* AltOS always uses simple integer serial numbers */
	int	idProduct;
	int	idVendor;
};

struct cc_usbdevs {
	struct cc_usbdev	**dev;
	int			ndev;
};

void
cc_usbdevs_free(struct cc_usbdevs *usbdevs);

struct cc_usbdevs *
cc_usbdevs_scan(void);

char *
cc_usbdevs_find_by_arg(char *arg, char *default_product);

void
cc_set_log_dir(char *dir);

char *
cc_get_log_dir(void);

char *
cc_make_filename(int serial, char *ext);

#endif /* _CC_H_ */
