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

__xdata uint8_t ao_gps_mutex;
__pdata uint16_t ao_gps_tick;
__xdata struct ao_telemetry_location	ao_gps_data;
__xdata struct ao_telemetry_satellite	ao_gps_tracking_data;

static const char ao_gps_set_nmea[] = "\r\n$PSRF100,0,57600,8,1,0*37\r\n";

const char ao_gps_config[] = {

	0xa0, 0xa2, 0x00, 0x0e,	/* length: 14 bytes */
	136,			/* mode control */
	0, 0,			/* reserved */
	0,			/* degraded mode (allow 1-SV navigation) */
	0, 0,			/* reserved */
	0, 0,			/* user specified altitude */
	2,			/* alt hold mode (disabled, require 3d fixes) */
	0,			/* alt hold source (use last computed altitude) */
	0,			/* reserved */
	10,			/* Degraded time out (10 sec) */
	10,			/* Dead Reckoning time out (10 sec) */
	0,			/* Track smoothing (disabled) */
	0x00, 0x8e, 0xb0, 0xb3,

	0xa0, 0xa2, 0x00, 0x08,	/* length: 8 bytes */
	166,			/* Set message rate */
	2,			/* enable/disable all messages */
	0,			/* message id (ignored) */
	0,			/* update rate (0 = disable) */
	0, 0, 0, 0,		/* reserved */
	0x00, 0xa8, 0xb0, 0xb3,

	0xa0, 0xa2, 0x00, 0x02,	/* length: 2 bytes */
	143,			/* static navigation */
	0,			/* disable */
	0x00, 0x8f, 0xb0, 0xb3,
};

#define NAV_TYPE_GPS_FIX_TYPE_MASK			(7 << 0)
#define NAV_TYPE_NO_FIX					(0 << 0)
#define NAV_TYPE_SV_KF					(1 << 0)
#define NAV_TYPE_2_SV_KF				(2 << 0)
#define NAV_TYPE_3_SV_KF				(3 << 0)
#define NAV_TYPE_4_SV_KF				(4 << 0)
#define NAV_TYPE_2D_LEAST_SQUARES			(5 << 0)
#define NAV_TYPE_3D_LEAST_SQUARES			(6 << 0)
#define NAV_TYPE_DR					(7 << 0)
#define NAV_TYPE_TRICKLE_POWER				(1 << 3)
#define NAV_TYPE_ALTITUDE_HOLD_MASK			(3 << 4)
#define NAV_TYPE_ALTITUDE_HOLD_NONE			(0 << 4)
#define NAV_TYPE_ALTITUDE_HOLD_KF			(1 << 4)
#define NAV_TYPE_ALTITUDE_HOLD_USER			(2 << 4)
#define NAV_TYPE_ALTITUDE_HOLD_ALWAYS			(3 << 4)
#define NAV_TYPE_DOP_LIMIT_EXCEEDED			(1 << 6)
#define NAV_TYPE_DGPS_APPLIED				(1 << 7)
#define NAV_TYPE_SENSOR_DR				(1 << 8)
#define NAV_TYPE_OVERDETERMINED				(1 << 9)
#define NAV_TYPE_DR_TIMEOUT_EXCEEDED			(1 << 10)
#define NAV_TYPE_FIX_MI_EDIT				(1 << 11)
#define NAV_TYPE_INVALID_VELOCITY			(1 << 12)
#define NAV_TYPE_ALTITUDE_HOLD_DISABLED			(1 << 13)
#define NAV_TYPE_DR_ERROR_STATUS_MASK			(3 << 14)
#define NAV_TYPE_DR_ERROR_STATUS_GPS_ONLY		(0 << 14)
#define NAV_TYPE_DR_ERROR_STATUS_DR_FROM_GPS		(1 << 14)
#define NAV_TYPE_DR_ERROR_STATUS_DR_SENSOR_ERROR	(2 << 14)
#define NAV_TYPE_DR_ERROR_STATUS_DR_IN_TEST		(3 << 14)

