/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_4;

import java.text.*;

/*
 * Telemetry data contents
 */


/*
 * The packet format is a simple hex dump of the raw telemetry frame.
 * It starts with 'TELEM', then contains hex digits with a checksum as the last
 * byte on the line.
 *
 * Version 4 is a replacement with consistent syntax. Each telemetry line
 * contains a sequence of space-separated names and values, the values are
 * either integers or strings. The names are all unique. All values are
 * optional
 *
 * VERSION 4 c KD7SQG n 236 f 18 r -25 s pad t 513 r_a 15756 r_b 26444 r_t 20944
 *   r_v 26640 r_d 512 r_m 208 c_a 15775 c_b 26439 c_p 15749 c_m 16281 a_a 15764
 *   a_s 0 a_b 26439 g_s u g_n 0 s_n 0
 *
 * VERSION 4 c KD7SQG n 19 f 0 r -23 s pad t 513 r_b 26372 r_t 21292 r_v 26788
 *   r_d 136 r_m 140 c_b 26370 k_h 0 k_s 0 k_a 0
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
 *
 * Version 3 is Version 2 with fixed RSSI numbers -- the radio reports
 * in 1/2dB increments while this protocol provides only integers. So,
 * the syntax didn't change just the interpretation of the RSSI
 * values.
 *
 * Version 2 of the telemetry data stream is a bit of a mess, with no
 * consistent formatting. In particular, the GPS data is formatted for
 * viewing instead of parsing.  However, the key feature is that every
 * telemetry line contains all of the information necessary to
 * describe the current rocket state, including the calibration values
 * for accelerometer and barometer.
 *
 * GPS unlocked:
 *
 * VERSION 2 CALL KB0G SERIAL  51 FLIGHT     2 RSSI  -68 STATUS ff STATE     pad  1001 \
 *    a: 16032 p: 21232 t: 20284 v: 25160 d:   204 m:   204 fa: 16038 ga: 16032 fv:       0 \
 *    fp: 21232 gp: 21230 a+: 16049 a-: 16304 GPS  0 sat unlocked SAT 1   15  30
 *
 * GPS locked:
 *
 * VERSION 2 CALL KB0G SERIAL  51 FLIGHT     2 RSSI  -71 STATUS ff STATE     pad  2504 \
 *     a: 16028 p: 21220 t: 20360 v: 25004 d:   208 m:   200 fa: 16031 ga: 16032 fv:     330 \
 *     fp: 21231 gp: 21230 a+: 16049 a-: 16304 \
 *     GPS  9 sat 2010-02-13 17:16:51 35°20.0803'N 106°45.2235'W  1790m  \
 *     0.00m/s(H) 0°     0.00m/s(V) 1.0(hdop)     0(herr)     0(verr) \
 *     SAT 10   29  30  24  28   5  25  21  20  15  33   1  23  30  24  18  26  10  29   2  26
 *
 */

public class AltosTelemetryLegacy extends AltosTelemetry {
	/*
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

	final static String AO_TELEM_VERSION	= "VERSION";
	final static String AO_TELEM_CALL	= "c";
	final static String AO_TELEM_SERIAL	= "n";
	final static String AO_TELEM_FLIGHT	= "f";
	final static String AO_TELEM_RSSI	= "r";
	final static String AO_TELEM_STATE	= "s";
	final static String AO_TELEM_TICK	= "t";

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

	final static String AO_TELEM_RAW_ACCEL	= "r_a";
	final static String AO_TELEM_RAW_BARO	= "r_b";
	final static String AO_TELEM_RAW_THERMO	= "r_t";
	final static String AO_TELEM_RAW_BATT	= "r_v";
	final static String AO_TELEM_RAW_DROGUE	= "r_d";
	final static String AO_TELEM_RAW_MAIN	= "r_m";

	/*
	 * Sensor calibration values
	 *
	 *	Name		Value
	 *	c_a		Ground accelerometer reading (integer)
	 *	c_b		Ground barometer reading (integer)
	 *	c_p		Accelerometer reading for +1g
	 *	c_m		Accelerometer reading for -1g
	 */

	final static String AO_TELEM_CAL_ACCEL_GROUND	= "c_a";
	final static String AO_TELEM_CAL_BARO_GROUND	= "c_b";
	final static String AO_TELEM_CAL_ACCEL_PLUS	= "c_p";
	final static String AO_TELEM_CAL_ACCEL_MINUS	= "c_m";

	/*
	 * Kalman state values
	 *
	 *	Name		Value
	 *	k_h		Height above pad (integer, meters)
	 *	k_s		Vertical speeed (integer, m/s * 16)
	 *	k_a		Vertical acceleration (integer, m/s² * 16)
	 */

