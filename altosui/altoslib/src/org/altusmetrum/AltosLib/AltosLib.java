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

package org.altusmetrum.AltosLib;

import java.awt.*;
import java.util.*;
import java.text.*;
import java.nio.charset.Charset;

public class AltosLib {
	/* EEProm command letters */
	public static final int AO_LOG_FLIGHT = 'F';
	public static final int AO_LOG_SENSOR = 'A';
	public static final int AO_LOG_TEMP_VOLT = 'T';
	public static final int AO_LOG_DEPLOY = 'D';
	public static final int AO_LOG_STATE = 'S';
	public static final int AO_LOG_GPS_TIME = 'G';
	public static final int AO_LOG_GPS_LAT = 'N';
	public static final int AO_LOG_GPS_LON = 'W';
	public static final int AO_LOG_GPS_ALT = 'H';
	public static final int AO_LOG_GPS_SAT = 'V';
	public static final int AO_LOG_GPS_DATE = 'Y';
	public static final int AO_LOG_PRESSURE = 'P';

	/* Added for header fields in eeprom files */
	public static final int AO_LOG_CONFIG_VERSION = 1000;
	public static final int AO_LOG_MAIN_DEPLOY = 1001;
	public static final int AO_LOG_APOGEE_DELAY = 1002;
	public static final int AO_LOG_RADIO_CHANNEL = 1003;
	public static final int AO_LOG_CALLSIGN = 1004;
	public static final int AO_LOG_ACCEL_CAL = 1005;
	public static final int AO_LOG_RADIO_CAL = 1006;
	public static final int AO_LOG_MAX_FLIGHT_LOG = 1007;
	public static final int AO_LOG_MANUFACTURER = 2000;
	public static final int AO_LOG_PRODUCT = 2001;
	public static final int AO_LOG_SERIAL_NUMBER = 2002;
	public static final int AO_LOG_LOG_FORMAT = 2003;
	public static final int AO_LOG_SOFTWARE_VERSION = 9999;

	/* Added to flag invalid records */
	public static final int AO_LOG_INVALID = -1;

	/* Flight state numbers and names */
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

	/* Telemetry modes */
	public static final int ao_telemetry_off = 0;
	public static final int ao_telemetry_min = 1;
	public static final int ao_telemetry_standard = 1;
	public static final int ao_telemetry_0_9 = 2;
	public static final int ao_telemetry_0_8 = 3;
	public static final int ao_telemetry_max = 3;

	public static final String[] ao_telemetry_name = {
		"Off", "Standard Telemetry", "TeleMetrum v0.9", "TeleMetrum v0.8"
	};

	public static final String launch_sites_url = "http://www.altusmetrum.org/AltOS/launch-sites.txt";

	public static final int ao_telemetry_standard_len = 32;
	public static final int ao_telemetry_0_9_len = 95;
	public static final int ao_telemetry_0_8_len = 94;

	public static final int[] ao_telemetry_len = {
		0, 32, 95, 94
	};

	public static HashMap<String,Integer>	string_to_state = new HashMap<String,Integer>();

	public static boolean map_initialized = false;

	public static void initialize_map()
	{
		string_to_state.put("startup", ao_flight_startup);
		string_to_state.put("idle", ao_flight_idle);
		string_to_state.put("pad", ao_flight_pad);
		string_to_state.put("boost", ao_flight_boost);
		string_to_state.put("fast", ao_flight_fast);
		string_to_state.put("coast", ao_flight_coast);
		string_to_state.put("drogue", ao_flight_drogue);
		string_to_state.put("apogee", ao_flight_coast);
		string_to_state.put("main", ao_flight_main);
		string_to_state.put("landed", ao_flight_landed);
		string_to_state.put("invalid", ao_flight_invalid);
		map_initialized = true;
	}

