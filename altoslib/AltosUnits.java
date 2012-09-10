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

package org.altusmetrum.AltosLib;

public abstract class AltosUnits {

	public abstract double value(double v);

	public abstract String show_units();

	public abstract String say_units();

	abstract int show_fraction(int width);

	int say_fraction() {
		return 0;
	}

	private String show_format(int width) {
		return String.format("%%%d.%df %s", width, show_fraction(width), show_units());
	}

	private String say_format() {
		return String.format("%%1.%df", say_fraction());
	}

	private String say_units_format() {
		return String.format("%%1.%df %s", say_fraction(), say_units());
	}

	public String show(int width, double v) {
		return String.format(show_format(width), value(v));
	}

	public String say(double v) {
		return String.format(say_format(), value(v));
	}

	public String say_units(double v) {
		return String.format(say_units_format(), value(v));
	}
}