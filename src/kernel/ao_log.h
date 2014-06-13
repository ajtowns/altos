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
extern __xdata uint8_t	ao_log_mutex;
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
#define AO_LOG_FORMAT_TELEMEGA		5	/* 32 byte typed telemega records */
#define AO_LOG_FORMAT_EASYMINI		6	/* 16-byte MS5607 baro only, 3.0V supply */
#define AO_LOG_FORMAT_TELEMETRUM	7	/* 16-byte typed telemetrum records */
#define AO_LOG_FORMAT_TELEMINI		8	/* 16-byte MS5607 baro only, 3.3V supply */
#define AO_LOG_FORMAT_TELEGPS		9	/* 32 byte telegps records */
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
#define AO_LOG_GPS_POS		'P'

#define AO_LOG_POS_NONE		(~0UL)

struct ao_log_record {
	char			type;				/* 0 */
	uint8_t			csum;				/* 1 */
	uint16_t		tick;				/* 2 */
	union {
		struct {
			int16_t		ground_accel;		/* 4 */
			uint16_t	flight;			/* 6 */
		} flight;
		struct {
			int16_t		accel;			/* 4 */
			int16_t		pres;			/* 6 */
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
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		ground_accel;		/* 6 */
			uint32_t	ground_pres;		/* 8 */
			int16_t		ground_accel_along;	/* 12 */
			int16_t		ground_accel_across;	/* 14 */
			int16_t		ground_accel_through;	/* 16 */
			int16_t		ground_roll;		/* 18 */
			int16_t		ground_pitch;		/* 20 */
			int16_t		ground_yaw;		/* 22 */
		} flight;					/* 24 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		/* AO_LOG_SENSOR */
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
		/* AO_LOG_TEMP_VOLT */
		struct {
			int16_t		v_batt;		/* 4 */
			int16_t		v_pbatt;	/* 6 */
			int16_t		n_sense;	/* 8 */
			int16_t		sense[10];	/* 10 */
			uint16_t	pyro;		/* 30 */
		} volt;					/* 32 */
		/* AO_LOG_GPS_TIME */
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
			uint8_t		course;		/* 21 */
			uint16_t	ground_speed;	/* 22 */
			int16_t		climb_rate;	/* 24 */
			uint8_t		pdop;		/* 26 */
			uint8_t		hdop;		/* 27 */
			uint8_t		vdop;		/* 28 */
			uint8_t		mode;		/* 29 */
		} gps;	/* 30 */
		/* AO_LOG_GPS_SAT */
		struct {
			uint16_t	channels;	/* 4 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[12];			/* 6 */
		} gps_sat;				/* 30 */
	} u;
};

struct ao_log_metrum {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
			int16_t		ground_accel;	/* 6 */
			uint32_t	ground_pres;	/* 8 */
			uint32_t	ground_temp;	/* 12 */
		} flight;	/* 16 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;	/* 8 */
		/* AO_LOG_SENSOR */
		struct {
			uint32_t	pres;		/* 4 */
			uint32_t	temp;		/* 8 */
			int16_t		accel;		/* 12 */
		} sensor;	/* 14 */
		/* AO_LOG_TEMP_VOLT */
		struct {
			int16_t		v_batt;		/* 4 */
			int16_t		sense_a;	/* 6 */
			int16_t		sense_m;	/* 8 */
		} volt;		/* 10 */
		/* AO_LOG_GPS_POS */
		struct {
			int32_t		latitude;	/* 4 */
			int32_t		longitude;	/* 8 */
			int16_t		altitude;	/* 12 */
		} gps;		/* 14 */
		/* AO_LOG_GPS_TIME */
		struct {
			uint8_t		hour;		/* 4 */
			uint8_t		minute;		/* 5 */
			uint8_t		second;		/* 6 */
			uint8_t		flags;		/* 7 */
			uint8_t		year;		/* 8 */
			uint8_t		month;		/* 9 */
			uint8_t		day;		/* 10 */
			uint8_t		pad;		/* 11 */
		} gps_time;	/* 12 */
		/* AO_LOG_GPS_SAT (up to three packets) */
		struct {
			uint8_t	channels;		/* 4 */
			uint8_t	more;			/* 5 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[4];			/* 6 */
		} gps_sat;				/* 14 */
		uint8_t		raw[12];		/* 4 */
	} u;	/* 16 */
};

struct ao_log_mini {
	char		type;				/* 0 */
	uint8_t		csum;				/* 1 */
	uint16_t	tick;				/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
			uint16_t	r6;
			uint32_t	ground_pres;	/* 8 */
		} flight;
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;
		/* AO_LOG_SENSOR */
		struct {
			uint8_t		pres[3];	/* 4 */
			uint8_t		temp[3];	/* 7 */
			int16_t		sense_a;	/* 10 */
			int16_t		sense_m;	/* 12 */
			int16_t		v_batt;		/* 14 */
		} sensor;				/* 16 */
	} u;						/* 16 */
};							/* 16 */

#define ao_log_pack24(dst,value) do {		\
		(dst)[0] = (value);		\
		(dst)[1] = (value) >> 8;	\
		(dst)[2] = (value) >> 16;	\
	} while (0)

struct ao_log_gps {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		start_altitude;		/* 6 */
			int32_t		start_latitude;		/* 8 */
			int32_t		start_longitude;	/* 12 */
		} flight;					/* 16 */
		/* AO_LOG_GPS_TIME */
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
			uint8_t		course;		/* 21 */
			uint16_t	ground_speed;	/* 22 */
			int16_t		climb_rate;	/* 24 */
			uint8_t		pdop;		/* 26 */
			uint8_t		hdop;		/* 27 */
			uint8_t		vdop;		/* 28 */
			uint8_t		mode;		/* 29 */
			uint8_t		state;		/* 30 */
		} gps;	/* 31 */
		/* AO_LOG_GPS_SAT */
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

uint8_t
ao_log_metrum(__xdata struct ao_log_metrum *log) __reentrant;

uint8_t
ao_log_mini(__xdata struct ao_log_mini *log) __reentrant;

uint8_t
ao_log_gps(__xdata struct ao_log_gps *log) __reentrant;

void
ao_log_flush(void);

void
ao_gps_report_metrum_init(void);

#endif /* _AO_LOG_H_ */
