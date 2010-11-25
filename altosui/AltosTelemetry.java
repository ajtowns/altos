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

public class AltosTelemetry extends AltosRecord {
	public AltosTelemetry(String line) throws ParseException, AltosCRCException {
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
		state = Altos.state(words[i++]);

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
	}
}
