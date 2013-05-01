/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#include "ao.h"
#include "ao_log.h"
#include "ao_fat.h"

static uint8_t	log_year, log_month, log_day;
static uint8_t	log_open;
static int8_t	log_fd;
static uint8_t	log_mutex;

static void
ao_log_open(void)
{
	static char	name[12];
	int8_t	status;

	sprintf(name,"%04d%02d%02dLOG", 2000 + log_year, log_month, log_day);
	status = ao_fat_open(name, AO_FAT_OPEN_WRITE);
	if (status >= 0) {
		log_fd = status;
		ao_fat_seek(log_fd, 0, AO_FAT_SEEK_END);
		log_open = 1;
	} else if (status == -AO_FAT_ENOENT) {
		status = ao_fat_creat(name);
		if (status >= 0) {
			log_fd = status;
			log_open = 1;
		}
	} 
}

static void
ao_log_close(void)
{
	if (log_open) {
		log_open = 0;
		ao_fat_close(log_fd);
		log_fd = -1;
	}
}

uint8_t
ao_log_full(void)
{
	return ao_fat_full();
}

uint8_t
ao_log_mega(struct ao_log_mega *log)
{
	uint8_t	wrote = 0;
	ao_mutex_get(&log_mutex);
	if (log->type == AO_LOG_GPS_TIME) {
		if (log_open &&
		    (log_year != log->u.gps.year ||
		     log_month != log->u.gps.month ||
		     log_day != log->u.gps.day)) {
			ao_log_close();
		}
		if (!log_open) {
			log_year = log->u.gps.year;
			log_month = log->u.gps.month;
			log_day = log->u.gps.day;
			ao_log_open();
		}
	}
	if (log_open) {
		wrote = ao_fat_write(log_fd, log, sizeof (*log)) == AO_FAT_SUCCESS;
		ao_fat_sync();
	}
	ao_mutex_put(&log_mutex);
	return wrote;
}

void
ao_log_flush(void)
{
	ao_fat_sync();
}
