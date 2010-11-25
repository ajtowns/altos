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

public class AltosGPS {
	public class AltosGPSSat {
		int	svid;
		int	c_n0;
	}

	int	nsat;
	boolean	locked;
	boolean	connected;
	boolean date_valid;
	double	lat;		/* degrees (+N -S) */
	double	lon;		/* degrees (+E -W) */
	int	alt;		/* m */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;

	int	gps_extended;	/* has extra data */
	double	ground_speed;	/* m/s */
	int	course;		/* degrees */
	double	climb_rate;	/* m/s */
	double	hdop;		/* unitless? */
	int	h_error;	/* m */
	int	v_error;	/* m */

	AltosGPSSat[] cc_gps_sat;	/* tracking data */

	void ParseGPSDate(String date) throws ParseException {
		String[] ymd = date.split("-");
		if (ymd.length != 3)
			throw new ParseException("error parsing GPS date " + date + " got " + ymd.length, 0);
		year = AltosParse.parse_int(ymd[0]);
		month = AltosParse.parse_int(ymd[1]);
		day = AltosParse.parse_int(ymd[2]);
	}

	void ParseGPSTime(String time) throws ParseException {
		String[] hms = time.split(":");
		if (hms.length != 3)
			throw new ParseException("Error parsing GPS time " + time + " got " + hms.length, 0);
		hour = AltosParse.parse_int(hms[0]);
		minute = AltosParse.parse_int(hms[1]);
		second = AltosParse.parse_int(hms[2]);
	}

	void ClearGPSTime() {
		year = month = day = 0;
		hour = minute = second = 0;
	}

	public AltosGPS(String[] words, int i, int version) throws ParseException {
		AltosParse.word(words[i++], "GPS");
		nsat = AltosParse.parse_int(words[i++]);
		AltosParse.word(words[i++], "sat");

		connected = false;
		locked = false;
		lat = lon = 0;
		alt = 0;
		ClearGPSTime();
		if ((words[i]).equals("unlocked")) {
			connected = true;
			i++;
		} else if ((words[i]).equals("not-connected")) {
			i++;
		} else if (words.length >= 40) {
			locked = true;
			connected = true;

			if (version > 1)
				ParseGPSDate(words[i++]);
			else
				year = month = day = 0;
			ParseGPSTime(words[i++]);
			lat = AltosParse.parse_coord(words[i++]);
			lon = AltosParse.parse_coord(words[i++]);
			alt = AltosParse.parse_int(words[i++]);
			if (version > 1 || (i < words.length && !words[i].equals("SAT"))) {
				ground_speed = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "m/s(H)"));
				course = AltosParse.parse_int(words[i++]);
				climb_rate = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "m/s(V)"));
				hdop = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "(hdop)"));
				h_error = AltosParse.parse_int(words[i++]);
				v_error = AltosParse.parse_int(words[i++]);
			}
		} else {
			i++;
		}
		if (i < words.length) {
			AltosParse.word(words[i++], "SAT");
			int tracking_channels = 0;
			if (words[i].equals("not-connected"))
				tracking_channels = 0;
			else
				tracking_channels = AltosParse.parse_int(words[i]);
			i++;
			cc_gps_sat = new AltosGPS.AltosGPSSat[tracking_channels];
			for (int chan = 0; chan < tracking_channels; chan++) {
				cc_gps_sat[chan] = new AltosGPS.AltosGPSSat();
				cc_gps_sat[chan].svid = AltosParse.parse_int(words[i++]);
				/* Older versions included SiRF status bits */
				if (version < 2)
					i++;
				cc_gps_sat[chan].c_n0 = AltosParse.parse_int(words[i++]);
			}
		} else
			cc_gps_sat = new AltosGPS.AltosGPSSat[0];
	}

	public void set_latitude(int in_lat) {
		lat = in_lat / 10.0e7;
	}

	public void set_longitude(int in_lon) {
		lon = in_lon / 10.0e7;
	}

	public void set_time(int hour, int minute, int second) {
		hour = hour;
		minute = minute;
		second = second;
	}

	public void set_date(int year, int month, int day) {
		year = year;
		month = month;
		day = day;
	}

	public void set_flags(int flags) {
		flags = flags;
	}

	public void set_altitude(int altitude) {
		altitude = altitude;
	}

	public void add_sat(int svid, int c_n0) {
		if (cc_gps_sat == null) {
			cc_gps_sat = new AltosGPS.AltosGPSSat[1];
		} else {
			AltosGPSSat[] new_gps_sat = new AltosGPS.AltosGPSSat[cc_gps_sat.length + 1];
			for (int i = 0; i < cc_gps_sat.length; i++)
				new_gps_sat[i] = cc_gps_sat[i];
			cc_gps_sat = new_gps_sat;
		}
		AltosGPS.AltosGPSSat	sat = new AltosGPS.AltosGPSSat();
		sat.svid = svid;
		sat.c_n0 = c_n0;
		cc_gps_sat[cc_gps_sat.length - 1] = sat;
	}

	public AltosGPS() {
		ClearGPSTime();
		cc_gps_sat = null;
	}

	public AltosGPS(AltosGPS old) {
		nsat = old.nsat;
		locked = old.locked;
		connected = old.connected;
		date_valid = old.date_valid;
		lat = old.lat;		/* degrees (+N -S) */
		lon = old.lon;		/* degrees (+E -W) */
		alt = old.alt;		/* m */
		year = old.year;
		month = old.month;
		day = old.day;
		hour = old.hour;
		minute = old.minute;
		second = old.second;

		gps_extended = old.gps_extended;	/* has extra data */
		ground_speed = old.ground_speed;	/* m/s */
		course = old.course;		/* degrees */
		climb_rate = old.climb_rate;	/* m/s */
		hdop = old.hdop;		/* unitless? */
		h_error = old.h_error;	/* m */
		v_error = old.v_error;	/* m */

		if (old.cc_gps_sat != null) {
			cc_gps_sat = new AltosGPSSat[old.cc_gps_sat.length];
			for (int i = 0; i < old.cc_gps_sat.length; i++) {
				cc_gps_sat[i] = new AltosGPSSat();
				cc_gps_sat[i].svid = old.cc_gps_sat[i].svid;
				cc_gps_sat[i].c_n0 = old.cc_gps_sat[i].c_n0;
			}
		}
	}
}
