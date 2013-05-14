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

#ifndef _AO_GPS_UBLOX_H_
#define _AO_GPS_UBLOX_H_

struct ublox_hdr {
	uint8_t		class, message;
	uint16_t	length;
};

#define UBLOX_NAV		0x01

#define UBLOX_NAV_DOP		0x04

struct ublox_nav_dop {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x04 */
	uint16_t	length;		/* 18 */

	uint32_t	itow;		/* ms */
	uint16_t	gdop;
	uint16_t	ddop;
	uint16_t	tdop;
	uint16_t	vdop;
	uint16_t	hdop;
	uint16_t	ndop;
	uint16_t	edop;
};

#define UBLOX_NAV_POSLLH	0x02

struct ublox_nav_posllh {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x02 */
	uint16_t	length;		/* 28 */

	uint32_t	itow;		/* ms */
	int32_t		lon;		/* deg * 1e7 */
	int32_t		lat;		/* deg * 1e7 */
	int32_t		height;		/* mm */
	int32_t		hmsl;		/* mm */
	uint32_t	hacc;		/* mm */
	uint32_t	vacc;		/* mm */
};

#define UBLOX_NAV_SOL		0x06

struct ublox_nav_sol {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x06 */
	uint16_t	length;		/* 52 */

	uint32_t	itow;		/* ms */
	int32_t		ftow;		/* ns */
	int16_t		week;
	int8_t		gpsfix;
	uint8_t		flags;
	int32_t		exefx;		/* cm */
	int32_t		exefy;		/* cm */
	int32_t		exefz;		/* cm */
	uint32_t	pacc;		/* cm */
	int32_t		exefvx;		/* cm/s */
	int32_t		exefvy;		/* cm/s */
	int32_t		exefvz;		/* cm/s */
	uint32_t	sacc;		/* cm/s */
	uint16_t	pdop;		/* * 100 */
	uint8_t		reserved1;
	uint8_t		numsv;
	uint32_t	reserved2;
};

#define UBLOX_NAV_SOL_GPSFIX_NO_FIX		0
#define UBLOX_NAV_SOL_GPSFIX_DEAD_RECKONING	1
#define UBLOX_NAV_SOL_GPSFIX_2D			2
#define UBLOX_NAV_SOL_GPSFIX_3D			3
#define UBLOX_NAV_SOL_GPSFIX_GPS_DEAD_RECKONING	4
#define UBLOX_NAV_SOL_GPSFIX_TIME_ONLY		5

#define UBLOX_NAV_SOL_FLAGS_GPSFIXOK		0
#define UBLOX_NAV_SOL_FLAGS_DIFFSOLN		1
#define UBLOX_NAV_SOL_FLAGS_WKNSET		2
#define UBLOX_NAV_SOL_FLAGS_TOWSET		3

#define UBLOX_NAV_STATUS	0x03

struct ublox_nav_status {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x03 */
	uint16_t	length;		/* 16 */

	uint8_t		gpsfix;
	uint8_t		flags;
	uint8_t		fixstat;
	uint8_t		flags2;

	uint32_t	ttff;		/* ms */
	uint32_t	msss;		/* ms */
};

#define UBLOX_NAV_STATUS_GPSFIX_NO_FIX			0
#define UBLOX_NAV_STATUS_GPSFIX_DEAD_RECKONING		1
#define UBLOX_NAV_STATUS_GPSFIX_2D			2
#define UBLOX_NAV_STATUS_GPSFIX_3D			3
#define UBLOX_NAV_STATUS_GPSFIX_GPS_DEAD_RECKONING	4
#define UBLOX_NAV_STATUS_GPSFIX_TIME_ONLY		5

#define UBLOX_NAV_STATUS_FLAGS_GPSFIXOK			0
#define UBLOX_NAV_STATUS_FLAGS_DIFFSOLN			1
#define UBLOX_NAV_STATUS_FLAGS_WKNSET			2
#define UBLOX_NAV_STATUS_FLAGS_TOWSET			3

#define UBLOX_NAV_STATUS_FIXSTAT_DGPSISTAT		0
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING		6
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING_NONE		0
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING_VALID		1
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING_USED		2
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING_DR			3
#define UBLOX_NAV_STATUS_FIXSTAT_MAPMATCHING_MASK		3

#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE		0
#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE_ACQUISITION			0
#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE_TRACKING			1
#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE_POWER_OPTIMIZED_TRACKING	2
#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE_INACTIVE			3
#define UBLOX_NAV_STATUS_FLAGS2_PSMSTATE_MASK				3

#define UBLOX_NAV_SVINFO	0x30

struct ublox_nav_svinfo {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x30 */
	uint16_t	length;		/* 8 + 12 * numch */

	uint32_t	itow;		/* ms */

