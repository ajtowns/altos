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

#ifndef AO_GPS_TEST
#include "ao.h"
#endif

#include "ao_gps_ublox.h"

__xdata uint8_t ao_gps_mutex;
__pdata uint16_t ao_gps_tick;
__xdata struct ao_telemetry_location	ao_gps_data;
__xdata struct ao_telemetry_satellite	ao_gps_tracking_data;

static const char ao_gps_set_nmea[] = "\r\n$PUBX,41,1,3,1,57600,0*2d\r\n";

const char ao_gps_config[] = {

};

struct ao_ublox_cksum {
	uint8_t	a, b;
};

static __pdata struct ao_ublox_cksum ao_ublox_cksum;
static __pdata uint16_t ao_ublox_len;

#ifndef ao_ublox_getchar
#define ao_ublox_getchar	ao_serial1_getchar
#define ao_ublox_putchar	ao_serial1_putchar
#define ao_ublox_set_speed	ao_serial1_set_speed
#endif

#define ao_ublox_byte()	((uint8_t) ao_ublox_getchar())

static inline void add_cksum(struct ao_ublox_cksum *cksum, uint8_t c)
{
	cksum->a += c;
	cksum->b += cksum->a;
}

static void ao_ublox_init_cksum(void)
{
	ao_ublox_cksum.a = ao_ublox_cksum.b = 0;
}

static void ao_ublox_putchar_cksum(uint8_t c)
{
	add_cksum(&ao_ublox_cksum, c);
	ao_ublox_putchar(c);
}

static uint8_t header_byte(void)
{
	uint8_t	c = ao_ublox_byte();
	add_cksum(&ao_ublox_cksum, c);
	return c;
}

static uint8_t data_byte(void)
{
	--ao_ublox_len;
	return header_byte();
}

static char __xdata *ublox_target;

static void ublox_u16(uint8_t offset)
{
	uint16_t __xdata *ptr = (uint16_t __xdata *) (ublox_target + offset);
	uint16_t val;

	val = data_byte();
	val |= data_byte () << 8;
	*ptr = val;
}

static void ublox_u8(uint8_t offset)
{
	uint8_t __xdata *ptr = (uint8_t __xdata *) (ublox_target + offset);
	uint8_t val;

	val = data_byte ();
	*ptr = val;
}

static void ublox_u32(uint8_t offset) __reentrant
{
	uint32_t __xdata *ptr = (uint32_t __xdata *) (ublox_target + offset);
	uint32_t val;

	val = ((uint32_t) data_byte ());
	val |= ((uint32_t) data_byte ()) << 8;
	val |= ((uint32_t) data_byte ()) << 16;
	val |= ((uint32_t) data_byte ()) << 24;
	*ptr = val;
}

static void ublox_discard(uint8_t len)
{
	while (len--)
		data_byte();
}

#define UBLOX_END	0
#define UBLOX_DISCARD	1
#define UBLOX_U8	2
#define UBLOX_U16	3
#define UBLOX_U32	4

struct ublox_packet_parse {
	uint8_t	type;
	uint8_t	offset;
};

static void
ao_ublox_parse(void __xdata *target, const struct ublox_packet_parse *parse) __reentrant
{
	uint8_t	i, offset;

	ublox_target = target;
	for (i = 0; ; i++) {
		offset = parse[i].offset;
		switch (parse[i].type) {
		case UBLOX_END:
			return;
		case UBLOX_DISCARD:
			ublox_discard(offset);
			break;
		case UBLOX_U8:
			ublox_u8(offset);
			break;
		case UBLOX_U16:
			ublox_u16(offset);
			break;
		case UBLOX_U32:
			ublox_u32(offset);
			break;
		}
	}
}

/*
 * NAV-DOP message parsing
 */

static struct nav_dop {
	uint16_t	pdop;
	uint16_t	hdop;
	uint16_t	vdop;
} nav_dop;

static const struct ublox_packet_parse nav_dop_packet[] = {
	{ UBLOX_DISCARD, 6 },					/* 0 GPS Millisecond Time of Week, gDOP */
	{ UBLOX_U16, offsetof(struct nav_dop, pdop) },		/* 6 pDOP */
	{ UBLOX_DISCARD, 2 },					/* 8 tDOP */
	{ UBLOX_U16, offsetof(struct nav_dop, vdop) },		/* 10 vDOP */
	{ UBLOX_U16, offsetof(struct nav_dop, hdop) },		/* 12 hDOP */
	{ UBLOX_DISCARD, 4 },					/* 14 nDOP, eDOP */
	{ UBLOX_END, 0 }
};

