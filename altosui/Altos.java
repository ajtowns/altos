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