	final static String AO_TELEM_KALMAN_HEIGHT	= "k_h";
	final static String AO_TELEM_KALMAN_SPEED	= "k_s";
	final static String AO_TELEM_KALMAN_ACCEL	= "k_a";

	/*
	 * Ad-hoc flight values
	 *
	 *	Name		Value
	 *	a_a		Acceleration (integer, sensor units)
	 *	a_s		Speed (integer, integrated acceleration value)
	 *	a_b		Barometer reading (integer, sensor units)
	 */

	final static String AO_TELEM_ADHOC_ACCEL	= "a_a";
	final static String AO_TELEM_ADHOC_SPEED	= "a_s";
	final static String AO_TELEM_ADHOC_BARO		= "a_b";

	/*
	 * GPS values
	 *
	 *	Name		Value
	 *	g_s		GPS state (string):
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
	 *	g_s		GPS horizontal speed (integer, cm/sec)
	 *	g_c		GPS course (integer, 0-359)
	 *	g_hd		GPS hdop (integer * 10)
	 *	g_vd		GPS vdop (integer * 10)
	 *	g_he		GPS h error (integer)
	 *	g_ve		GPS v error (integer)
	 */

	final static String AO_TELEM_GPS_STATE	 		= "g";
	final static String AO_TELEM_GPS_STATE_LOCKED		= "l";
	final static String AO_TELEM_GPS_STATE_UNLOCKED		= "u";
	final static String AO_TELEM_GPS_STATE_ERROR		= "e";
	final static String AO_TELEM_GPS_NUM_SAT		= "g_n";
	final static String AO_TELEM_GPS_LATITUDE		= "g_ns";
	final static String AO_TELEM_GPS_LONGITUDE		= "g_ew";
	final static String AO_TELEM_GPS_ALTITUDE		= "g_a";
	final static String AO_TELEM_GPS_YEAR			= "g_Y";
	final static String AO_TELEM_GPS_MONTH			= "g_M";
	final static String AO_TELEM_GPS_DAY			= "g_D";
	final static String AO_TELEM_GPS_HOUR			= "g_h";
	final static String AO_TELEM_GPS_MINUTE			= "g_m";
	final static String AO_TELEM_GPS_SECOND			= "g_s";
	final static String AO_TELEM_GPS_VERTICAL_SPEED		= "g_v";
	final static String AO_TELEM_GPS_HORIZONTAL_SPEED	= "g_g";
	final static String AO_TELEM_GPS_COURSE			= "g_c";
	final static String AO_TELEM_GPS_HDOP			= "g_hd";
	final static String AO_TELEM_GPS_VDOP			= "g_vd";
	final static String AO_TELEM_GPS_HERROR			= "g_he";
	final static String AO_TELEM_GPS_VERROR			= "g_ve";

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

	final static String AO_TELEM_SAT_NUM	= "s_n";
	final static String AO_TELEM_SAT_SVID	= "s_v";
	final static String AO_TELEM_SAT_C_N_0	= "s_c";

	public int	version;
	public String 	callsign;
	public int	flight;
	public int	state;

	public AltosGPS	gps;
	public int	gps_sequence;

	/* Telemetry sources have these values recorded from the flight computer */
	public double	kalman_height;
	public double	kalman_speed;
	public double	kalman_acceleration;

	/* Sensor values */
	public int	accel;
	public int	pres;
	public int	temp;
	public int	batt;
	public int	apogee;
	public int	main;

	public int      ground_accel;
	public int      ground_pres;
	public int      accel_plus_g;
	public int      accel_minus_g;

	public int	flight_accel;
	public int	flight_vel;
	public int	flight_pres;