	uint8_t		numch;
	uint8_t		globalflags;
	uint16_t	reserved;
};

#define UBLOX_NAV_SVINFO_GLOBAL_FLAGS_CHIPGEN	0
#define UBLOX_NAV_SVINFO_GLOBAL_FLAGS_CHIPGEN_ANTARIS	0
#define UBLOX_NAV_SVINFO_GLOBAL_FLAGS_CHIPGEN_U_BLOX_5	1
#define UBLOX_NAV_SVINFO_GLOBAL_FLAGS_CHIPGEN_U_BLOX_6	2
#define UBLOX_NAV_SVINFO_GLOBAL_FLAGS_CHIPGEN_MASK	7

struct ublox_nav_svinfo_block {
	uint8_t		chn;
	uint8_t		svid;
	uint8_t		flags;
	uint8_t		quality;

	uint8_t		cno;		/* dbHz */
	int8_t		elev;		/* deg */
	int16_t		azim;		/* deg */

	int32_t		prres;		/* cm */
};

#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_SVUSED	0
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_DIFFCORR	1
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_ORBITAVAIL	2
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_ORBITEPH	3
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_UNHEALTHY	4
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_ORBITALM	5
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_ORBITAOP	6
#define UBLOX_NAV_SVINFO_BLOCK_FLAGS_SMOOTHED	7

#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND	0
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_IDLE			0
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_SEARCHING		1
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_ACQUIRED		2
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_UNUSABLE		3
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_CODE_LOCK		4
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_CARRIER_LOCKED_5	5
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_CARRIER_LOCKED_6	6
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_CARRIER_LOCKED_7	7
#define UBLOX_NAV_SVINFO_BLOCK_QUALITY_QUALITYIND_MASK			7

#define UBLOX_NAV_TIMEUTC	0x21

struct ublox_nav_timeutc {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x21 */
	uint16_t	length;		/* 20 */

	uint32_t	itow;		/* ms */
	uint32_t	tacc;		/* ns */
	int32_t		nano;		/* ns */

	uint16_t	year;
	uint8_t		month;
	uint8_t		day;

	uint8_t		hour;
	uint8_t		min;
	uint8_t		sec;
	uint8_t		valid;
};

#define UBLOX_NAV_TIMEUTC_VALID_VALIDTOW	0
#define UBLOX_NAV_TIMEUTC_VALID_VALIDWKN	1
#define UBLOX_NAV_TIMEUTC_VALID_VALIDUTC	2

#define UBLOX_NAV_VELNED	0x12

struct ublox_nav_velned {
	uint8_t		class;		/* 0x01 */
	uint8_t		message;	/* 0x12 */
	uint16_t	length;		/* 36 */

	uint32_t	itow;		/* ms */

	int32_t		veln;		/* cm/s */
	int32_t		vele;		/* cm/s */
	int32_t		veld;		/* cm/s */

	uint32_t	speed;		/* cm/s */
	uint32_t	gspeed;		/* cm/s */

	int32_t		heading;	/* deg */
	uint32_t	sacc;		/* cm/s */
	uint32_t	cacc;		/* deg */
};

#define UBLOX_CFG	0x06

#define UBLOX_CFG_NAV5	0x24

#define UBLOX_CFG_NAV5_MASK_DYN			0
#define UBLOX_CFG_NAV5_MASK_MINE1		1
#define UBLOX_CFG_NAV5_MASK_FIXMODE		2
#define UBLOX_CFG_NAV5_MASK_DRLIM		3
#define UBLOX_CFG_NAV5_MASK_POSMASK		4
#define UBLOX_CFG_NAV5_MASK_TIMEMASK		5
#define UBLOX_CFG_NAV5_MASK_STATICHOLDMASK	6
#define UBLOX_CFG_NAV5_MASK_DGPSMASK		7

#define UBLOX_CFG_NAV5_DYNMODEL_PORTABLE	0
#define UBLOX_CFG_NAV5_DYNMODEL_STATIONARY	2
#define UBLOX_CFG_NAV5_DYNMODEL_PEDESTRIAN	3
#define UBLOX_CFG_NAV5_DYNMODEL_AUTOMOTIVE	4
#define UBLOX_CFG_NAV5_DYNMODEL_SEA		5
#define UBLOX_CFG_NAV5_DYNMODEL_AIRBORNE_1G	6
#define UBLOX_CFG_NAV5_DYNMODEL_AIRBORNE_2G	7
#define UBLOX_CFG_NAV5_DYNMODEL_AIRBORNE_4G	8

#define UBLOX_CFG_NAV5_FIXMODE_2D		1
#define UBLOX_CFG_NAV5_FIXMODE_3D		2
#define UBLOX_CFG_NAV5_FIXMODE_AUTO		3

#endif /* _AO_GPS_UBLOX_H_ */
