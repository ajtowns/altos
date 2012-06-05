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

import org.altusmetrum.AltosLib.*;

public class Altos extends AltosLib {

	static final int tab_elt_pad = 5;

	static Font label_font;
	static Font value_font;
	static Font status_font;
	static Font table_label_font;
	static Font table_value_font;

	final static int font_size_small = 1;
	final static int font_size_medium = 2;
	final static int font_size_large = 3;

	static void set_fonts(int size) {
		int	brief_size;
		int	table_size;
		int	status_size;

		switch (size) {
		case font_size_small:
			brief_size = 16;
			status_size = 18;
			table_size = 11;
			break;
		default:
		case font_size_medium:
			brief_size = 22;
			status_size = 24;
			table_size = 14;
			break;
		case font_size_large:
			brief_size = 26;
			status_size = 30;
			table_size = 17;
			break;
		}
		label_font = new Font("Dialog", Font.PLAIN, brief_size);
		value_font = new Font("Monospaced", Font.PLAIN, brief_size);
		status_font = new Font("SansSerif", Font.BOLD, status_size);
		table_label_font = new Font("SansSerif", Font.PLAIN, table_size);
		table_value_font = new Font("Monospaced", Font.PLAIN, table_size);
	}

	static final int text_width = 20;

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
	static final int AO_LOG_FORMAT_MEGAMETRUM = 5;
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
				try {
					System.loadLibrary("altos64");
					libaltos.altos_init();
					loaded_library = true;
				} catch (UnsatisfiedLinkError e2) {
					loaded_library = false;
				}
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

	public static AltosBTKnown bt_known = new AltosBTKnown();
}
