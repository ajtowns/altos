/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#ifndef _CC_TELEMETRY_H_
#define _CC_TELEMETRY_H_
/*
 * ao_telemetry.c
 */
#define AO_MAX_CALLSIGN			8
#define AO_MAX_VERSION			8
#define AO_MAX_TELEMETRY		128

struct ao_telemetry_generic {
	uint16_t	serial;		/* 0 */
	uint16_t	tick;		/* 2 */
	uint8_t		type;		/* 4 */
	uint8_t		payload[27];	/* 5 */
	uint8_t		rssi;		/* 32 */
	uint8_t		status;		/* 33 */
	/* 34 */
};

#define AO_TELEMETRY_SENSOR_TELEMETRUM	0x01
#define AO_TELEMETRY_SENSOR_TELEMINI	0x02
#define AO_TELEMETRY_SENSOR_TELENANO	0x03

struct ao_telemetry_sensor {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         state;          /*  5 flight state */
	int16_t		accel;		/*  6 accelerometer (TM only) */
	int16_t		pres;		/*  8 pressure sensor */
	int16_t		temp;		/* 10 temperature sensor */
	int16_t		v_batt;		/* 12 battery voltage */
	int16_t		sense_d;	/* 14 drogue continuity sense (TM/Tm) */
	int16_t		sense_m;	/* 16 main continuity sense (TM/Tm) */

	int16_t         acceleration;   /* 18 m/s² * 16 */
	int16_t         speed;          /* 20 m/s * 16 */
	int16_t         height;         /* 22 m */

	int16_t		ground_pres;	/* 24 average pres on pad */
	int16_t		ground_accel;	/* 26 average accel on pad */
	int16_t		accel_plus_g;	/* 28 accel calibration at +1g */
	int16_t		accel_minus_g;	/* 30 accel calibration at -1g */
	/* 32 */
};

#define AO_TELEMETRY_CONFIGURATION	0x04

struct ao_telemetry_configuration {
	uint16_t	serial;				/*  0 */
	uint16_t	tick;				/*  2 */
	uint8_t		type;				/*  4 */

	uint8_t         device;         		/*  5 device type */
	uint16_t        flight;				/*  6 flight number */
	uint8_t		config_major;			/*  8 Config major version */
	uint8_t		config_minor;			/*  9 Config minor version */
	uint16_t	apogee_delay;			/* 10 Apogee deploy delay in seconds */
	uint16_t	main_deploy;			/* 12 Main deploy alt in meters */
	uint16_t	flight_log_max;			/* 14 Maximum flight log size in kB */
	char		callsign[AO_MAX_CALLSIGN];	/* 16 Radio operator identity */
	char		version[AO_MAX_VERSION];	/* 24 Software version */
	/* 32 */
};

#define AO_TELEMETRY_LOCATION		0x05

#define AO_GPS_MODE_NOT_VALID		'N'
#define AO_GPS_MODE_AUTONOMOUS		'A'
#define AO_GPS_MODE_DIFFERENTIAL	'D'
#define AO_GPS_MODE_ESTIMATED		'E'
#define AO_GPS_MODE_MANUAL		'M'
#define AO_GPS_MODE_SIMULATED		'S'

struct ao_telemetry_location {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         flags;		/*  5 Number of sats and other flags */
	int16_t         altitude;	/*  6 GPS reported altitude (m) */
	int32_t         latitude;	/*  8 latitude (degrees * 10⁷) */
	int32_t         longitude;	/* 12 longitude (degrees * 10⁷) */
	uint8_t         year;		/* 16 (- 2000) */
	uint8_t         month;		/* 17 (1-12) */
	uint8_t         day;		/* 18 (1-31) */
	uint8_t         hour;		/* 19 (0-23) */
	uint8_t         minute;		/* 20 (0-59) */
	uint8_t         second;		/* 21 (0-59) */
	uint8_t         pdop;		/* 22 (m * 5) */
	uint8_t         hdop;		/* 23 (m * 5) */
	uint8_t         vdop;		/* 24 (m * 5) */
	uint8_t         mode;		/* 25 */
	uint16_t	ground_speed;	/* 26 cm/s */
	int16_t		climb_rate;	/* 28 cm/s */
	uint8_t		course;		/* 30 degrees / 2 */
	uint8_t		unused[1];	/* 31 */
	/* 32 */
};

#define AO_TELEMETRY_SATELLITE		0x06

struct ao_telemetry_satellite_info {
	uint8_t		svid;
	uint8_t		c_n_1;
};

struct ao_telemetry_satellite {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					channels;	/*  5 number of reported sats */

	struct ao_telemetry_satellite_info	sats[12];	/* 6 */
	uint8_t					unused[2];	/* 30 */
	/* 32 */
};

#define AO_TELEMETRY_COMPANION		0x07

#define AO_COMPANION_MAX_CHANNELS	12

struct ao_telemetry_companion {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					board_id;	/*  5 */

	uint8_t					update_period;	/*  6 */
	uint8_t					channels;	/*  7 */
	uint16_t				companion_data[AO_COMPANION_MAX_CHANNELS];	/*  8 */
	/* 32 */
};
	
#define AO_TELEMETRY_MEGA_SENSOR	0x08

struct ao_telemetry_mega_sensor {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t		orient;		/*  5 angle from vertical */
	int16_t		accel;		/*  6 Z axis */

	int32_t		pres;		/*  8 Pa * 10 */
	int16_t		temp;		/* 12 °C * 100 */