static void
ao_ublox_parse_nav_dop(void)
{
	ao_ublox_parse(&nav_dop, nav_dop_packet);
}

/*
 * NAV-POSLLH message parsing
 */
static struct nav_posllh {
	int32_t		lat;
	int32_t		lon;
	int32_t		alt_msl;
} nav_posllh;

static const struct ublox_packet_parse nav_posllh_packet[] = {
	{ UBLOX_DISCARD, 4 },						/* 0 GPS Millisecond Time of Week */
	{ UBLOX_U32, offsetof(struct nav_posllh, lon) },		/* 4 Longitude */
	{ UBLOX_U32, offsetof(struct nav_posllh, lat) },		/* 8 Latitude */
	{ UBLOX_DISCARD, 4 },						/* 12 Height above Ellipsoid */
	{ UBLOX_U32, offsetof(struct nav_posllh, alt_msl) },		/* 16 Height above mean sea level */
	{ UBLOX_DISCARD, 8 },						/* 20 hAcc, vAcc */
	{ UBLOX_END, 0 },						/* 28 */
};

static void
ao_ublox_parse_nav_posllh(void)
{
	ao_ublox_parse(&nav_posllh, nav_posllh_packet);
}

/*
 * NAV-SOL message parsing
 */
static struct nav_sol {
	uint8_t		gps_fix;
	uint8_t		flags;
	uint8_t		nsat;
} nav_sol;

static const struct ublox_packet_parse nav_sol_packet[] = {
	{ UBLOX_DISCARD, 10 },						/* 0 iTOW, fTOW, week */
	{ UBLOX_U8, offsetof(struct nav_sol, gps_fix) },		/* 10 gpsFix */
	{ UBLOX_U8, offsetof(struct nav_sol, flags) },			/* 11 flags */
	{ UBLOX_DISCARD, 35 },						/* 12 ecefX, ecefY, ecefZ, pAcc, ecefVX, ecefVY, ecefVZ, sAcc, pDOP, reserved1 */
	{ UBLOX_U8, offsetof(struct nav_sol, nsat) },			/* 47 numSV */
	{ UBLOX_DISCARD, 4 },						/* 48 reserved2 */
	{ UBLOX_END, 0 }
};

#define NAV_SOL_FLAGS_GPSFIXOK		0
#define NAV_SOL_FLAGS_DIFFSOLN		1
#define NAV_SOL_FLAGS_WKNSET		2
#define NAV_SOL_FLAGS_TOWSET		3

static void
ao_ublox_parse_nav_sol(void)
{
	ao_ublox_parse(&nav_sol, nav_sol_packet);
}

/*
 * NAV-SVINFO message parsing
 */

static struct nav_svinfo {
	uint8_t	num_ch;
	uint8_t flags;
} nav_svinfo;

static const struct ublox_packet_parse nav_svinfo_packet[] = {
	{ UBLOX_DISCARD, 4 },						/* 0 iTOW */
	{ UBLOX_U8, offsetof(struct nav_svinfo, num_ch) },		/* 4 numCh */
	{ UBLOX_U8, offsetof(struct nav_svinfo, flags) },		/* 5 globalFlags */
	{ UBLOX_DISCARD, 2 },						/* 6 reserved2 */
	{ UBLOX_END, 0 }
};

#define NAV_SVINFO_MAX_SAT	16

static struct nav_svinfo_sat {
	uint8_t	chn;
	uint8_t svid;
	uint8_t flags;
	uint8_t quality;
	uint8_t cno;
} nav_svinfo_sat[NAV_SVINFO_MAX_SAT];

static uint8_t	nav_svinfo_nsat;

