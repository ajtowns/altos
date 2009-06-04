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

#define AO_GPS_LEADER		6

static const char ao_gps_header[] = "GPGGA,";
__xdata uint8_t ao_gps_mutex;
static __xdata char ao_gps_char;
static __xdata uint8_t ao_gps_cksum;
static __xdata uint8_t ao_gps_error;

__xdata struct ao_gps_data	ao_gps_data;
static __xdata struct ao_gps_data	ao_gps_next;

const char ao_gps_config[] =
	"$PSRF103,00,00,01,01*25\r\n"	/* GGA 1 per sec */
	"$PSRF103,01,00,00,01*25\r\n"	/* GLL disable */
	"$PSRF103,02,00,00,01*26\r\n"	/* GSA disable */
	"$PSRF103,03,00,00,01*27\r\n"	/* GSV disable */
	"$PSRF103,04,00,00,01*20\r\n"	/* RMC disable */
	"$PSRF103,05,00,00,01*21\r\n";	/* VTG disable */

static void
ao_gps_lexchar(void)
{
	if (ao_gps_error)
		ao_gps_char = '\n';
	else
		ao_gps_char = ao_serial_getchar();
	ao_gps_cksum ^= ao_gps_char;
}

void
ao_gps_skip(void)
{
	while (ao_gps_char >= '0')
		ao_gps_lexchar();
}

void
ao_gps_skip_field(void)
{
	while (ao_gps_char != ',' && ao_gps_char != '*' && ao_gps_char != '\n')
		ao_gps_lexchar();
}

void
ao_gps_skip_sep(void)
{
	while (ao_gps_char == ',' || ao_gps_char == '.' || ao_gps_char == '*')
		ao_gps_lexchar();
}

__xdata static uint8_t ao_gps_num_width;

static int16_t
ao_gps_decimal(uint8_t max_width)
{
	int16_t	v;
	__xdata uint8_t	neg = 0;

	ao_gps_skip_sep();
	if (ao_gps_char == '-') {
		neg = 1;
		ao_gps_lexchar();
	}
	v = 0;
	ao_gps_num_width = 0;
	while (ao_gps_num_width < max_width) {
		if (ao_gps_char < '0' || '9' < ao_gps_char)
			break;
		v = v * (int16_t) 10 + ao_gps_char - '0';
		ao_gps_num_width++;
		ao_gps_lexchar();
	}
	if (neg)
		v = -v;
	return v;
}

static uint8_t
ao_gps_hex(uint8_t max_width)
{
	uint8_t	v, d;

	ao_gps_skip_sep();
	v = 0;
	ao_gps_num_width = 0;
	while (ao_gps_num_width < max_width) {
		if ('0' <= ao_gps_char && ao_gps_char <= '9')
			d = ao_gps_char - '0';
		else if ('A' <= ao_gps_char && ao_gps_char <= 'F')
			d = ao_gps_char - 'A' + 10;
		else if ('a' <= ao_gps_char && ao_gps_char <= 'f')
			d = ao_gps_char - 'a' + 10;
		else
			break;
		v = (v << 4) | d;
		ao_gps_num_width++;
		ao_gps_lexchar();
	}
	return v;
}

static void
ao_gps_parse_pos(__xdata struct ao_gps_pos * pos, uint8_t deg_width) __reentrant
{
	pos->degrees = ao_gps_decimal(deg_width);
	pos->minutes = ao_gps_decimal(2);
	if (ao_gps_char == '.') {
		pos->minutes_fraction = ao_gps_decimal(4);
		while (ao_gps_num_width < 4) {
			pos->minutes_fraction *= 10;
			ao_gps_num_width++;
		}
	} else {
		pos->minutes_fraction = 0;
		if (ao_gps_char != ',')
			ao_gps_error = 1;
	}
}

static void
ao_gps_parse_flag(char yes_c, uint8_t yes, char no_c, uint8_t no) __reentrant
{
	ao_gps_skip_sep();
	if (ao_gps_char == yes_c)
		ao_gps_next.flags |= yes;
	else if (ao_gps_char == no_c)
		ao_gps_next.flags |= no;
	else
		ao_gps_error = 1;
	ao_gps_lexchar();
}


