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

#ifndef _AO_TELEM_H_
#define _AO_TELEM_H_

#define AO_TELEMETRY_VERSION		4

/*
 * Telemetry version 4 and higher format:
 *
 * General header fields
 *
 *	Name		Value
 *
 *	VERSION		Telemetry version number (4 or more). Must be first.
 * 	c		Callsign (string, no spaces allowed)
 *	n		Flight unit serial number (integer)
 * 	f		Flight number (integer)
 *	r		Packet RSSI value (integer)
 * 	s		Flight computer state (string, no spaces allowed)
 *	t		Flight computer clock (integer in centiseconds)
 */

#define AO_TELEM_VERSION	"VERSION"
#define AO_TELEM_CALL		"c"
#define AO_TELEM_SERIAL		"n"
#define AO_TELEM_FLIGHT		"f"
#define AO_TELEM_RSSI		"r"
#define AO_TELEM_STATE		"s"
#define AO_TELEM_TICK		"t"

/*
 * Raw sensor values
 *
 *	Name		Value
 *	r_a		Accelerometer reading (integer)
 *	r_b		Barometer reading (integer)
 *	r_t		Thermometer reading (integer)
 *	r_v		Battery reading (integer)
 *	r_d		Drogue continuity (integer)
 *	r_m		Main continuity (integer)
 */

#define AO_TELEM_RAW_ACCEL	"r_a"
#define AO_TELEM_RAW_BARO	"r_b"
#define AO_TELEM_RAW_THERMO	"r_t"
#define AO_TELEM_RAW_BATT	"r_v"
#define AO_TELEM_RAW_DROGUE	"r_d"
#define AO_TELEM_RAW_MAIN	"r_m"

/*
 * Sensor calibration values
 *
 *	Name		Value
 *	c_a		Ground accelerometer reading (integer)
 *	c_b		Ground barometer reading (integer)
 *	c_p		Accelerometer reading for +1g
 *	c_m		Accelerometer reading for -1g
 */

#define AO_TELEM_CAL_ACCEL_GROUND	"c_a"
#define AO_TELEM_CAL_BARO_GROUND	"c_b"
#define AO_TELEM_CAL_ACCEL_PLUS		"c_p"
#define AO_TELEM_CAL_ACCEL_MINUS	"c_m"

/*
 * Kalman state values
 *
 *	Name		Value
 *	k_h		Height above pad (integer, meters)
 *	k_s		Vertical speeed (integer, m/s * 16)
 *	k_a		Vertical acceleration (integer, m/s² * 16)
 */

#define AO_TELEM_KALMAN_HEIGHT		"k_h"
#define AO_TELEM_KALMAN_SPEED		"k_s"
#define AO_TELEM_KALMAN_ACCEL		"k_a"

/*
 * Ad-hoc flight values
 *
 *	Name		Value
 *	a_a		Acceleration (integer, sensor units)
 *	a_s		Speed (integer, integrated acceleration value)
 *	a_b		Barometer reading (integer, sensor units)
 */

#define AO_TELEM_ADHOC_ACCEL		"a_a"
#define AO_TELEM_ADHOC_SPEED		"a_s"
#define AO_TELEM_ADHOC_BARO		"a_b"

/*
 * GPS values
 *
 *	Name		Value
 *	g		GPS state (string):
 *				l	locked
 *				u	unlocked
 *				e	error (missing or broken)
 *	g_n		Number of sats used in solution
 *	g_ns		Latitude (degrees * 10e7)
 *	g_ew		Longitude (degrees * 10e7)
 *	g_a		Altitude (integer meters)
 *	g_Y		GPS year (integer)
 *	g_M		GPS month (integer - 1-12)
 *	g_D		GPS day (integer - 1-31)
 *	g_h		GPS hour (integer - 0-23)
 *	g_m		GPS minute (integer - 0-59)
 *	g_s		GPS second (integer - 0-59)
 *	g_v		GPS vertical speed (integer, cm/sec)
 *	g_g		GPS horizontal speed (integer, cm/sec)
 *	g_c		GPS course (integer, 0-359)
 *	g_hd		GPS hdop (integer * 10)
 *	g_vd		GPS vdop (integer * 10)
 *	g_he		GPS h error (integer)
 *	g_ve		GPS v error (integer)
 */

#define AO_TELEM_GPS_STATE		"g"
#define AO_TELEM_GPS_STATE_LOCKED	'l'
#define AO_TELEM_GPS_STATE_UNLOCKED	'u'
#define AO_TELEM_GPS_STATE_ERROR	'e'
#define AO_TELEM_GPS_NUM_SAT		"g_n"
#define AO_TELEM_GPS_LATITUDE		"g_ns"
#define AO_TELEM_GPS_LONGITUDE		"g_ew"
#define AO_TELEM_GPS_ALTITUDE		"g_a"
#define AO_TELEM_GPS_YEAR		"g_Y"
#define AO_TELEM_GPS_MONTH		"g_M"
#define AO_TELEM_GPS_DAY		"g_D"
#define AO_TELEM_GPS_HOUR		"g_h"
#define AO_TELEM_GPS_MINUTE		"g_m"
#define AO_TELEM_GPS_SECOND		"g_s"
#define AO_TELEM_GPS_VERTICAL_SPEED	"g_v"
#define AO_TELEM_GPS_HORIZONTAL_SPEED	"g_g"
#define AO_TELEM_GPS_COURSE		"g_c"
#define AO_TELEM_GPS_HDOP		"g_hd"
#define AO_TELEM_GPS_VDOP		"g_vd"
#define AO_TELEM_GPS_HERROR		"g_he"
#define AO_TELEM_GPS_VERROR		"g_ve"

/*
 * GPS satellite values
 *
 *	Name		Value
 *	s_n		Number of satellites reported (integer)
 *	s_v0		Space vehicle ID (integer) for report 0
 *	s_c0		C/N0 number (integer) for report 0
 *	s_v1		Space vehicle ID (integer) for report 1
 *	s_c1		C/N0 number (integer) for report 1
 *	...
 */

#define AO_TELEM_SAT_NUM		"s_n"
#define AO_TELEM_SAT_SVID		"s_v"
#define AO_TELEM_SAT_C_N_0		"s_c"

#endif /* _AO_TELEM_H_ */
