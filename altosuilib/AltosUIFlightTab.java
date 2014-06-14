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

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public abstract class AltosUIFlightTab extends JComponent implements AltosFlightDisplay, HierarchyListener {
	public GridBagLayout	layout;

	AltosState		last_state;
	AltosListenerState	last_listener_state;

	LinkedList<AltosUIIndicator>	indicators = new LinkedList<AltosUIIndicator>();

	public void add (AltosUIIndicator indicator) {
		indicators.add(indicator);
	}

	public void remove(AltosUIIndicator indicator) {
		indicators.remove(indicator);
	}

	public void reset() {
		for (AltosUIIndicator i : indicators)
			i.reset();
	}

	public void font_size_changed(int font_size) {
		for (AltosUIIndicator i : indicators)
			i.font_size_changed(font_size);
	}

	public void units_changed(boolean imperial_units) {
		for (AltosUIIndicator i : indicators)
			i.units_changed(imperial_units);
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		if (!isShowing()) {
			last_state = state;
			last_listener_state = listener_state;
			return;
		}

		for (AltosUIIndicator i : indicators)
			i.show(state, listener_state);
	}

	public void hierarchyChanged(HierarchyEvent e) {
		if (last_state != null && isShowing()) {
			AltosState		state = last_state;
			AltosListenerState	listener_state = last_listener_state;

			last_state = null;
			last_listener_state = null;
			show(state, listener_state);
		}
	}

	abstract public String getName();

	public AltosUIFlightTab() {
		layout = new GridBagLayout();

		setLayout(layout);

		addHierarchyListener(this);
	}
}