void
ao_gps(void) __reentrant
{
	char	c;
	uint8_t	i;

	for (i = 0; (c = ao_gps_config[i]); i++)
		ao_serial_putchar(c);
	for (;;) {
		/* Locate the begining of the next record */
		for (;;) {
			c = ao_serial_getchar();
			if (c == '$')
				break;
		}

		ao_gps_cksum = 0;
		ao_gps_error = 0;

		/* Skip anything other than GGA */
		for (i = 0; i < AO_GPS_LEADER; i++) {
			ao_gps_lexchar();
			if (ao_gps_char != ao_gps_header[i])
				break;
		}
		if (i != AO_GPS_LEADER)
			continue;

		/* Now read the data into the gps data record
		 *
		 * $GPGGA,025149.000,4528.1723,N,12244.2480,W,1,05,2.0,103.5,M,-19.5,M,,0000*66
		 *
		 * Essential fix data
		 *
		 *	   025149.000	time (02:51:49.000 GMT)
		 *	   4528.1723,N	Latitude 45°28.1723' N
		 *	   12244.2480,W	Longitude 122°44.2480' W
		 *	   1		Fix quality:
		 *				   0 = invalid
		 *				   1 = GPS fix (SPS)
		 *				   2 = DGPS fix
		 *				   3 = PPS fix
		 *				   4 = Real Time Kinematic
		 *				   5 = Float RTK
		 *				   6 = estimated (dead reckoning)
		 *				   7 = Manual input mode
		 *				   8 = Simulation mode
		 *	   05		Number of satellites (5)
		 *	   2.0		Horizontal dilution
		 *	   103.5,M		Altitude, 103.5M above msl
		 *	   -19.5,M		Height of geoid above WGS84 ellipsoid
		 *	   ?		time in seconds since last DGPS update
		 *	   0000		DGPS station ID
		 *	   *66		checksum
		 */

		ao_gps_next.flags = 0;
		ao_gps_next.hour = ao_gps_decimal(2);
		ao_gps_next.minute = ao_gps_decimal(2);
		ao_gps_next.second = ao_gps_decimal(2);
		ao_gps_skip_field();	/* skip seconds fraction */

		ao_gps_parse_pos(&ao_gps_next.latitude, 2);
		ao_gps_parse_flag('N', AO_GPS_LATITUDE_NORTH, 'S', AO_GPS_LATITUDE_SOUTH);
		ao_gps_parse_pos(&ao_gps_next.longitude, 3);
		ao_gps_parse_flag('W', AO_GPS_LONGITUDE_WEST, 'E', AO_GPS_LONGITUDE_EAST);

		i = ao_gps_decimal(0xff);
		if (i == 1)
			ao_gps_next.flags |= AO_GPS_VALID;

		i = ao_gps_decimal(0xff) << AO_GPS_NUM_SAT_SHIFT;
		if (i > AO_GPS_NUM_SAT_MASK)
			i = AO_GPS_NUM_SAT_MASK;
		ao_gps_next.flags |= i;

		ao_gps_lexchar();
		ao_gps_skip_field();	/* Horizontal dilution */

		ao_gps_next.altitude = ao_gps_decimal(0xff);
		ao_gps_skip_field();	/* skip any fractional portion */

		/* Skip remaining fields */
		while (ao_gps_char != '*' && ao_gps_char != '\n' && ao_gps_char != '\r') {
			ao_gps_lexchar();
			ao_gps_skip_field();
		}
		if (ao_gps_char == '*') {
			uint8_t cksum = ao_gps_cksum ^ '*';
			if (cksum != ao_gps_hex(2))
				ao_gps_error = 1;
		} else
			ao_gps_error = 1;
		if (!ao_gps_error) {
			ao_mutex_get(&ao_gps_mutex);
			memcpy(&ao_gps_data, &ao_gps_next, sizeof (struct ao_gps_data));
			ao_mutex_put(&ao_gps_mutex);
			ao_wakeup(&ao_gps_data);
		}
	}
}

__xdata struct ao_task ao_gps_task;

static void
gps_dump(void) __reentrant
{
	ao_mutex_get(&ao_gps_mutex);
	ao_gps_print(&ao_gps_data);
	ao_mutex_put(&ao_gps_mutex);
}

__code struct ao_cmds ao_gps_cmds[] = {
	{ 'g', gps_dump,	"g                                  Display current GPS values" },
	{ 0, gps_dump, NULL },
};

void
ao_gps_init(void)
{
	ao_add_task(&ao_gps_task, ao_gps, "gps");
	ao_cmd_register(&ao_gps_cmds[0]);
}