	public static int telemetry_len(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_len[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
	}

	public static String telemetry_name(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_name[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
	}
	
	public static String[] state_to_string = {
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

	public static String[] state_to_string_capital = {
		"Startup",
		"Idle",
		"Pad",
		"Boost",
		"Fast",
		"Coast",
		"Drogue",
		"Main",
		"Landed",
		"Invalid",
	};

	public static int state(String state) {
		if (!map_initialized)
			initialize_map();
		if (string_to_state.containsKey(state))
			return string_to_state.get(state);
		return ao_flight_invalid;
	}

	public static String state_name(int state) {
		if (state < 0 || state_to_string.length <= state)
			return "invalid";
		return state_to_string[state];
	}

	public static final int AO_GPS_VALID = (1 << 4);
	public static final int AO_GPS_RUNNING = (1 << 5);
	public static final int AO_GPS_DATE_VALID = (1 << 6);
	public static final int AO_GPS_NUM_SAT_SHIFT = 0;
	public static final int AO_GPS_NUM_SAT_MASK = 0xf;

	public static final int AO_LOG_FORMAT_UNKNOWN = 0;
	public static final int AO_LOG_FORMAT_FULL = 1;
	public static final int AO_LOG_FORMAT_TINY = 2;
	public static final int AO_LOG_FORMAT_TELEMETRY = 3;
	public static final int AO_LOG_FORMAT_TELESCIENCE = 4;
	public static final int AO_LOG_FORMAT_NONE = 127;

	public static boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	public static boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	public static boolean ishex(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!ishex(s.charAt(i)))
				return false;
		return true;
	}

	public static int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	public static int fromhex(String s) throws NumberFormatException {
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

	public static boolean isdec(int c) {
		if ('0' <= c && c <= '9')
			return true;
		return false;
	}

	public static boolean isdec(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!isdec(s.charAt(i)))
				return false;
		return true;
	}

	public static int fromdec(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		return -1;
	}

	public static int int8(int[] bytes, int i) {
		return (int) (byte) bytes[i];
	}

	public static int uint8(int[] bytes, int i) {
		return bytes[i];
	}

	public static int int16(int[] bytes, int i) {
		return (int) (short) (bytes[i] + (bytes[i+1] << 8));
	}

	public static int uint16(int[] bytes, int i) {
		return bytes[i] + (bytes[i+1] << 8);
	}

	public static int uint32(int[] bytes, int i) {
		return bytes[i] +
			(bytes[i+1] << 8) +
			(bytes[i+2] << 16) +
			(bytes[i+3] << 24);
	}

	public static final Charset	unicode_set = Charset.forName("UTF-8");

	public static String string(int[] bytes, int s, int l) {
		if (s + l > bytes.length) {
			if (s > bytes.length) {
				s = bytes.length;
				l = 0;
			} else {
				l = bytes.length - s;
			}
		}

		int i;
		for (i = l - 1; i >= 0; i--)
			if (bytes[s+i] != 0)
				break;

		l = i + 1;
		byte[]	b = new byte[l];

		for (i = 0; i < l; i++)
			b[i] = (byte) bytes[s+i];
		String n = new String(b, unicode_set);
		return n;
	}

	public static int hexbyte(String s, int i) {
		int c0, c1;

		if (s.length() < i + 2)
			throw new NumberFormatException(String.format("invalid hex \"%s\"", s));
		c0 = s.charAt(i);
		if (!ishex(c0))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c0));
		c1 = s.charAt(i+1);
		if (!ishex(c1))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c1));
		return fromhex(c0) * 16 + fromhex(c1);
	}

	public static int[] hexbytes(String s) {
		int	n;
		int[]	r;
		int	i;

		if ((s.length() & 1) != 0)
			throw new NumberFormatException(String.format("invalid line \"%s\"", s));
		n = s.length() / 2;
		r = new int[n];
		for (i = 0; i < n; i++)
			r[i] = hexbyte(s, i * 2);
		return r;
	}

	public static int fromdec(String s) throws NumberFormatException {
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

	public static String replace_extension(String input, String extension) {
		int dot = input.lastIndexOf(".");
		if (dot > 0)
			input = input.substring(0,dot);
		return input.concat(extension);
	}
}