	private void parse_v4(String[] words, int i) throws ParseException {
		AltosTelemetryMap	map = new AltosTelemetryMap(words, i);

		callsign = map.get_string(AO_TELEM_CALL, "N0CALL");
		serial = map.get_int(AO_TELEM_SERIAL, AltosLib.MISSING);
		flight = map.get_int(AO_TELEM_FLIGHT, AltosLib.MISSING);
		rssi = map.get_int(AO_TELEM_RSSI, AltosLib.MISSING);
		state = AltosLib.state(map.get_string(AO_TELEM_STATE, "invalid"));
		tick = map.get_int(AO_TELEM_TICK, 0);

		/* raw sensor values */
		accel = map.get_int(AO_TELEM_RAW_ACCEL, AltosLib.MISSING);
		pres = map.get_int(AO_TELEM_RAW_BARO, AltosLib.MISSING);
		temp = map.get_int(AO_TELEM_RAW_THERMO, AltosLib.MISSING);
		batt = map.get_int(AO_TELEM_RAW_BATT, AltosLib.MISSING);
		apogee = map.get_int(AO_TELEM_RAW_DROGUE, AltosLib.MISSING);
		main = map.get_int(AO_TELEM_RAW_MAIN, AltosLib.MISSING);

		/* sensor calibration information */
		ground_accel = map.get_int(AO_TELEM_CAL_ACCEL_GROUND, AltosLib.MISSING);
		ground_pres = map.get_int(AO_TELEM_CAL_BARO_GROUND, AltosLib.MISSING);
		accel_plus_g = map.get_int(AO_TELEM_CAL_ACCEL_PLUS, AltosLib.MISSING);
		accel_minus_g = map.get_int(AO_TELEM_CAL_ACCEL_MINUS, AltosLib.MISSING);

		/* flight computer values */
		kalman_acceleration = map.get_double(AO_TELEM_KALMAN_ACCEL, AltosLib.MISSING, 1/16.0);
		kalman_speed = map.get_double(AO_TELEM_KALMAN_SPEED, AltosLib.MISSING, 1/16.0);
		kalman_height = map.get_int(AO_TELEM_KALMAN_HEIGHT, AltosLib.MISSING);

		flight_accel = map.get_int(AO_TELEM_ADHOC_ACCEL, AltosLib.MISSING);
		flight_vel = map.get_int(AO_TELEM_ADHOC_SPEED, AltosLib.MISSING);
		flight_pres = map.get_int(AO_TELEM_ADHOC_BARO, AltosLib.MISSING);

		if (map.has(AO_TELEM_GPS_STATE))
			gps = new AltosGPS(map);
		else
			gps = null;
	}

