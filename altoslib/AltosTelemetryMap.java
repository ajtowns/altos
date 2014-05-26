/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_4;
import java.text.*;
import java.util.HashMap;

public class AltosTelemetryMap extends HashMap<String,String> {
	public boolean has(String key) {
		return containsKey(key);
	}

	public String get_string(String key) throws ParseException {
		if (!has(key))
			throw new ParseException ("missing " + key, 0);
		return (String) get(key);
	}

	public String get_string(String key, String def) {
		if (has(key))
			return get(key);
		else
			return def;
	}

	public int get_int(String key) throws ParseException {
		return AltosParse.parse_int(get_string(key));
	}

	public int get_int(String key, int def) throws ParseException {
		if (has(key))
			return get_int(key);
		else
			return def;
	}

	public double get_double(String key, double def, double scale) throws ParseException {
		if (has(key))
			return get_int(key) * scale;
		else
			return def;
	}

	public AltosTelemetryMap(String[] words, int start) {
		for (int i = start; i < words.length - 1; i += 2)
			put(words[i], words[i+1]);
	}
}
