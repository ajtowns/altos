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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import libaltosJNI.*;

import org.altusmetrum.altoslib_4.*;

public class AltosUILib extends AltosLib {

	public static final int tab_elt_pad = 5;

	public static Font label_font;
	public static Font value_font;
	public static Font status_font;
	public static Font table_label_font;
	public static Font table_value_font;

	final public static int font_size_small = 1;
	final public static int font_size_medium = 2;
	final public static int font_size_large = 3;

	final public static int position_top_left = 0;
	final public static int position_top = 1;
	final public static int position_top_right = 2;
	final public static int position_left = 3;
	final public static int position_center = 4;
	final public static int position_right = 5;
	final public static int position_bottom_left = 6;
	final public static int position_bottom = 7;
	final public static int position_bottom_right = 8;

	public static void set_fonts(int size) {
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

	static public final int text_width = 20;

	static public boolean initialized = false;
	static public boolean loaded_library = false;

	static final String[] library_names = { "altos", "altos32", "altos64" };

	public static boolean load_library() {
		if (!initialized) {
			for (String name : library_names) {
				try {
					System.loadLibrary(name);
					libaltos.altos_init();
					loaded_library = true;
					break;
				} catch (UnsatisfiedLinkError e) {
					System.out.printf("Link error %s\n", e.getMessage());
					loaded_library = false;
				}
			}
			initialized = true;
		}
		return loaded_library;
	}
}