static const struct ublox_packet_parse nav_svinfo_sat_packet[] = {
	{ UBLOX_U8, offsetof(struct nav_svinfo_sat, chn) },		/* 8 + 12*N chn */
	{ UBLOX_U8, offsetof(struct nav_svinfo_sat, svid) },		/* 9 + 12*N svid */
	{ UBLOX_U8, offsetof(struct nav_svinfo_sat, flags) },		/* 10 + 12*N flags */
	{ UBLOX_U8, offsetof(struct nav_svinfo_sat, quality) },		/* 11 + 12*N quality */
	{ UBLOX_U8, offsetof(struct nav_svinfo_sat, cno) },		/* 12 + 12*N cno */
	{ UBLOX_DISCARD, 7 },						/* 13 + 12*N elev, azim, prRes */
	{ UBLOX_END, 0 }
};

#define NAV_SVINFO_SAT_FLAGS_SVUSED		0
#define NAV_SVINFO_SAT_FLAGS_DIFFCORR		1
#define NAV_SVINFO_SAT_FLAGS_ORBITAVAIL		2
#define NAV_SVINFO_SAT_FLAGS_ORBITEPH		3
#define NAV_SVINFO_SAT_FLAGS_UNHEALTHY		4
#define NAV_SVINFO_SAT_FLAGS_ORBITALM		5
#define NAV_SVINFO_SAT_FLAGS_ORBITAOP		6
#define NAV_SVINFO_SAT_FLAGS_SMOOTHED		7

#define NAV_SVINFO_SAT_QUALITY_IDLE		0
#define NAV_SVINFO_SAT_QUALITY_SEARCHING	1
#define NAV_SVINFO_SAT_QUALITY_ACQUIRED		2
#define NAV_SVINFO_SAT_QUALITY_UNUSABLE		3
#define NAV_SVINFO_SAT_QUALITY_LOCKED		4
#define NAV_SVINFO_SAT_QUALITY_RUNNING		5

static void
ao_ublox_parse_nav_svinfo(void)
{
	uint8_t	nsat;
	nav_svinfo_nsat = 0;
	ao_ublox_parse(&nav_svinfo, nav_svinfo_packet);
	for (nsat = 0; nsat < nav_svinfo.num_ch && ao_ublox_len >= 12; nsat++) {
		if (nsat < NAV_SVINFO_MAX_SAT) {
			ao_ublox_parse(&nav_svinfo_sat[nav_svinfo_nsat++], nav_svinfo_sat_packet);
		} else {
			ublox_discard(12);
		}
	}
}

/*
 * NAV-TIMEUTC message parsing
 */
static struct nav_timeutc {
	uint16_t	year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t		min;
	uint8_t		sec;
	uint8_t		valid;
} nav_timeutc;

#define NAV_TIMEUTC_VALID_TOW	0
#define NAV_TIMEUTC_VALID_WKN	1
#define NAV_TIMEUTC_VALID_UTC	2

static const struct ublox_packet_parse nav_timeutc_packet[] = {
	{ UBLOX_DISCARD, 12 },						/* 0 iTOW, tAcc, nano */
	{ UBLOX_U16, offsetof(struct nav_timeutc, year) },		/* 12 year */
	{ UBLOX_U8, offsetof(struct nav_timeutc, month) },		/* 14 month */
	{ UBLOX_U8, offsetof(struct nav_timeutc, day) },		/* 15 day */
	{ UBLOX_U8, offsetof(struct nav_timeutc, hour) },		/* 16 hour */
	{ UBLOX_U8, offsetof(struct nav_timeutc, min) },		/* 17 min */
	{ UBLOX_U8, offsetof(struct nav_timeutc, sec) },		/* 18 sec */
	{ UBLOX_U8, offsetof(struct nav_timeutc, valid) },		/* 19 valid */
	{ UBLOX_END, 0 }
};

static void
ao_ublox_parse_nav_timeutc(void)
{
	ao_ublox_parse(&nav_timeutc, nav_timeutc_packet);
}

/*
 * NAV-VELNED message parsing
 */

static struct nav_velned {
	int32_t		vel_d;
	uint32_t	g_speed;
	int32_t		heading;
} nav_velned;