struct sirf_geodetic_nav_data {
	uint16_t	nav_type;
	uint16_t	utc_year;
	uint8_t 	utc_month;
	uint8_t 	utc_day;
	uint8_t 	utc_hour;
	uint8_t 	utc_minute;
	uint16_t 	utc_second;
	int32_t		lat;
	int32_t		lon;
	int32_t		alt_msl;
	uint16_t	ground_speed;
	uint16_t	course;
	int16_t		climb_rate;
	uint32_t	h_error;
	uint32_t	v_error;
	uint8_t		num_sv;
	uint8_t		hdop;
};

static __xdata struct sirf_geodetic_nav_data	ao_sirf_data;

struct sirf_measured_sat_data {
	uint8_t		svid;
	uint8_t		c_n_1;
};

struct sirf_measured_tracker_data {
	int16_t				gps_week;
	uint32_t			gps_tow;
	uint8_t				channels;
	struct sirf_measured_sat_data	sats[12];
};

static __xdata struct sirf_measured_tracker_data	ao_sirf_tracker_data;

static __pdata uint16_t ao_sirf_cksum;
static __pdata uint16_t ao_sirf_len;

#define ao_sirf_byte()	((uint8_t) ao_serial_getchar())

static uint8_t data_byte(void)
{
	uint8_t	c = ao_sirf_byte();
	--ao_sirf_len;
	ao_sirf_cksum += c;
	return c;
}

static char __xdata *sirf_target;

static void sirf_u16(uint8_t offset)
{
	uint16_t __xdata *ptr = (uint16_t __xdata *) (sirf_target + offset);
	uint16_t val;

	val = data_byte() << 8;
	val |= data_byte ();
	*ptr = val;
}

static void sirf_u8(uint8_t offset)
{
	uint8_t __xdata *ptr = (uint8_t __xdata *) (sirf_target + offset);
	uint8_t val;

	val = data_byte ();
	*ptr = val;
}

static void sirf_u32(uint8_t offset) __reentrant
{
	uint32_t __xdata *ptr = (uint32_t __xdata *) (sirf_target + offset);
	uint32_t val;

	val = ((uint32_t) data_byte ()) << 24;
	val |= ((uint32_t) data_byte ()) << 16;
	val |= ((uint32_t) data_byte ()) << 8;
	val |= ((uint32_t) data_byte ());
	*ptr = val;
}

static void sirf_discard(uint8_t len)
{
	while (len--)
		data_byte();
}

#define SIRF_END	0
#define SIRF_DISCARD	1
#define SIRF_U8		2
#define SIRF_U16	3
#define SIRF_U32	4
#define SIRF_U8X10	5

struct sirf_packet_parse {
	uint8_t	type;
	uint8_t	offset;
};

static void
ao_sirf_parse(void __xdata *target, const struct sirf_packet_parse *parse) __reentrant
{
	uint8_t	i, offset, j;

	sirf_target = target;
	for (i = 0; ; i++) {
		offset = parse[i].offset;
		switch (parse[i].type) {
		case SIRF_END:
			return;
		case SIRF_DISCARD:
			sirf_discard(offset);
			break;
		case SIRF_U8:
			sirf_u8(offset);
			break;
		case SIRF_U16:
			sirf_u16(offset);
			break;
		case SIRF_U32:
			sirf_u32(offset);
			break;
		case SIRF_U8X10:
			for (j = 10; j--;)
				sirf_u8(offset++);
			break;
		}
	}
}

static const struct sirf_packet_parse geodetic_nav_data_packet[] = {
	{ SIRF_DISCARD, 2 },							/* 1 nav valid */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, nav_type) },	/* 3 */
	{ SIRF_DISCARD, 6 },							/* 5 week number, time of week */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, utc_year) },	/* 11 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, utc_month) },	/* 13 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, utc_day) },		/* 14 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, utc_hour) },		/* 15 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, utc_minute) },	/* 16 */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, utc_second) },	/* 17 */
	{ SIRF_DISCARD, 4 },	/* satellite id list */				/* 19 */
	{ SIRF_U32, offsetof(struct sirf_geodetic_nav_data, lat) },		/* 23 */
	{ SIRF_U32, offsetof(struct sirf_geodetic_nav_data, lon) },		/* 27 */
	{ SIRF_DISCARD, 4 },	/* altitude from ellipsoid */			/* 31 */
	{ SIRF_U32, offsetof(struct sirf_geodetic_nav_data, alt_msl) },		/* 35 */
	{ SIRF_DISCARD, 1 },	/* map datum */					/* 39 */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, ground_speed) },	/* 40 */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, course) },		/* 42 */
	{ SIRF_DISCARD, 2 },	/* magnetic variation */			/* 44 */
	{ SIRF_U16, offsetof(struct sirf_geodetic_nav_data, climb_rate) },	/* 46 */
	{ SIRF_DISCARD, 2 },	/* turn rate */					/* 48 */
	{ SIRF_U32, offsetof(struct sirf_geodetic_nav_data, h_error) },		/* 50 */
	{ SIRF_U32, offsetof(struct sirf_geodetic_nav_data, v_error) },		/* 54 */
	{ SIRF_DISCARD, 30 },	/* time error, h_vel error, clock_bias,
				   clock bias error, clock drift,
				   clock drift error, distance,
				   distance error, heading error */		/* 58 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, num_sv) },		/* 88 */
	{ SIRF_U8, offsetof(struct sirf_geodetic_nav_data, hdop) },		/* 89 */
	{ SIRF_DISCARD, 1 },	/* additional mode info */			/* 90 */
	{ SIRF_END, 0 },							/* 91 */
};

