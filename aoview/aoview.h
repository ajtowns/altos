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

#define _GNU_SOURCE

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

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>

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
	int	flight_accel;
	int	ground_accel;
	int	flight_vel;
	int	flight_pres;
	int	ground_pres;
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

gboolean
aoview_monitor_connect(char *tty);

struct aoview_serial *
aoview_serial_open(const char *tty);

void
aoview_serial_close(struct aoview_serial *serial);

typedef void (*aoview_serial_callback)(gpointer user_data, struct aoview_serial *serial, gint revents);

void
aoview_serial_set_callback(struct aoview_serial *serial,
			   aoview_serial_callback func,
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
aoview_state_new(void);

void
aoview_state_init(GladeXML *xml);

int16_t
aoview_pres_to_altitude(int16_t pres);

int16_t
aoview_altitude_to_pres(int16_t alt);

char *
aoview_fullname (char *dir, char *file);

char *
aoview_basename(char *file);

GtkTreeViewColumn *
aoview_add_plain_text_column (GtkTreeView *view, const gchar *title, gint model_column, gint width);

int
aoview_mkdir(char *dir);

void
aoview_log_init(GladeXML *xml);

void
aoview_log_set_serial(int serial);

int
aoview_log_get_serial(void);

void
aoview_log_printf(char *format, ...);

void
aoview_log_new(void);

void
aoview_table_start(void);

void
aoview_table_add_row(char *label, char *format, ...);

void
aoview_table_finish(void);

void
aoview_table_init(GladeXML *xml);

void
aoview_table_clear(void);

struct aoview_file;

void
aoview_file_finish(struct aoview_file *file);

gboolean
aoview_file_start(struct aoview_file *file);

void
aoview_file_set_serial(struct aoview_file *file, int serial);

int
aoview_file_get_serial(struct aoview_file *file);

void
aoview_file_printf(struct aoview_file *file, char *format, ...);

void
aoview_file_vprintf(struct aoview_file *file, char *format, va_list ap);

struct aoview_file *
aoview_file_new(char *ext);

void
aoview_file_destroy(struct aoview_file *file);

void
aoview_file_init(GladeXML *xml);

#endif /* _AOVIEW_H_ */
