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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include <math.h>

#include "cc.h"

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>

struct aostate {
	struct cc_telem	data;

	/* derived data */

	struct cc_telem	prev_data;

	double		report_time;

	gboolean	ascent;	/* going up? */

	int	ground_altitude;
	int	height;
	double	speed;
	double	acceleration;
	double	battery;
	double	temperature;
	double	main_sense;
	double	drogue_sense;
	double	baro_speed;

	int	max_height;
	double	max_acceleration;
	double	max_speed;

	struct cc_gps	gps;
	struct cc_gps_tracking	gps_tracking;

	int	gps_valid;
	double	pad_lat;
	double	pad_lon;
	double	pad_alt;
	double	pad_lat_total;
	double	pad_lon_total;
	double	pad_alt_total;
	int	npad;
	int	prev_npad;

	double	distance;
	double	bearing;
	int	gps_height;

	int	speak_tick;
	int	speak_altitude;
};

extern struct aostate aostate;

/* GPS is 'stable' when we've seen at least this many samples */
#define MIN_PAD_SAMPLES	10

void
aoview_monitor_disconnect(void);

gboolean
aoview_monitor_connect(char *tty);

gboolean
aoview_monitor_parse(const char *line);

void
aoview_monitor_set_channel(int channel);

void
aoview_monitor_reset(void);

struct aoview_serial *
aoview_serial_open(const char *tty);

void
aoview_serial_close(struct aoview_serial *serial);

typedef void (*aoview_serial_callback)(gpointer user_data, struct aoview_serial *serial, gint revents);

void
aoview_serial_set_callback(struct aoview_serial *serial,
			   aoview_serial_callback func);

void
aoview_serial_printf(struct aoview_serial *serial, char *format, ...);

int
aoview_serial_read(struct aoview_serial *serial, char *buf, int len);

int
aoview_serial_getc(struct aoview_serial *serial);

void
aoview_dev_dialog_init(GladeXML *xml);

void
aoview_state_notify(struct cc_telem *data);

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
aoview_table_add_row(int column, char *label, char *format, ...);

void
aoview_table_finish(void);

void
aoview_table_init(GladeXML *xml);

void
aoview_table_clear(void);

struct aoview_file;

extern char *aoview_file_dir;

void
aoview_file_finish(struct aoview_file *file);

gboolean
aoview_file_start(struct aoview_file *file);

const char *
aoview_file_name(struct aoview_file *file);

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

/* aoview_eeprom.c */

gboolean
aoview_eeprom_save(const char *device);

void
aoview_eeprom_init(GladeXML *xml);

/* aoview_voice.c */
void aoview_voice_open(void);

void aoview_voice_close(void);

void aoview_voice_speak(char *format, ...);

/* aoview_label.c */

void aoview_label_init(GladeXML *xml);

void
aoview_label_show(struct aostate *state);

/* aoview_flite.c */

FILE *
aoview_flite_start(void);

void
aoview_flite_stop(void);

/* aoview_main.c */

extern char *aoview_tty;

/* aoview_channel.c */

int
aoview_channel_current(void);

void
aoview_channel_init(GladeXML *xml);

#endif /* _AOVIEW_H_ */