static void
ao_sirf_parse_41(void) __reentrant
{
	ao_sirf_parse(&ao_sirf_data, geodetic_nav_data_packet);
}

static const struct sirf_packet_parse measured_tracker_data_packet[] = {
	{ SIRF_U16, offsetof (struct sirf_measured_tracker_data, gps_week) },	/* 1 week */
	{ SIRF_U32, offsetof (struct sirf_measured_tracker_data, gps_tow) },	/* 3 time of week */
	{ SIRF_U8, offsetof (struct sirf_measured_tracker_data, channels) },	/* 7 channels */
	{ SIRF_END, 0 },
};

static const struct sirf_packet_parse measured_sat_data_packet[] = {
	{ SIRF_U8, offsetof (struct sirf_measured_sat_data, svid) },		/* 0 SV id */
	{ SIRF_DISCARD, 4 },							/* 1 azimuth, 2 elevation, 3 state */
	{ SIRF_U8, offsetof (struct sirf_measured_sat_data, c_n_1) },		/* C/N0 1 */
	{ SIRF_DISCARD, 9 },							/* C/N0 2-10 */
	{ SIRF_END, 0 },
};

static void
ao_sirf_parse_4(void) __reentrant
{
	uint8_t	i;
	ao_sirf_parse(&ao_sirf_tracker_data, measured_tracker_data_packet);
	for (i = 0; i < 12; i++)
		ao_sirf_parse(&ao_sirf_tracker_data.sats[i], measured_sat_data_packet);
}

static void
ao_gps_setup(void) __reentrant
{
	uint8_t	i, k;
	ao_serial_set_speed(AO_SERIAL_SPEED_4800);
	for (i = 0; i < 64; i++)
		ao_serial_putchar(0x00);
	for (k = 0; k < 3; k++)
		for (i = 0; i < sizeof (ao_gps_set_nmea); i++)
			ao_serial_putchar(ao_gps_set_nmea[i]);
	ao_serial_set_speed(AO_SERIAL_SPEED_57600);
	for (i = 0; i < 64; i++)
		ao_serial_putchar(0x00);
}

static const char ao_gps_set_message_rate[] = {
	0xa0, 0xa2, 0x00, 0x08,
	166,
	0,
};

void
ao_sirf_set_message_rate(uint8_t msg, uint8_t rate) __reentrant
{
	uint16_t	cksum = 0x00a6;
	uint8_t		i;

	for (i = 0; i < sizeof (ao_gps_set_message_rate); i++)
		ao_serial_putchar(ao_gps_set_message_rate[i]);
	ao_serial_putchar(msg);
	ao_serial_putchar(rate);
	cksum = 0xa6 + msg + rate;
	for (i = 0; i < 4; i++)
		ao_serial_putchar(0);
	ao_serial_putchar((cksum >> 8) & 0x7f);
	ao_serial_putchar(cksum & 0xff);
	ao_serial_putchar(0xb0);
	ao_serial_putchar(0xb3);
}

static const uint8_t sirf_disable[] = {
	2,
	9,
	10,
	27,
	50,
	52,
};

