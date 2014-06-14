/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

public abstract class AltosLocation extends AltosUnits {

	public abstract String pos();
	public abstract String neg();

	public double value(double v, boolean imperial_units) {
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		return v;
	}

	public String show_units(boolean imperial_units) {
		return "°";
	}

	public String say_units(boolean imperial_units) {
		return "degrees";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return 2;
	}

	public String show(int width, double v, boolean imperial_units) {
		String	h = pos();
		if (v < 0) {
			h = neg();
			v = -v;
		}
		int deg = (int) Math.floor(v);
		double min = (v - Math.floor(v)) * 60.0;
		return String.format("%s %4d° %9.6f", h, deg, min);
	}

	public String say(double v, boolean imperial_units) {
		String	h = pos();
		if (v < 0) {
			h = neg();
			v = -v;
		}
		int deg = (int) Math.floor(v);
		double min = (v - Math.floor(v)) * 60.0;
		return String.format("%s %d degrees %d", h, deg, (int) Math.floor(min + 0.5));
	}
}
