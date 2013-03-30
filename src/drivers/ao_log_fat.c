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
static uint8_t	log_running;
static uint8_t	log_mutex;

static void
ao_log_open(void)
{
	char	name[12];

	sprintf(name,"%04d%02d%02dLOG", 2000 + log_year, log_month, log_day);
	if (ao_fat_open(name, AO_FAT_OPEN_WRITE) == AO_FAT_SUCCESS)
		log_running = 1;
}

static void
ao_log_close(void)
{
	log_running = 0;
	ao_fat_close();
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
		if (log_running &&
		    (log_year != log->u.gps.year ||
		     log_month != log->u.gps.month ||
		     log_day != log->u.gps.day)) {
			ao_log_close();
		}
		if (!log_running) {
			log_year = log->u.gps.year;
			log_month = log->u.gps.month;
			log_day = log->u.gps.day;
			ao_log_open();
		}
	}
	if (log_running) {
		wrote = ao_fat_write(log, sizeof (*log)) == AO_FAT_SUCCESS;
		ao_fat_sync();
	}
	ao_mutex_put(&log_mutex);
	return wrote;
}
