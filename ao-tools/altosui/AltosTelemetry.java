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

package altosui;

import java.lang.*;
import java.text.*;
import java.util.HashMap;
import altosui.AltosConvert;
import altosui.AltosGPS;

/*
 * Telemetry data contents
 */


/*
 * The telemetry data stream is a bit of a mess at present, with no consistent
 * formatting. In particular, the GPS data is formatted for viewing instead of parsing.
 * However, the key feature is that every telemetry line contains all of the information
 * necessary to describe the current rocket state, including the calibration values
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
 */

public class AltosTelemetry {
	int	version;
	String 	callsign;
	int	serial;
	int	flight;
	int	rssi;
	int	status;
	String	state;
	int	tick;
	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;
	int	flight_accel;
	int	ground_accel;
	int	flight_vel;
	int	flight_pres;
	int	ground_pres;
	int	accel_plus_g;
	int	accel_minus_g;
	AltosGPS	gps;

	public static final int ao_flight_startup = 0;
	public static final int ao_flight_idle = 1;
	public static final int ao_flight_pad = 2;
	public static final int ao_flight_boost = 3;
	public static final int ao_flight_fast = 4;
	public static final int ao_flight_coast = 5;
	public static final int ao_flight_drogue = 6;
	public static final int ao_flight_main = 7;
	public static final int ao_flight_landed = 8;
	public static final int ao_flight_invalid = 9;

	static HashMap<String,Integer>	states = new HashMap<String,Integer>();
	{
		states.put("startup", ao_flight_startup);
		states.put("idle", ao_flight_idle);
		states.put("pad", ao_flight_pad);
		states.put("boost", ao_flight_boost);
		states.put("fast", ao_flight_fast);
		states.put("coast", ao_flight_coast);
		states.put("drogue", ao_flight_drogue);
		states.put("main", ao_flight_main);
		states.put("landed", ao_flight_landed);
		states.put("invalid", ao_flight_invalid);
	}

	public int state() {
		if (states.containsKey(state))
			return states.get(state);
		return ao_flight_invalid;
	}

	public double altitude() {
		return AltosConvert.cc_pressure_to_altitude(AltosConvert.cc_barometer_to_pressure(pres));
	}

	public double pad_altitude() {
		return AltosConvert.cc_pressure_to_altitude(AltosConvert.cc_barometer_to_pressure(ground_pres));
	}

	public AltosTelemetry(String line) throws ParseException {
		String[] words = line.split("\\s+");

		int	i = 0;

		AltosParse.word (words[i++], "VERSION");
		version = AltosParse.parse_int(words[i++]);

		AltosParse.word (words[i++], "CALL");
		callsign = words[i++];

		AltosParse.word (words[i++], "SERIAL");
		serial = AltosParse.parse_int(words[i++]);

		AltosParse.word (words[i++], "FLIGHT");
		flight = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "RSSI");
		rssi = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "STATUS");
		status = AltosParse.parse_hex(words[i++]);

		AltosParse.word(words[i++], "STATE");
		state = words[i++];

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
		drogue = AltosParse.parse_int(words[i++]);

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

		AltosParse.word(words[i++], "gp:");
		ground_pres = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "a+:");
		accel_plus_g = AltosParse.parse_int(words[i++]);

		AltosParse.word(words[i++], "a-:");
		accel_minus_g = AltosParse.parse_int(words[i++]);

	}
}