static const struct ublox_packet_parse nav_velned_packet[] = {
	{ UBLOX_DISCARD, 12 },						/* 0 iTOW, velN, velE */
	{ UBLOX_U32, offsetof(struct nav_velned, vel_d) },		/* 12 velD */
	{ UBLOX_DISCARD, 4 },						/* 16 speed */
	{ UBLOX_U32, offsetof(struct nav_velned, g_speed) },		/* 20 gSpeed */
	{ UBLOX_U32, offsetof(struct nav_velned, heading) },		/* 24 heading */
	{ UBLOX_DISCARD, 8 },						/* 28 sAcc, cAcc */
	{ UBLOX_END, 0 }
};

static void
ao_ublox_parse_nav_velned(void)
{
	ao_ublox_parse(&nav_velned, nav_velned_packet);
}

/*
 * Set the protocol mode and baud rate
 */

static void
ao_gps_setup(void)
{
	uint8_t	i, k;
	ao_ublox_set_speed(AO_SERIAL_SPEED_9600);

	/*
	 * A bunch of nulls so the start bit
	 * is clear
	 */
	for (i = 0; i < 64; i++)
		ao_ublox_putchar(0x00);

	/*
	 * Send the baud-rate setting and protocol-setting
	 * command three times
	 */
	for (k = 0; k < 3; k++)
		for (i = 0; i < sizeof (ao_gps_set_nmea); i++)
			ao_ublox_putchar(ao_gps_set_nmea[i]);

	/*
	 * Increase the baud rate
	 */
	ao_ublox_set_speed(AO_SERIAL_SPEED_57600);

	/*
	 * Pad with nulls to give the chip
	 * time to see the baud rate switch
	 */
	for (i = 0; i < 64; i++)
		ao_ublox_putchar(0x00);
}

void
ao_ublox_putstart(uint8_t class, uint8_t id, uint16_t len)
{
	ao_ublox_init_cksum();
	ao_ublox_putchar(0xb5);
	ao_ublox_putchar(0x62);
	ao_ublox_putchar_cksum(class);
	ao_ublox_putchar_cksum(id);
	ao_ublox_putchar_cksum(len);
	ao_ublox_putchar_cksum(len >> 8);
}

void
ao_ublox_putend(void)
{
	ao_ublox_putchar(ao_ublox_cksum.a);
	ao_ublox_putchar(ao_ublox_cksum.b);
}

void
ao_ublox_set_message_rate(uint8_t class, uint8_t msgid, uint8_t rate)
{
	ao_ublox_putstart(0x06, 0x01, 3);
	ao_ublox_putchar_cksum(class);
	ao_ublox_putchar_cksum(msgid);
	ao_ublox_putchar_cksum(rate);
	ao_ublox_putend();
}

/*
 * Disable all MON message
 */
static const uint8_t ublox_disable_mon[] = {
	0x0b, 0x09, 0x02, 0x06, 0x07, 0x21, 0x08, 0x04
};

/*
 * Disable all NAV messages. The desired
 * ones will be explicitly re-enabled
 */

static const uint8_t ublox_disable_nav[] = {
	0x60, 0x22, 0x31, 0x04, 0x40, 0x01, 0x02, 0x32,
	0x06, 0x03, 0x30, 0x20, 0x21, 0x11, 0x12
};

/*
 * Enable enough messages to get all of the data we want
 */
static const uint8_t ublox_enable_nav[] = {
	UBLOX_NAV_DOP,
	UBLOX_NAV_POSLLH,
	UBLOX_NAV_SOL,
	UBLOX_NAV_SVINFO,
	UBLOX_NAV_VELNED,
	UBLOX_NAV_TIMEUTC
};

