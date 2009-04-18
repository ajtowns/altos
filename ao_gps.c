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

#include "ao.h"

__xdata struct ao_task ao_gps_task;

#define AO_GPS_MAX_LINE		80

#define AO_GPS_LEADER		6

__xdata uint8_t ao_gps_line[AO_GPS_MAX_LINE+1] = "GPGGA,";
__xdata uint8_t ao_gps_mutex;
__xdata uint8_t ao_gps_data;

const uint8_t ao_gps_config[] =
	"$PSRF103,00,00,01,01*25\r\n"	/* GGA 1 per sec */
	"$PSRF103,01,00,00,01*25\r\n"	/* GLL disable */
	"$PSRF103,02,00,00,01*26\r\n"	/* GSA disable */
	"$PSRF103,03,00,00,01*27\r\n"	/* GSV disable */
	"$PSRF103,04,00,00,01*20\r\n"	/* RMC disable */
	"$PSRF103,05,00,00,01*21\r\n";	/* VTG disable */


void
ao_gps(void)
{
	uint8_t	c;
	uint8_t	i;

	for (i = 0; (c = ao_gps_config[i]); i++)
		ao_serial_putchar(c);
	for (;;) {
		for (;;) {
			c = ao_serial_getchar();
			if (c == '$')
				break;
		}
		for (i = 0; i < AO_GPS_LEADER; i++)
			if (ao_serial_getchar() != ao_gps_line[i])
				break;
		if (i != AO_GPS_LEADER)
			continue;
		ao_mutex_get(&ao_gps_mutex);
		for (;;) {
			c = ao_serial_getchar();
			if (c < ' ')
				break;
			if (i < AO_GPS_MAX_LINE)
				ao_gps_line[i++] = c;
		}
		ao_gps_line[i] = '\0';
		ao_gps_data = 1;
		ao_mutex_put(&ao_gps_mutex);
		ao_wakeup(&ao_gps_data);
		puts(ao_gps_line);
		ao_usb_flush();
	}
}

void
ao_gps_init(void)
{
	ao_add_task(&ao_gps_task, ao_gps);
}
