/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_LOG_H_
#define _AO_LOG_H_

#include <ao_flight.h>

/*
 * ao_log.c
 */

/* We record flight numbers in the first record of
 * the log. Tasks may wait for this to be initialized
 * by sleeping on this variable.
 */
extern __xdata uint16_t ao_flight_number;

extern __pdata uint32_t ao_log_current_pos;
extern __pdata uint32_t ao_log_end_pos;
extern __pdata uint32_t ao_log_start_pos;
extern __xdata uint8_t	ao_log_running;
extern __pdata enum ao_flight_state ao_log_state;

/* required functions from the underlying log system */

#define AO_LOG_FORMAT_UNKNOWN		0	/* unknown; altosui will have to guess */
#define AO_LOG_FORMAT_FULL		1	/* 8 byte typed log records */
#define AO_LOG_FORMAT_TINY		2	/* two byte state/baro records */
#define AO_LOG_FORMAT_TELEMETRY		3	/* 32 byte ao_telemetry records */
#define AO_LOG_FORMAT_TELESCIENCE	4	/* 32 byte typed telescience records */
#define AO_LOG_FORMAT_MEGAMETRUM	5	/* 32 byte typed megametrum records */
#define AO_LOG_FORMAT_NONE		127	/* No log at all */

extern __code uint8_t ao_log_format;

/* Return the flight number from the given log slot, 0 if none */
uint16_t
ao_log_flight(uint8_t slot);

/* Flush the log */
void
ao_log_flush(void);

/* Logging thread main routine */
void
ao_log(void);

/* functions provided in ao_log.c */

/* Figure out the current flight number */
void
ao_log_scan(void) __reentrant;

/* Return the position of the start of the given log slot */
uint32_t
ao_log_pos(uint8_t slot);

/* Start logging to eeprom */
void
ao_log_start(void);

/* Stop logging */
void
ao_log_stop(void);

/* Initialize the logging system */
void
ao_log_init(void);

/* Write out the current flight number to the erase log */
void
ao_log_write_erase(uint8_t pos);

/* Returns true if there are any logs stored in eeprom */
uint8_t
ao_log_present(void);

/* Returns true if there is no more storage space available */
uint8_t
ao_log_full(void);

/*
 * ao_log_big.c
 */

/*
 * The data log is recorded in the eeprom as a sequence
 * of data packets.
 *
 * Each packet starts with a 4-byte header that has the
 * packet type, the packet checksum and the tick count. Then
 * they all contain 2 16 bit values which hold packet-specific
 * data.
 *
 * For each flight, the first packet
 * is FLIGHT packet, indicating the serial number of the
 * device and a unique number marking the number of flights
 * recorded by this device.
 *
 * During flight, data from the accelerometer and barometer
 * are recorded in SENSOR packets, using the raw 16-bit values
 * read from the A/D converter.
 *
 * Also during flight, but at a lower rate, the deployment
 * sensors are recorded in DEPLOY packets. The goal here is to
 * detect failure in the deployment circuits.
 *
 * STATE packets hold state transitions as the flight computer
 * transitions through different stages of the flight.
 */
#define AO_LOG_FLIGHT		'F'
#define AO_LOG_SENSOR		'A'
#define AO_LOG_TEMP_VOLT	'T'
#define AO_LOG_DEPLOY		'D'
#define AO_LOG_STATE		'S'
#define AO_LOG_GPS_TIME		'G'
#define AO_LOG_GPS_LAT		'N'
#define AO_LOG_GPS_LON		'W'
#define AO_LOG_GPS_ALT		'H'
#define AO_LOG_GPS_SAT		'V'
#define AO_LOG_GPS_DATE		'Y'

#define AO_LOG_POS_NONE		(~0UL)

struct ao_log_record {
	char			type;
	uint8_t			csum;
	uint16_t		tick;
	union {
		struct {
			int16_t		ground_accel;
			uint16_t	flight;
		} flight;
		struct {
			int16_t		accel;
			int16_t		pres;
		} sensor;
		struct {
			int16_t		temp;
			int16_t		v_batt;
		} temp_volt;
		struct {
			int16_t		drogue;
			int16_t		main;
		} deploy;
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		struct {
			uint8_t		hour;
			uint8_t		minute;
			uint8_t		second;
			uint8_t		flags;
		} gps_time;
		int32_t		gps_latitude;
		int32_t		gps_longitude;
		struct {
			int16_t		altitude;
			uint16_t	unused;
		} gps_altitude;
		struct {
			uint16_t	svid;
			uint8_t		unused;
			uint8_t		c_n;
		} gps_sat;
		struct {
			uint8_t		year;
			uint8_t		month;
			uint8_t		day;
			uint8_t		extra;
		} gps_date;
		struct {
			uint16_t	d0;
			uint16_t	d1;
		} anon;
	} u;
};

struct ao_log_mega {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		struct {
			uint16_t	flight;		/* 4 */
			int16_t		ground_accel;	/* 6 */
			uint32_t	ground_pres;	/* 8 */
			uint32_t	ground_temp;	/* 12 */
		} flight;				/* 16 */
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		struct {
			uint32_t	pres;		/* 4 */
			uint32_t	temp;		/* 8 */
			int16_t		accel_x;	/* 12 */
			int16_t		accel_y;	/* 14 */
			int16_t		accel_z;	/* 16 */
			int16_t		gyro_x;		/* 18 */
			int16_t		gyro_y;		/* 20 */
			int16_t		gyro_z;		/* 22 */
			int16_t		mag_x;		/* 24 */
			int16_t		mag_y;		/* 26 */
			int16_t		mag_z;		/* 28 */
			int16_t		accel;		/* 30 */
		} sensor;	/* 32 */
		struct {
			int16_t		v_batt;		/* 4 */
			int16_t		v_pbatt;	/* 6 */
			int16_t		n_sense;	/* 8 */
			int16_t		sense[10];	/* 10 */
		} volt;					/* 30 */
		struct {
			int32_t		latitude;	/* 4 */
			int32_t		longitude;	/* 8 */
			int16_t		altitude;	/* 12 */
			uint8_t		hour;		/* 14 */
			uint8_t		minute;		/* 15 */
			uint8_t		second;		/* 16 */
			uint8_t		flags;		/* 17 */
			uint8_t		year;		/* 18 */
			uint8_t		month;		/* 19 */
			uint8_t		day;		/* 20 */
			uint8_t		pad;		/* 21 */
		} gps;	/* 22 */
		struct {
			uint16_t	channels;	/* 4 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[12];			/* 6 */
		} gps_sat;				/* 30 */
	} u;
};

/* Write a record to the eeprom log */
uint8_t
ao_log_data(__xdata struct ao_log_record *log) __reentrant;

uint8_t
ao_log_mega(__xdata struct ao_log_mega *log) __reentrant;

#endif /* _AO_LOG_H_ */
