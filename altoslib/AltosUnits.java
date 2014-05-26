/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

public abstract class AltosUnits {

	public abstract double value(double v, boolean imperial_units);

	public abstract double inverse(double v, boolean imperial_units);

	public abstract String show_units(boolean imperial_units);

	public abstract String say_units(boolean imperial_units);

	public abstract int show_fraction(int width, boolean imperial_units);

	public double parse(String s, boolean imperial_units) throws NumberFormatException {
		double v = Double.parseDouble(s);
		return inverse(v, imperial_units);
	}

	public double parse(String s) throws NumberFormatException {
		return parse(s, AltosConvert.imperial_units);
	}

	public double value(double v) {
		return value(v, AltosConvert.imperial_units);
	}

	public double inverse(double v) {
		return inverse(v, AltosConvert.imperial_units);
	}

	public String show_units() {
		return show_units(AltosConvert.imperial_units);
	}

	public String say_units() {
		return say_units(AltosConvert.imperial_units);
	}

	public int show_fraction(int width) {
		return show_fraction(width, AltosConvert.imperial_units);
	}

	int say_fraction(boolean imperial_units) {
		return 0;
	}

	private String show_format(int width, boolean imperial_units) {
		return String.format("%%%d.%df %s", width, show_fraction(width, imperial_units), show_units(imperial_units));
	}

	private String say_format(boolean imperial_units) {
		return String.format("%%1.%df", say_fraction(imperial_units));
	}

	private String say_units_format(boolean imperial_units) {
		return String.format("%%1.%df %s", say_fraction(imperial_units), say_units(imperial_units));
	}

	public String graph_format(int width, boolean imperial_units) {
		return String.format(String.format("%%%d.%df", width, show_fraction(width, imperial_units)), 0.0);
	}

	public String graph_format(int width) {
		return graph_format(width, AltosConvert.imperial_units);
	}

	public String show(int width, double v, boolean imperial_units) {
		return String.format(show_format(width, imperial_units), value(v, imperial_units));
	}

	public String show(int width, double v) {
		return show(width, v, AltosConvert.imperial_units);
	}

	public String say(double v, boolean imperial_units) {
		return String.format(say_format(imperial_units), value(v, imperial_units));
	}

	public String say(double v) {
		return say(v, AltosConvert.imperial_units);
	}

	public String say_units(double v, boolean imperial_units) {
		return String.format(say_units_format(imperial_units), value(v, imperial_units));
	}

	public String say_units(double v) {
		return say_units(v, AltosConvert.imperial_units);
	}
}