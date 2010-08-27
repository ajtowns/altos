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

#ifndef AO_GPS_TEST
#include "ao.h"
#endif

#define AO_GPS_LEADER		2

static const char ao_gps_header[] = "GP";

__xdata uint8_t ao_gps_mutex;
static __xdata char ao_gps_char;
static __xdata uint8_t ao_gps_cksum;
static __xdata uint8_t ao_gps_error;

__xdata uint16_t ao_gps_tick;
__xdata struct ao_gps_data	ao_gps_data;
__xdata struct ao_gps_tracking_data	ao_gps_tracking_data;

static __xdata uint16_t				ao_gps_next_tick;
static __xdata struct ao_gps_data		ao_gps_next;
static __xdata uint8_t				ao_gps_date_flags;
static __xdata struct ao_gps_tracking_data	ao_gps_tracking_next;

static const char ao_gps_config[] = {
	0xa0, 0xa1, 0x00, 0x09,		/* length 9 bytes */
	0x08,				/* configure nmea */
	1,				/* gga interval */
	1,				/* gsa interval */
	1,				/* gsv interval */
	1,				/* gll interval */
	1,				/* rmc interval */
	1,				/* vtg interval */
	1,				/* zda interval */
	0,				/* attributes (0 = update to sram, 1 = update flash too) */
	0x09, 0x0d, 0x0a,

	0xa0, 0xa1, 0x00, 0x03,		/* length: 3 bytes */
	0x3c,				/* configure navigation mode */
	0x00,				/* 0 = car, 1 = pedestrian */
	0x00,				/* 0 = update to sram, 1 = update sram + flash */
	0x3c, 0x0d, 0x0a,
};

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
	if (ao_gps_char == ',' || ao_gps_char == '.' || ao_gps_char == '*')
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

static int32_t
ao_gps_parse_pos(uint8_t deg_width) __reentrant
{
	int32_t	d;
	int32_t	m;
	int32_t	f;

	d = ao_gps_decimal(deg_width);
	m = ao_gps_decimal(2);
	if (ao_gps_char == '.') {
		f = ao_gps_decimal(4);
		while (ao_gps_num_width < 4) {
			f *= 10;
			ao_gps_num_width++;
		}
	} else {
		f = 0;
		if (ao_gps_char != ',')
			ao_gps_error = 1;
	}
	d = d * 10000000l;
	m = m * 10000l + f;
	d = d + m * 50 / 3;
	return d;
}

static uint8_t
ao_gps_parse_flag(char no_c, char yes_c) __reentrant
{
	uint8_t	ret = 0;
	ao_gps_skip_sep();
	if (ao_gps_char == yes_c)
		ret = 1;
	else if (ao_gps_char == no_c)
		ret = 0;
	else
		ao_gps_error = 1;
	ao_gps_lexchar();
	return ret;
}