	private void parse_legacy(String[] words, int i) throws ParseException {

		AltosParse.word (words[i++], "CALL");
		callsign = words[i++];

		AltosParse.word (words[i++], "SERIAL");
		serial = AltosParse.parse_int(words[i++]);

		if (version >= 2) {
			AltosParse.word (words[i++], "FLIGHT");
			flight = AltosParse.parse_int(words[i++]);
		} else
			flight = 0;

		AltosParse.word(words[i++], "RSSI");
		rssi = AltosParse.parse_int(words[i++]);

		/* Older telemetry data had mis-computed RSSI value */
		if (version <= 2)
			rssi = (rssi + 74) / 2 - 74;

		AltosParse.word(words[i++], "STATUS");
		status = AltosParse.parse_hex(words[i++]);

		AltosParse.word(words[i++], "STATE");
		state = AltosLib.state(words[i++]);

		tick = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "a:");
		accel = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "p:");
		pres = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "t:");
		temp = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "v:");
		batt = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "d:");
		apogee = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "m:");
		main = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "fa:");
		flight_accel = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "ga:");
		ground_accel = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "fv:");
		flight_vel = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "fp:");
		flight_pres = AltosParse.parse_int(words[i++]);

		/* Old TeleDongle code with kalman-reporting TeleMetrum code */
		if ((flight_vel & 0xffff0000) == 0x80000000) {
			kalman_speed = ((short) flight_vel) / 16.0;
			kalman_acceleration = flight_accel / 16.0;
			kalman_height = flight_pres;
			flight_vel = AltosLib.MISSING;
			flight_pres = AltosLib.MISSING;
			flight_accel = AltosLib.MISSING;
		} else {
			kalman_speed = AltosLib.MISSING;
			kalman_acceleration = AltosLib.MISSING;
			kalman_height = AltosLib.MISSING;
		}

		AltosParse.word(words[i++], "gp:");
		ground_pres = AltosParse.parse_int(words[i++]);

		if (version >= 1) {
			AltosParse.word(words[i++], "a+:");
			accel_plus_g = AltosParse.parse_int(words[i++]);

			AltosParse.word(words[i++], "a-:");
			accel_minus_g = AltosParse.parse_int(words[i++]);
		} else {
			accel_plus_g = ground_accel;
			accel_minus_g = ground_accel + 530;
		}

		gps = new AltosGPS(words, i, version);
		gps_sequence++;
	}

	public AltosTelemetryLegacy(String line) throws ParseException, AltosCRCException {
		String[] words = line.split("\\s+");
		int	i = 0;

		if (words[i].equals("CRC") && words[i+1].equals("INVALID")) {
			i += 2;
			AltosParse.word(words[i++], "RSSI");
			rssi = AltosParse.parse_int(words[i++]);
			throw new AltosCRCException(rssi);
		}
		if (words[i].equals("CALL")) {
			version = 0;
		} else {
			AltosParse.word (words[i++], "VERSION");
			version = AltosParse.parse_int(words[i++]);
		}

		if (version < 4)
			parse_legacy(words, i);
		else
			parse_v4(words, i);
	}

	/*
	 * Given a hex dump of a legacy telemetry line, construct an AltosRecordTM from that
	 */

	int[]	bytes;
	int	adjust;

	/*
	private int int8(int i) {
		return AltosLib.int8(bytes, i + 1 + adjust);
	}
	*/
	private int uint8(int i) {
		return AltosLib.uint8(bytes, i + 1 + adjust);
	}
	private int int16(int i) {
		return AltosLib.int16(bytes, i + 1 + adjust);
	}
	private int uint16(int i) {
		return AltosLib.uint16(bytes, i + 1 + adjust);
	}
	private int uint32(int i) {
		return AltosLib.uint32(bytes, i + 1 + adjust);
	}
	private String string(int i, int l) {
		return AltosLib.string(bytes, i + 1 + adjust, l);
	}

	static final int AO_GPS_NUM_SAT_MASK	= (0xf << 0);
	static final int AO_GPS_NUM_SAT_SHIFT	= (0);

	static final int AO_GPS_VALID		= (1 << 4);
	static final int AO_GPS_RUNNING		= (1 << 5);
	static final int AO_GPS_DATE_VALID	= (1 << 6);
	static final int AO_GPS_COURSE_VALID	= (1 << 7);

	public AltosTelemetryLegacy(int[] in_bytes) {
		bytes = in_bytes;
		version = 4;
		adjust = 0;

		if (bytes.length == AltosLib.ao_telemetry_0_8_len + 4) {
			serial = uint8(0);
			adjust = -1;
		} else
			serial = uint16(0);

		callsign = string(62, 8);
		flight = uint16(2);
		state = uint8(4);
		tick = uint16(21);
		accel = int16(23);
		pres = int16(25);
		temp = int16(27);
		batt = int16(29);
		apogee = int16(31);
		main = int16(33);

		ground_accel = int16(7);
		ground_pres = int16(15);
		accel_plus_g = int16(17);
		accel_minus_g = int16(19);

		if (uint16(11) == 0x8000) {
			kalman_acceleration = int16(5);
			kalman_speed = int16(9);
			kalman_height = int16(13);
			flight_accel = AltosLib.MISSING;
			flight_vel = AltosLib.MISSING;
			flight_pres = AltosLib.MISSING;
		} else {
			flight_accel = int16(5);
			flight_vel = uint32(9);
			flight_pres = int16(13);
			kalman_acceleration = AltosLib.MISSING;
			kalman_speed = AltosLib.MISSING;
			kalman_height = AltosLib.MISSING;
		}

		gps = null;

		int gps_flags = uint8(41);

		if ((gps_flags & (AO_GPS_VALID|AO_GPS_RUNNING)) != 0) {
			gps = new AltosGPS();
			gps_sequence++;

			gps.nsat = (gps_flags & AO_GPS_NUM_SAT_MASK);
			gps.locked = (gps_flags & AO_GPS_VALID) != 0;
			gps.connected = true;
			gps.lat = uint32(42) / 1.0e7;
			gps.lon = uint32(46) / 1.0e7;
			gps.alt = int16(50);
			gps.ground_speed = uint16(52) / 100.0;
			gps.course = uint8(54) * 2;
			gps.hdop = uint8(55) / 5.0;
			gps.h_error = uint16(58);
			gps.v_error = uint16(60);

			int	n_tracking_reported = uint8(70);
			if (n_tracking_reported > 12)
				n_tracking_reported = 12;
			int	n_tracking_actual = 0;
			for (int i = 0; i < n_tracking_reported; i++) {
				if (uint8(71 + i*2) != 0)
					n_tracking_actual++;
			}
			if (n_tracking_actual > 0) {
				gps.cc_gps_sat = new AltosGPSSat[n_tracking_actual];

				n_tracking_actual = 0;
				for (int i = 0; i < n_tracking_reported; i++) {
					int	svid = uint8(71 + i*2);
					int	c_n0 = uint8(72 + i*2);
					if (svid != 0)
						gps.cc_gps_sat[n_tracking_actual++] = new AltosGPSSat(svid, c_n0);
				}
			}
		}
	}

	public void update_state(AltosState state) {
		state.set_tick(tick);
		state.set_state(this.state);
		state.set_flight(flight);
		state.set_serial(serial);
		state.set_rssi(rssi, status);

		state.set_pressure(AltosConvert.barometer_to_pressure(pres));
		state.set_accel_g(accel_plus_g, accel_minus_g);
		state.set_accel(accel);
		if (kalman_height != AltosLib.MISSING)
			state.set_kalman(kalman_height, kalman_speed, kalman_acceleration);
		state.set_temperature(AltosConvert.thermometer_to_temperature(temp));
		state.set_battery_voltage(AltosConvert.cc_battery_to_voltage(batt));
		state.set_apogee_voltage(AltosConvert.cc_ignitor_to_voltage(apogee));
		state.set_main_voltage(AltosConvert.cc_ignitor_to_voltage(main));
		if (gps != null)
			state.set_gps(gps, gps_sequence);
	}
}