	int16_t		accel_x;	/* 14 */
	int16_t		accel_y;	/* 16 */
	int16_t		accel_z;	/* 18 */

	int16_t		gyro_x;		/* 20 */
	int16_t		gyro_y;		/* 22 */
	int16_t		gyro_z;		/* 24 */

	int16_t		mag_x;		/* 26 */
	int16_t		mag_y;		/* 28 */
	int16_t		mag_z;		/* 30 */
	/* 32 */
};
	
#define AO_TELEMETRY_MEGA_DATA		0x09

struct ao_telemetry_mega_data {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         state;          /*  5 flight state */

	int16_t		v_batt;		/*  6 battery voltage */
	int16_t		v_pyro;		/*  8 pyro battery voltage */
	int8_t		sense[6];	/* 10 continuity sense */

	int32_t		ground_pres;	/* 16 average pres on pad */
	int16_t		ground_accel;	/* 20 average accel on pad */
	int16_t		accel_plus_g;	/* 22 accel calibration at +1g */
	int16_t		accel_minus_g;	/* 24 accel calibration at -1g */

	int16_t         acceleration;   /* 26 m/s² * 16 */
	int16_t         speed;          /* 28 m/s * 16 */
	int16_t         height;         /* 30 m */
	/* 32 */
};

#define AO_TELEMETRY_METRUM_SENSOR	0x0A

struct ao_telemetry_metrum_sensor {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         state;          /*  5 flight state */
	int16_t		accel;		/*  6 Z axis */

	int32_t		pres;		/*  8 Pa * 10 */
	int16_t		temp;		/* 12 °C * 100 */

	int16_t         acceleration;   /* 14 m/s² * 16 */
	int16_t         speed;          /* 16 m/s * 16 */
	int16_t         height;         /* 18 m */

	int16_t		v_batt;		/* 20 battery voltage */
	int16_t		sense_a;	/* 22 apogee continuity sense */
	int16_t		sense_m;	/* 24 main continuity sense */

	uint8_t		pad[6];		/* 26 */
	/* 32 */
};
	
#define AO_TELEMETRY_METRUM_DATA	0x0B

struct ao_telemetry_metrum_data {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	int32_t		ground_pres;	/* 8 average pres on pad */
	int16_t		ground_accel;	/* 12 average accel on pad */
	int16_t		accel_plus_g;	/* 14 accel calibration at +1g */
	int16_t		accel_minus_g;	/* 16 accel calibration at -1g */

	uint8_t		pad[14];	/* 18 */
	/* 32 */
};

#define AO_TELEMETRY_MINI		0x10

struct ao_telemetry_mini {
	uint16_t	serial;		/*  0 */
	uint16_t	tick;		/*  2 */
	uint8_t		type;		/*  4 */

	uint8_t         state;          /*  5 flight state */
	int16_t		v_batt;		/*  6 battery voltage */
	int16_t		sense_a;	/*  8 apogee continuity */
	int16_t		sense_m;	/* 10 main continuity */

	int32_t		pres;		/* 12 Pa * 10 */
	int16_t		temp;		/* 16 °C * 100 */

	int16_t         acceleration;   /* 18 m/s² * 16 */
	int16_t         speed;          /* 20 m/s * 16 */
	int16_t         height;         /* 22 m */

	int32_t		ground_pres;	/* 24 average pres on pad */

	int32_t		pad28;		/* 28 */
	/* 32 */
};

/* #define AO_SEND_ALL_BARO */

#define AO_TELEMETRY_BARO		0x80

/*
 * This packet allows the full sampling rate baro
 * data to be captured over the RF link so that the
 * flight software can be tested using 'real' data.
 *
 * Along with this telemetry packet, the flight
 * code is modified to send full-rate telemetry all the time
 * and never send an RDF tone; this ensure that the full radio
 * link is available.
 */
struct ao_telemetry_baro {
	uint16_t				serial;		/*  0 */
	uint16_t				tick;		/*  2 */
	uint8_t					type;		/*  4 */
	uint8_t					samples;	/*  5 number samples */

	int16_t					baro[12];	/* 6 samples */
	/* 32 */
};

union ao_telemetry_all {
	struct ao_telemetry_generic		generic;
	struct ao_telemetry_sensor		sensor;
	struct ao_telemetry_configuration	configuration;
	struct ao_telemetry_location		location;
	struct ao_telemetry_satellite		satellite;
	struct ao_telemetry_companion		companion;
	struct ao_telemetry_mega_sensor		mega_sensor;
	struct ao_telemetry_mega_data		mega_data;
	struct ao_telemetry_metrum_sensor	metrum_sensor;
	struct ao_telemetry_metrum_data		metrum_data;
	struct ao_telemetry_mini		mini;
	struct ao_telemetry_baro		baro;
};

#define CC_TELEMETRY_HEADER	"TELEM"

/* "TELEM " 1 byte length 32 data bytes 1 rssi 1 status 1 checksum 1 null */

#define CC_TELEMETRY_BUFSIZE	(6 + (1 + 32 + 3) * 2 + 1)

int
cc_telemetry_parse(const char *input_line, union ao_telemetry_all *telemetry);

uint8_t
cc_telemetry_cksum(const union ao_telemetry_all *telemetry);

void
cc_telemetry_unparse(const union ao_telemetry_all *telemetry, char output_line[CC_TELEMETRY_BUFSIZE]);

#endif
