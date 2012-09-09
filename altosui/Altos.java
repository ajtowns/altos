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
}