void
ao_gps(void) __reentrant
{
	char	a, c;
	uint8_t	i;

	ao_serial_set_speed(AO_SERIAL_SPEED_9600);
	for (i = 0; i < sizeof (ao_gps_config); i++)
		ao_serial_putchar(ao_gps_config[i]);
	for (;;) {
		/* Locate the begining of the next record */
		for (;;) {
			c = ao_serial_getchar();
			if (c == '$')
				break;
		}

		ao_gps_cksum = 0;
		ao_gps_error = 0;

		/* Skip anything other than GP */
		for (i = 0; i < AO_GPS_LEADER; i++) {
			ao_gps_lexchar();
			if (ao_gps_char != ao_gps_header[i])
				break;
		}
		if (i != AO_GPS_LEADER)
			continue;

		/* pull the record identifier characters off the link */
		ao_gps_lexchar();
		a = ao_gps_char;
		ao_gps_lexchar();
		c = ao_gps_char;
		ao_gps_lexchar();
		i = ao_gps_char;
		ao_gps_lexchar();
		if (ao_gps_char != ',')
			continue;

		if (a == (uint8_t) 'G' && c == (uint8_t) 'G' && i == (uint8_t) 'A') {
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

			ao_gps_next_tick = ao_time();
			ao_gps_next.flags = AO_GPS_RUNNING | ao_gps_date_flags;
			ao_gps_next.hour = ao_gps_decimal(2);
			ao_gps_next.minute = ao_gps_decimal(2);
			ao_gps_next.second = ao_gps_decimal(2);
			ao_gps_skip_field();	/* skip seconds fraction */

			ao_gps_next.latitude = ao_gps_parse_pos(2);
			if (ao_gps_parse_flag('N', 'S'))
				ao_gps_next.latitude = -ao_gps_next.latitude;
			ao_gps_next.longitude = ao_gps_parse_pos(3);
			if (ao_gps_parse_flag('E', 'W'))
				ao_gps_next.longitude = -ao_gps_next.longitude;

			i = ao_gps_decimal(0xff);
			if (i == 1)
				ao_gps_next.flags |= AO_GPS_VALID;

			i = ao_gps_decimal(0xff) << AO_GPS_NUM_SAT_SHIFT;
			if (i > AO_GPS_NUM_SAT_MASK)
				i = AO_GPS_NUM_SAT_MASK;
			ao_gps_next.flags |= i;

			ao_gps_lexchar();
			ao_gps_next.hdop = ao_gps_decimal(0xff);
			if (ao_gps_next.hdop <= 50) {
				ao_gps_next.hdop = (uint8_t) 5 * ao_gps_next.hdop;
				if (ao_gps_char == '.')
					ao_gps_next.hdop = (ao_gps_next.hdop +
							    ((uint8_t) ao_gps_decimal(1) >> 1));
			} else
				ao_gps_next.hdop = 255;
			ao_gps_skip_field();

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
				ao_gps_tick = ao_gps_next_tick;
				memcpy(&ao_gps_data, &ao_gps_next, sizeof (struct ao_gps_data));
				ao_mutex_put(&ao_gps_mutex);
				ao_wakeup(&ao_gps_data);
			}
		} else if (a == (uint8_t) 'G' && c == (uint8_t) 'S' && i == (uint8_t) 'V') {
			uint8_t	done;
			/* Now read the data into the GPS tracking data record
			 *
			 * $GPGSV,3,1,12,05,54,069,45,12,44,061,44,21,07,184,46,22,78,289,47*72<CR><LF>
			 *
			 * Satellites in view data
			 *
			 *	3		Total number of GSV messages
			 *	1		Sequence number of current GSV message
			 *	12		Total sats in view (0-12)
			 *	05		SVID
			 *	54		Elevation
			 *	069		Azimuth
			 *	45		C/N0 in dB
			 *	...		other SVIDs
			 *	72		checksum
			 */
			c = ao_gps_decimal(1);	/* total messages */
			i = ao_gps_decimal(1);	/* message sequence */
			if (i == 1) {
				ao_gps_tracking_next.channels = 0;
			}
			done = (uint8_t) c == i;
			ao_gps_lexchar();
			ao_gps_skip_field();	/* sats in view */
			while (ao_gps_char != '*' && ao_gps_char != '\n' && ao_gps_char != '\r') {
				i = ao_gps_tracking_next.channels;
				ao_gps_tracking_next.sats[i].svid = ao_gps_decimal(2);	/* SVID */
				ao_gps_lexchar();
				ao_gps_skip_field();	/* elevation */
				ao_gps_lexchar();
				ao_gps_skip_field();	/* azimuth */
				if (!(ao_gps_tracking_next.sats[i].c_n_1 = ao_gps_decimal(2)))	/* C/N0 */
					ao_gps_tracking_next.sats[i].svid = 0;
				ao_gps_tracking_next.channels = i + 1;
			}
			if (ao_gps_char == '*') {
				uint8_t cksum = ao_gps_cksum ^ '*';
				if (cksum != ao_gps_hex(2))
					ao_gps_error = 1;
			}
			else
				ao_gps_error = 1;
			if (ao_gps_error)
				ao_gps_tracking_next.channels = 0;
			else if (done) {
				ao_mutex_get(&ao_gps_mutex);
				memcpy(&ao_gps_tracking_data, &ao_gps_tracking_next,
				       sizeof(ao_gps_tracking_data));
				ao_mutex_put(&ao_gps_mutex);
				ao_wakeup(&ao_gps_tracking_data);
			}
		} else if (a == (uint8_t) 'R' && c == (uint8_t) 'M' && i == (uint8_t) 'C') {
			/* Parse the RMC record to read out the current date */

			/* $GPRMC,111636.932,A,2447.0949,N,12100.5223,E,000.0,000.0,030407,,,A*61
			 *
			 * Recommended Minimum Specific GNSS Data
			 *
			 *	111636.932	UTC time 11:16:36.932
			 *	A		Data Valid (V = receiver warning)
			 *	2447.0949	Latitude
			 *	N		North/south indicator
			 *	12100.5223	Longitude
			 *	E		East/west indicator
			 *	000.0		Speed over ground
			 *	000.0		Course over ground
			 *	030407		UTC date (ddmmyy format)
			 *	A		Mode indicator:
			 *			N = data not valid
			 *			A = autonomous mode
			 *			D = differential mode
			 *			E = estimated (dead reckoning) mode
			 *			M = manual input mode
			 *			S = simulator mode
			 *	61		checksum
			 */
			ao_gps_skip_field();
			for (i = 0; i < 8; i++) {
				ao_gps_lexchar();
				ao_gps_skip_field();
			}
			a = ao_gps_decimal(2);
			c = ao_gps_decimal(2);
			i = ao_gps_decimal(2);
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
				ao_gps_next.year = i;
				ao_gps_next.month = c;
				ao_gps_next.day = a;
				ao_gps_date_flags = AO_GPS_DATE_VALID;
			}
		}
	}
}

__xdata struct ao_task ao_gps_task;

static void
gps_dump(void) __reentrant
{
	ao_mutex_get(&ao_gps_mutex);
	printf ("Date: %02d/%02d/%02d\n", ao_gps_data.year, ao_gps_data.month, ao_gps_data.day);
	printf ("Time: %02d:%02d:%02d\n", ao_gps_data.hour, ao_gps_data.minute, ao_gps_data.second);
	printf ("Lat/Lon: %ld %ld\n", ao_gps_data.latitude, ao_gps_data.longitude);
	printf ("Alt: %d\n", ao_gps_data.altitude);
	printf ("Flags: 0x%x\n", ao_gps_data.flags);
	ao_mutex_put(&ao_gps_mutex);
}

__code struct ao_cmds ao_gps_cmds[] = {
	{ 'g', gps_dump, 	"g                                  Display current GPS values" },
	{ 0,   gps_dump, NULL },
};

void
ao_gps_init(void)
{
	ao_add_task(&ao_gps_task, ao_gps, "gps");
	ao_cmd_register(&ao_gps_cmds[0]);
}
