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

/*
 * Telemetry data contents
 */

class AltosGPSTime {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	int parse_int(String v) throws ParseException {
		try {
			return Integer.parseInt(v);
		} catch (NumberFormatException e) {
			throw new ParseException(v, 0);
		}
	}

	public AltosGPSTime(String date, String time) throws ParseException {
		String[] ymd = date.split("-");
		if (ymd.length != 3) {
			System.out.println("Error parsing GPS date " + date + " got " + ymd.length);
			throw new ParseException(date, 0);
		}
		year = parse_int(ymd[0]);
		month = parse_int(ymd[1]);
		day = parse_int(ymd[2]);

		String[] hms = time.split(":");
		if (hms.length != 3) {
			System.out.println("Error parsing GPS time " + time + " got " + hms.length);
			throw new ParseException(time, 0);
		}
		hour = parse_int(hms[0]);
		minute = parse_int(hms[1]);
		second = parse_int(hms[2]);
	}

	public AltosGPSTime() {
		year = month = day = 0;
		hour = minute = second = 0;
	}
};

class AltosGPS {
	int	nsat;
	int	gps_locked;
	int	gps_connected;
	AltosGPSTime gps_time;
	double	lat;		/* degrees (+N -S) */
	double	lon;		/* degrees (+E -W) */
	int	alt;		/* m */

	int	gps_extended;	/* has extra data */
	double	ground_speed;	/* m/s */
	int	course;		/* degrees */
	double	climb_rate;	/* m/s */
	double	hdop;		/* unitless? */
	int	h_error;	/* m */
	int	v_error;	/* m */

}

class AltosGPSSat {
	int	svid;
	int	c_n0;
}

class AltosGPSTracking {
	int			channels;
	AltosGPSSat[]		cc_gps_sat;
}

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
	AltosGPSTracking	gps_tracking;

	int parse_int(String v) throws ParseException {
		try {
			return Integer.parseInt(v);
		} catch (NumberFormatException e) {
			System.out.println("error parsing int " + v);
			throw new ParseException(v, 0);
		}
	}

	int parse_hex(String v) throws ParseException {
		try {
			return Integer.parseInt(v, 16);
		} catch (NumberFormatException e) {
			System.out.println("error parsing hex " + v);
			throw new ParseException(v, 0);
		}
	}

	double parse_double(String v) throws ParseException {
		try {
			return Double.parseDouble(v);
		} catch (NumberFormatException e) {
			System.out.println("error parsing double " + v);
			throw new ParseException(v, 0);
		}
	}

	double parse_coord(String coord) throws ParseException {
		String[]	dsf = coord.split("\\D+");

		if (dsf.length != 3) {
			System.out.println("error parsing coord " + coord);
			throw new ParseException(coord, 0);
		}
		int deg = parse_int(dsf[0]);
		int min = parse_int(dsf[1]);
		int frac = parse_int(dsf[2]);

		double r = deg + (min + frac / 10000.0) / 60.0;
		if (coord.endsWith("S") || coord.endsWith("W"))
			r = -r;
		return r;
	}

	String strip_suffix(String v, String suffix) {
		if (v.endsWith(suffix))
			return v.substring(0, v.length() - suffix.length());
		return v;
	}

	void word(String v, String m) throws ParseException {
		if (!v.equals(m)) {
			System.out.println("error matching '" + v + "' '" + m + "'");
			throw new ParseException(v, 0);
		}
	}

	public AltosTelemetry(String line) throws ParseException {
		String[] words = line.split("\\s+");

		int	i = 0;

		word (words[i++], "VERSION");
		version = parse_int(words[i++]);

		word (words[i++], "CALL");
		callsign = words[i++];

		word (words[i++], "SERIAL");
		serial = parse_int(words[i++]);

		word (words[i++], "FLIGHT");
		flight = parse_int(words[i++]);

		word(words[i++], "RSSI");
		rssi = parse_int(words[i++]);

		word(words[i++], "STATUS");
		status = parse_hex(words[i++]);

		word(words[i++], "STATE");
		state = words[i++];

		tick = parse_int(words[i++]);

		word(words[i++], "a:");
		accel = parse_int(words[i++]);

		word(words[i++], "p:");
		pres = parse_int(words[i++]);

		word(words[i++], "t:");
		temp = parse_int(words[i++]);

		word(words[i++], "v:");
		batt = parse_int(words[i++]);

		word(words[i++], "d:");
		drogue = parse_int(words[i++]);

		word(words[i++], "m:");
		main = parse_int(words[i++]);

		word(words[i++], "fa:");
		flight_accel = parse_int(words[i++]);

		word(words[i++], "ga:");
		ground_accel = parse_int(words[i++]);

		word(words[i++], "fv:");
		flight_vel = parse_int(words[i++]);

		word(words[i++], "fp:");
		flight_pres = parse_int(words[i++]);

		word(words[i++], "gp:");
		ground_pres = parse_int(words[i++]);

		word(words[i++], "a+:");
		accel_plus_g = parse_int(words[i++]);

		word(words[i++], "a-:");
		accel_minus_g = parse_int(words[i++]);

		word(words[i++], "GPS");
		gps = new AltosGPS();
		gps.nsat = parse_int(words[i++]);
		word(words[i++], "sat");

		gps.gps_connected = 0;
		gps.gps_locked = 0;
		gps.lat = gps.lon = 0;
		gps.alt = 0;
		if ((words[i]).equals("unlocked")) {
			gps.gps_connected = 1;
			gps.gps_time = new AltosGPSTime();
			i++;
		} else if (words.length >= 40) {
			gps.gps_locked = 1;
			gps.gps_connected = 1;

			gps.gps_time = new AltosGPSTime(words[i], words[i+1]); i += 2;
			gps.lat = parse_coord(words[i++]);
			gps.lon = parse_coord(words[i++]);
			gps.alt = parse_int(strip_suffix(words[i++], "m"));
			gps.ground_speed = parse_double(strip_suffix(words[i++], "m/s(H)"));
			gps.course = parse_int(strip_suffix(words[i++], "Â°"));
			gps.climb_rate = parse_double(strip_suffix(words[i++], "m/s(V)"));
			gps.hdop = parse_double(strip_suffix(words[i++], "(hdop)"));
			gps.h_error = parse_int(strip_suffix(words[i++], "(herr)"));
			gps.v_error = parse_int(strip_suffix(words[i++], "(verr)"));
		} else {
			gps.gps_time = new AltosGPSTime();
		}
		word(words[i++], "SAT");
		gps_tracking = new AltosGPSTracking();
		gps_tracking.channels = parse_int(words[i++]);
		gps_tracking.cc_gps_sat = new AltosGPSSat[gps_tracking.channels];
		for (int chan = 0; chan < gps_tracking.channels; chan++) {
			gps_tracking.cc_gps_sat[chan] = new AltosGPSSat();
			gps_tracking.cc_gps_sat[chan].svid = parse_int(words[i++]);
			gps_tracking.cc_gps_sat[chan].c_n0 = parse_int(words[i++]);
		}
	}
}
