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
import altosui.AltosParse;


public class AltosGPS {
	public class AltosGPSTime {
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;

		public AltosGPSTime(String date, String time) throws ParseException {
			String[] ymd = date.split("-");
			if (ymd.length != 3)
				throw new ParseException("error parsing GPS date " + date + " got " + ymd.length, 0);
			year = AltosParse.parse_int(ymd[0]);
			month = AltosParse.parse_int(ymd[1]);
			day = AltosParse.parse_int(ymd[2]);

			String[] hms = time.split(":");
			if (hms.length != 3)
				throw new ParseException("Error parsing GPS time " + time + " got " + hms.length, 0);
			hour = AltosParse.parse_int(hms[0]);
			minute = AltosParse.parse_int(hms[1]);
			second = AltosParse.parse_int(hms[2]);
		}

		public AltosGPSTime() {
			year = month = day = 0;
			hour = minute = second = 0;
		}

	}

	public class AltosGPSSat {
		int	svid;
		int	c_n0;
	}

	int	nsat;
	boolean	gps_locked;
	boolean	gps_connected;
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

	AltosGPSSat[] cc_gps_sat;	/* tracking data */

	public AltosGPS(String[] words, int i) throws ParseException {
		AltosParse.word(words[i++], "GPS");
		nsat = AltosParse.parse_int(words[i++]);
		AltosParse.word(words[i++], "sat");

		gps_connected = false;
		gps_locked = false;
		lat = lon = 0;
		alt = 0;
		if ((words[i]).equals("unlocked")) {
			gps_connected = true;
			gps_time = new AltosGPSTime();
			i++;
		} else if (words.length >= 40) {
			gps_locked = true;
			gps_connected = true;

			gps_time = new AltosGPSTime(words[i], words[i+1]); i += 2;
			lat = AltosParse.parse_coord(words[i++]);
			lon = AltosParse.parse_coord(words[i++]);
			alt = AltosParse.parse_int(AltosParse.strip_suffix(words[i++], "m"));
			ground_speed = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "m/s(H)"));
			course = AltosParse.parse_int(AltosParse.strip_suffix(words[i++], "Â°"));
			climb_rate = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "m/s(V)"));
			hdop = AltosParse.parse_double(AltosParse.strip_suffix(words[i++], "(hdop)"));
			h_error = AltosParse.parse_int(AltosParse.strip_suffix(words[i++], "(herr)"));
			v_error = AltosParse.parse_int(AltosParse.strip_suffix(words[i++], "(verr)"));
		} else {
			gps_time = new AltosGPSTime();
		}
		AltosParse.word(words[i++], "SAT");
		int tracking_channels = AltosParse.parse_int(words[i++]);
		cc_gps_sat = new AltosGPS.AltosGPSSat[tracking_channels];
		for (int chan = 0; chan < tracking_channels; chan++) {
			cc_gps_sat[chan].svid = AltosParse.parse_int(words[i++]);
			cc_gps_sat[chan].c_n0 = AltosParse.parse_int(words[i++]);
		}
	}
}
