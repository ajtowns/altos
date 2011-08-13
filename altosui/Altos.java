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
import java.nio.charset.Charset;

import libaltosJNI.*;

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
	static final int AO_LOG_PRESSURE = 'P';

	/* Added for header fields in eeprom files */
	static final int AO_LOG_CONFIG_VERSION = 1000;
	static final int AO_LOG_MAIN_DEPLOY = 1001;
	static final int AO_LOG_APOGEE_DELAY = 1002;
	static final int AO_LOG_RADIO_CHANNEL = 1003;
	static final int AO_LOG_CALLSIGN = 1004;
	static final int AO_LOG_ACCEL_CAL = 1005;
	static final int AO_LOG_RADIO_CAL = 1006;
	static final int AO_LOG_MAX_FLIGHT_LOG = 1007;
	static final int AO_LOG_MANUFACTURER = 2000;
	static final int AO_LOG_PRODUCT = 2001;
	static final int AO_LOG_SERIAL_NUMBER = 2002;
	static final int AO_LOG_LOG_FORMAT = 2003;
	static final int AO_LOG_SOFTWARE_VERSION = 9999;

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

	/* Telemetry modes */
	static final int ao_telemetry_off = 0;
	static final int ao_telemetry_min = 1;
	static final int ao_telemetry_standard = 1;
	static final int ao_telemetry_0_9 = 2;
	static final int ao_telemetry_0_8 = 3;
	static final int ao_telemetry_max = 3;

	static final String[] ao_telemetry_name = {
		"Off", "Standard Telemetry", "TeleMetrum v0.9", "TeleMetrum v0.8"
	};

	static final String launch_sites_url = "http://www.altusmetrum.org/AltOS/launch-sites.txt";

	static final int ao_telemetry_standard_len = 32;
	static final int ao_telemetry_0_9_len = 95;
	static final int ao_telemetry_0_8_len = 94;

	static final int[] ao_telemetry_len = {
		0, 32, 95, 94
	};

	static HashMap<String,Integer>	string_to_state = new HashMap<String,Integer>();

	static boolean map_initialized = false;

	static final int tab_elt_pad = 5;

	static final Font label_font = new Font("Dialog", Font.PLAIN, 22);
	static final Font value_font = new Font("Monospaced", Font.PLAIN, 22);
	static final Font status_font = new Font("SansSerif", Font.BOLD, 24);

	static final int text_width = 20;

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

	static int telemetry_len(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_len[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
	}

	static String telemetry_name(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_name[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
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

	static String[] state_to_string_capital = {
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

	static final int AO_LOG_FORMAT_UNKNOWN = 0;
	static final int AO_LOG_FORMAT_FULL = 1;
	static final int AO_LOG_FORMAT_TINY = 2;
	static final int AO_LOG_FORMAT_TELEMETRY = 3;
	static final int AO_LOG_FORMAT_TELESCIENCE = 4;
	static final int AO_LOG_FORMAT_NONE = 127;

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

	static int int8(int[] bytes, int i) {
		return (int) (byte) bytes[i];
	}

	static int uint8(int[] bytes, int i) {
		return bytes[i];
	}

	static int int16(int[] bytes, int i) {
		return (int) (short) (bytes[i] + (bytes[i+1] << 8));
	}

	static int uint16(int[] bytes, int i) {
		return bytes[i] + (bytes[i+1] << 8);
	}

	static int uint32(int[] bytes, int i) {
		return bytes[i] +
			(bytes[i+1] << 8) +
			(bytes[i+2] << 16) +
			(bytes[i+3] << 24);
	}

	static final Charset	unicode_set = Charset.forName("UTF-8");

	static String string(int[] bytes, int s, int l) {
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

	static int hexbyte(String s, int i) {
		int c0, c1;

		if (s.length() < i + 2)
			throw new NumberFormatException(String.format("invalid hex \"%s\"", s));
		c0 = s.charAt(i);
		if (!Altos.ishex(c0))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c0));
		c1 = s.charAt(i+1);
		if (!Altos.ishex(c1))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c1));
		return Altos.fromhex(c0) * 16 + Altos.fromhex(c1);
	}

	static int[] hexbytes(String s) {
		int	n;
		int[]	r;
		int	i;

		if ((s.length() & 1) != 0)
			throw new NumberFormatException(String.format("invalid line \"%s\"", s));
		n = s.length() / 2;
		r = new int[n];
		for (i = 0; i < n; i++)
			r[i] = Altos.hexbyte(s, i * 2);
		return r;
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

	static public boolean initialized = false;
	static public boolean loaded_library = false;

	public static boolean load_library() {
		if (!initialized) {
			try {
				System.loadLibrary("altos");
				libaltos.altos_init();
				loaded_library = true;
			} catch (UnsatisfiedLinkError e) {
				loaded_library = false;
			}
			initialized = true;
		}
		return loaded_library;
	}

	static int usb_vendor_altusmetrum() {
		load_library();
		return 0xfffe;
	}

	static int usb_product_altusmetrum() {
		load_library();
		return 0x000a;
	}

	static int usb_product_altusmetrum_min() {
		load_library();
		return 0x000a;
	}

	static int usb_product_altusmetrum_max() {
		load_library();
		return 0x0013;
	}

	static int usb_product_telemetrum() {
		load_library();
		return 0x000b;
	}

	static int usb_product_teledongle() {
		load_library();
		return 0x000c;
	}

	static int usb_product_teleterra() {
		load_library();
		return 0x000d;
	}

	static int usb_product_telebt() {
		load_library();
		return 0x000e;
	}

	static int usb_product_telelaunch() {
		load_library();
		return 0x000f;
	}

	static int usb_product_telelco() {
		load_library();
		return 0x0010;
	}

	static int usb_product_telescience() {
		load_library();
		return 0x0011;
	}

	static int usb_product_telepyro() {
		load_library();
		return 0x0012;
	}

	public final static int vendor_altusmetrum = usb_vendor_altusmetrum();
	public final static int product_altusmetrum = usb_product_altusmetrum();
	public final static int product_telemetrum = usb_product_telemetrum();
	public final static int product_teledongle = usb_product_teledongle();
	public final static int product_teleterra = usb_product_teleterra();
	public final static int product_telebt = usb_product_telebt();
	public final static int product_telelaunch = usb_product_telelaunch();
	public final static int product_tele10 = usb_product_telelco();
	public final static int product_telescience = usb_product_telescience();
	public final static int product_telepyro = usb_product_telepyro();
	public final static int product_altusmetrum_min = usb_product_altusmetrum_min();
	public final static int product_altusmetrum_max = usb_product_altusmetrum_max();

	public final static int product_any = 0x10000;
	public final static int product_basestation = 0x10000 + 1;

	static String bt_product_telebt() {
		load_library();
		return "TeleBT";
	}

	public final static String bt_product_telebt = bt_product_telebt();

//	public static AltosBTKnown bt_known = new AltosBTKnown();
}
