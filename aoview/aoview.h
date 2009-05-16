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

#ifndef _AOVIEW_H_
#define _AOVIEW_H_

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

struct usbdev {
	char	*sys;
	char	*tty;
	char	*manufacturer;
	char	*product;
	char	*serial;
	int	idProduct;
	int	idVendor;
};

struct aostate {
	char	callsign[16];
	int	serial;
	int	rssi;
	char	state[16];
	int	tick;
	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;
	int	nsat;
	int	locked;
	struct {
		int hour;
		int minute;
		int second;
	} gps_time;
	double	lat;
	double	lon;
	int	alt;
};

void
aoview_monitor_disconnect(void);

void
aoview_monitor_connect(char *tty);

struct aoview_serial *
aoview_serial_open(const char *tty);

void
aoview_serial_close(struct aoview_serial *serial);

void
aoview_serial_set_callback(struct aoview_serial *serial,
			   GSourceFunc func,
			   gpointer data,
			   GDestroyNotify notify);

void
aoview_serial_printf(struct aoview_serial *serial, char *format, ...);

int
aoview_serial_read(struct aoview_serial *serial, char *buf, int len);

int
aoview_serial_getc(struct aoview_serial *serial);

void
aoview_dev_dialog_init(GladeXML *xml);

int
aoview_usb_scan(struct usbdev ***devs_ret);

void
aoview_usbdev_free(struct usbdev *usbdev);

void
aoview_state_notify(struct aostate *state);

void
aoview_state_init(GladeXML *xml);

int16_t
aoview_pres_to_altitude(int16_t pres);

int16_t
aoview_altitude_to_pres(int16_t alt);

#endif /* _AOVIEW_H_ */
