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

import java.awt.*;
import java.util.*;
import java.text.*;

public class Altos {
	/* EEProm command letters */
	static final int AO_LOG_FLIGHT = 'F';
	static final int AO_LOG_SENSOR = 'A';
	static final int AO_LOG_TEMP_VOLT = 'T';
	static final int AO_LOG_DEPLOY = 'D';
	static final int AO_LOG_STATE = 'S';
	static final int AO_LOG_GPS_TIME = 'G';
	static final int AO_LOG_GPS_LAT = 'N';
	static final int AO_LOG_GPS_LON = 'W';
	static final int AO_LOG_GPS_ALT = 'H';
	static final int AO_LOG_GPS_SAT = 'V';
	static final int AO_LOG_GPS_DATE = 'Y';

	/* Added for header fields in eeprom files */
	static final int AO_LOG_CONFIG_VERSION = 1000;
	static final int AO_LOG_MAIN_DEPLOY = 1001;
	static final int AO_LOG_APOGEE_DELAY = 1002;
	static final int AO_LOG_RADIO_CHANNEL = 1003;
	static final int AO_LOG_CALLSIGN = 1004;
	static final int AO_LOG_ACCEL_CAL = 1005;
	static final int AO_LOG_RADIO_CAL = 1006;
	static final int AO_LOG_MANUFACTURER = 1007;
	static final int AO_LOG_PRODUCT = 1008;
	static final int AO_LOG_SERIAL_NUMBER = 1009;
	static final int AO_LOG_SOFTWARE_VERSION = 1010;

	/* Added to flag invalid records */
	static final int AO_LOG_INVALID = -1;

	/* Flight state numbers and names */
	static final int ao_flight_startup = 0;
	static final int ao_flight_idle = 1;
	static final int ao_flight_pad = 2;
	static final int ao_flight_boost = 3;
	static final int ao_flight_fast = 4;
	static final int ao_flight_coast = 5;
	static final int ao_flight_drogue = 6;
	static final int ao_flight_main = 7;
	static final int ao_flight_landed = 8;
	static final int ao_flight_invalid = 9;

	static HashMap<String,Integer>	string_to_state = new HashMap<String,Integer>();

	static boolean map_initialized = false;

	static final int tab_elt_pad = 5;

	static final Font label_font = new Font("Dialog", Font.PLAIN, 22);
	static final Font value_font = new Font("Monospaced", Font.PLAIN, 22);
	static final Font status_font = new Font("SansSerif", Font.BOLD, 24);

	static final int text_width = 16;

	static void initialize_map()
	{
		string_to_state.put("startup", ao_flight_startup);
		string_to_state.put("idle", ao_flight_idle);
		string_to_state.put("pad", ao_flight_pad);
		string_to_state.put("boost", ao_flight_boost);
		string_to_state.put("fast", ao_flight_fast);
		string_to_state.put("coast", ao_flight_coast);
		string_to_state.put("drogue", ao_flight_drogue);
		string_to_state.put("main", ao_flight_main);
		string_to_state.put("landed", ao_flight_landed);
		string_to_state.put("invalid", ao_flight_invalid);
		map_initialized = true;
	}

	static String[] state_to_string = {
		"startup",
		"idle",
		"pad",
		"boost",
		"fast",
		"coast",
		"drogue",
		"main",
		"landed",
		"invalid",
	};

	static public int state(String state) {
		if (!map_initialized)
			initialize_map();
		if (string_to_state.containsKey(state))
			return string_to_state.get(state);
		return ao_flight_invalid;
	}

	static public String state_name(int state) {
		if (state < 0 || state_to_string.length <= state)
			return "invalid";
		return state_to_string[state];
	}

	static final int AO_GPS_VALID = (1 << 4);
	static final int AO_GPS_RUNNING = (1 << 5);
	static final int AO_GPS_DATE_VALID = (1 << 6);
	static final int AO_GPS_NUM_SAT_SHIFT = 0;
	static final int AO_GPS_NUM_SAT_MASK = 0xf;

	static boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	static boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	static boolean ishex(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!ishex(s.charAt(i)))
				return false;
		return true;
	}

	static int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	static int fromhex(String s) throws NumberFormatException {
		int c, v = 0;
		for (int i = 0; i < s.length(); i++) {
			c = s.charAt(i);
			if (!ishex(c)) {
				if (i == 0)
					throw new NumberFormatException(String.format("invalid hex \"%s\"", s));
				return v;
			}
			v = v * 16 + fromhex(c);
		}
		return v;
	}

	static boolean isdec(int c) {
		if ('0' <= c && c <= '9')
			return true;
		return false;
	}

	static boolean isdec(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!isdec(s.charAt(i)))
				return false;
		return true;
	}

	static int fromdec(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		return -1;
	}

	static int fromdec(String s) throws NumberFormatException {
		int c, v = 0;
		int sign = 1;
		for (int i = 0; i < s.length(); i++) {
			c = s.charAt(i);
			if (i == 0 && c == '-') {
				sign = -1;
			} else if (!isdec(c)) {
				if (i == 0)
					throw new NumberFormatException(String.format("invalid number \"%s\"", s));
				return v;
			} else
				v = v * 10 + fromdec(c);
		}
		return v * sign;
	}

	static String replace_extension(String input, String extension) {
		int dot = input.lastIndexOf(".");
		if (dot > 0)
			input = input.substring(0,dot);
		return input.concat(extension);
	}
}
