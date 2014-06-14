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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosIgnitor extends AltosUIFlightTab {

	public class Ignitor extends AltosUIUnitsIndicator {
		int		ignitor;

		public double value(AltosState state, int i) {
			if (state.ignitor_voltage == null ||
			    state.ignitor_voltage.length < ignitor)
				return AltosLib.MISSING;
			return state.ignitor_voltage[ignitor];
		}

		public double good() { return AltosLib.ao_igniter_good; }

		public Ignitor (AltosUIFlightTab container, int y) {
			super(container, y, AltosConvert.voltage, String.format ("%s Voltage", AltosLib.ignitor_name(y)), 1, true, 1);
			ignitor = y;
		}
	}

	Ignitor[] ignitors;

	public void show(AltosState state, AltosListenerState listener_state) {
		if (isShowing())
			make_ignitors(state);
		super.show(state, listener_state);
	}

	public boolean should_show(AltosState state) {
		if (state == null)
			return false;
		if (state.ignitor_voltage == null)
			return false;
		return state.ignitor_voltage.length > 0;
	}

	void make_ignitors(AltosState state) {
		int n = (state == null || state.ignitor_voltage == null) ? 0 : state.ignitor_voltage.length;
		int old_n = ignitors == null ? 0 : ignitors.length;

		if (n != old_n) {

			if (ignitors != null) {
				for (int i = 0; i < ignitors.length; i++) {
					remove(ignitors[i]);
					ignitors[i].remove(this);
					ignitors = null;
				}
			}

			if (n > 0) {
				setVisible(true);
				ignitors = new Ignitor[n];
				for (int i = 0; i < n; i++) {
					ignitors[i] = new Ignitor(this, i);
					add(ignitors[i]);
				}
			} else
				setVisible(false);
		}
	}

	public String getName() {
		return "Ignitors";
	}
}
