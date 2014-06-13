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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public abstract class AltosUIUnitsIndicator extends AltosUIIndicator {

	AltosUnits	units;

	abstract public double value(AltosState state, int i);
	public boolean good(double value) { return false; }

	public double[] last_values;

	public void show(double... v) {
		for (int i = 0; i < values.length; i++) {
			if (v[i] != last_values[i]) {
				String	value_text;
				boolean	good = false;

				if (v[i] == AltosLib.MISSING) {
					value_text = "Missing";
				} else {
					value_text = units.show(8, v[i]);
					if (i == 0)
						good = good(v[i]);
				}
				last_values[i] = v[i];
				if (i == 0 && lights != null)
					set_lights(good);
				values[i].setText(value_text);
			}
		}
	}

	public void units_changed(boolean imperial_units) {
		show(last_values);
	}

	public void show (AltosState state, AltosListenerState listener_state) {

		double[] v = new double[values.length];

		for (int i = 0; i < values.length; i++) {
			if (state != null)
				v[i] = value(state, i);
			else
				v[i] = AltosLib.MISSING;
		}
		show(v);

	}

	public AltosUIUnitsIndicator (Container container, int y, AltosUnits units, String name, int number_values, boolean has_lights, int width) {
		super(container, y, name, number_values, has_lights, width);
		this.units = units;
		last_values = new double[values.length];
		for (int i = 0; i < last_values.length; i++)
			last_values[i] = AltosLib.MISSING - 1;
	}
}