void
ao_gps(void) __reentrant
{
	uint8_t			class, id;
	struct ao_ublox_cksum	cksum;
	uint8_t			i;

	ao_gps_setup();

	/* Disable all messages */
	for (i = 0; i < sizeof (ublox_disable_mon); i++)
		ao_ublox_set_message_rate(0x0a, ublox_disable_mon[i], 0);
	for (i = 0; i < sizeof (ublox_disable_nav); i++)
		ao_ublox_set_message_rate(UBLOX_NAV, ublox_disable_nav[i], 0);

	/* Enable all of the messages we want */
	for (i = 0; i < sizeof (ublox_enable_nav); i++)
		ao_ublox_set_message_rate(UBLOX_NAV, ublox_enable_nav[i], 1);
	
	for (;;) {
		/* Locate the begining of the next record */
		while (ao_ublox_byte() != (uint8_t) 0xb5)
			;
		if (ao_ublox_byte() != (uint8_t) 0x62)
			continue;

		ao_ublox_init_cksum();

		class = header_byte();
		id = header_byte();

		/* Length */
		ao_ublox_len = header_byte();
		ao_ublox_len |= header_byte() << 8;

		if (ao_ublox_len > 1023)
			continue;

		switch (class) {
		case UBLOX_NAV:
			switch (id) {
			case UBLOX_NAV_DOP:
				if (ao_ublox_len != 18)
					break;
				ao_ublox_parse_nav_dop();
				break;
			case UBLOX_NAV_POSLLH:
				if (ao_ublox_len != 28)
					break;
				ao_ublox_parse_nav_posllh();
				break;
			case UBLOX_NAV_SOL:
				if (ao_ublox_len != 52)
					break;
				ao_ublox_parse_nav_sol();
				break;
			case UBLOX_NAV_SVINFO:
				if (ao_ublox_len < 8)
					break;
				ao_ublox_parse_nav_svinfo();
				break;
			case UBLOX_NAV_VELNED:
				if (ao_ublox_len != 36)
					break;
				ao_ublox_parse_nav_velned();
				break;
			case UBLOX_NAV_TIMEUTC:
				if (ao_ublox_len != 20)
					break;
				ao_ublox_parse_nav_timeutc();
				break;
			}
			break;
		}

		if (ao_ublox_len != 0)
			continue;

		/* verify checksum and end sequence */
		cksum.a = ao_ublox_byte();
		cksum.b = ao_ublox_byte();
		if (ao_ublox_cksum.a != cksum.a || ao_ublox_cksum.b != cksum.b)
			continue;

		switch (class) {
		case 0x01:
			switch (id) {
			case 0x21:
				ao_mutex_get(&ao_gps_mutex);
				ao_gps_tick = ao_time();

				ao_gps_data.flags = 0;
				ao_gps_data.flags |= AO_GPS_RUNNING;
				if (nav_sol.gps_fix & (1 << NAV_SOL_FLAGS_GPSFIXOK)) {
					uint8_t	nsat = nav_sol.nsat;
					ao_gps_data.flags |= AO_GPS_VALID;
					if (nsat > 15)
						nsat = 15;
					ao_gps_data.flags |= nsat;
				}
				if (nav_timeutc.valid & (1 << NAV_TIMEUTC_VALID_UTC))
					ao_gps_data.flags |= AO_GPS_DATE_VALID;
				
				ao_gps_data.altitude = nav_posllh.alt_msl / 1000;
				ao_gps_data.latitude = nav_posllh.lat;
				ao_gps_data.longitude = nav_posllh.lon;

				ao_gps_data.year = nav_timeutc.year - 2000;
				ao_gps_data.month = nav_timeutc.month;
				ao_gps_data.day = nav_timeutc.day;

				ao_gps_data.hour = nav_timeutc.hour;
				ao_gps_data.minute = nav_timeutc.min;
				ao_gps_data.second = nav_timeutc.sec;

				ao_gps_data.pdop = nav_dop.pdop;
				ao_gps_data.hdop = nav_dop.hdop;
				ao_gps_data.vdop = nav_dop.vdop;

				/* mode is not set */

				ao_gps_data.ground_speed = nav_velned.g_speed;
				ao_gps_data.climb_rate = -nav_velned.vel_d;
				ao_gps_data.course = nav_velned.heading / 200000;
				
				ao_gps_tracking_data.channels = 0;

				struct ao_telemetry_satellite_info *dst = &ao_gps_tracking_data.sats[0];

				for (i = 0; i < nav_svinfo_nsat; i++) {
					struct nav_svinfo_sat *src = &nav_svinfo_sat[i];

					if (!(src->flags & (1 << NAV_SVINFO_SAT_FLAGS_UNHEALTHY)) &&
					    src->quality >= NAV_SVINFO_SAT_QUALITY_ACQUIRED)
					{
						dst->svid = src->svid;
						dst->c_n_1 = src->cno;
						dst++;
						ao_gps_tracking_data.channels++;
					}
				}

				ao_mutex_put(&ao_gps_mutex);
				ao_wakeup(&ao_gps_data);
				ao_wakeup(&ao_gps_tracking_data);
				break;
			}
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