void
ao_gps(void) __reentrant
{
	uint8_t	i, k;
	uint16_t cksum;

	ao_gps_setup();
	for (k = 0; k < 5; k++)
	{
		for (i = 0; i < sizeof (ao_gps_config); i++)
			ao_serial_putchar(ao_gps_config[i]);
		for (i = 0; i < sizeof (sirf_disable); i++)
			ao_sirf_set_message_rate(sirf_disable[i], 0);
		ao_sirf_set_message_rate(41, 1);
		ao_sirf_set_message_rate(4, 1);
	}
	for (;;) {
		/* Locate the begining of the next record */
		while (ao_sirf_byte() != (uint8_t) 0xa0)
			;
		if (ao_sirf_byte() != (uint8_t) 0xa2)
			continue;

		/* Length */
		ao_sirf_len = ao_sirf_byte() << 8;
		ao_sirf_len |= ao_sirf_byte();
		if (ao_sirf_len > 1023)
			continue;

		ao_sirf_cksum = 0;

		/* message ID */
		i = data_byte ();							/* 0 */

		switch (i) {
		case 41:
			if (ao_sirf_len < 90)
				break;
			ao_sirf_parse_41();
			break;
		case 4:
			if (ao_sirf_len < 187)
				break;
			ao_sirf_parse_4();
			break;
		}
		if (ao_sirf_len != 0)
			continue;

		/* verify checksum and end sequence */
		ao_sirf_cksum &= 0x7fff;
		cksum = ao_sirf_byte() << 8;
		cksum |= ao_sirf_byte();
		if (ao_sirf_cksum != cksum)
			continue;
		if (ao_sirf_byte() != (uint8_t) 0xb0)
			continue;
		if (ao_sirf_byte() != (uint8_t) 0xb3)
			continue;

		switch (i) {
		case 41:
			ao_mutex_get(&ao_gps_mutex);
			ao_gps_tick = ao_time();
			ao_gps_data.hour = ao_sirf_data.utc_hour;
			ao_gps_data.minute = ao_sirf_data.utc_minute;
			ao_gps_data.second = ao_sirf_data.utc_second / 1000;
			ao_gps_data.flags = ((ao_sirf_data.num_sv << AO_GPS_NUM_SAT_SHIFT) & AO_GPS_NUM_SAT_MASK) | AO_GPS_RUNNING;
			if ((ao_sirf_data.nav_type & NAV_TYPE_GPS_FIX_TYPE_MASK) >= NAV_TYPE_4_SV_KF)
				ao_gps_data.flags |= AO_GPS_VALID;
			ao_gps_data.latitude = ao_sirf_data.lat;
			ao_gps_data.longitude = ao_sirf_data.lon;
			ao_gps_data.altitude = ao_sirf_data.alt_msl / 100;
			ao_gps_data.ground_speed = ao_sirf_data.ground_speed;
			ao_gps_data.course = ao_sirf_data.course / 200;
			ao_gps_data.hdop = ao_sirf_data.hdop;
			ao_gps_data.climb_rate = ao_sirf_data.climb_rate;
			ao_gps_data.flags |= AO_GPS_COURSE_VALID;
#if 0
			if (ao_sirf_data.h_error > 6553500)
				ao_gps_data.h_error = 65535;
			else
				ao_gps_data.h_error = ao_sirf_data.h_error / 100;
			if (ao_sirf_data.v_error > 6553500)
				ao_gps_data.v_error = 65535;
			else
				ao_gps_data.v_error = ao_sirf_data.v_error / 100;
#endif
			ao_mutex_put(&ao_gps_mutex);
			ao_wakeup(&ao_gps_data);
			break;
		case 4:
			ao_mutex_get(&ao_gps_mutex);
			ao_gps_tracking_data.channels = ao_sirf_tracker_data.channels;
			for (i = 0; i < 12; i++) {
				ao_gps_tracking_data.sats[i].svid = ao_sirf_tracker_data.sats[i].svid;
				ao_gps_tracking_data.sats[i].c_n_1 = ao_sirf_tracker_data.sats[i].c_n_1;
			}
			ao_mutex_put(&ao_gps_mutex);
			ao_wakeup(&ao_gps_tracking_data);
			break;
		}
	}
}

__xdata struct ao_task ao_gps_task;

void
ao_gps_init(void)
{
	ao_add_task(&ao_gps_task, ao_gps, "gps");
}
