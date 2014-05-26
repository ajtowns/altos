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
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosIgnitor extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public class LaunchStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
			lights.set(false);
		}

		public void show() {
			label.setVisible(true);
			value.setVisible(true);
			lights.setVisible(true);
		}

		void show(String s) {
			show();
			value.setText(s);
		}

		void show(String format, double value) {
			show(String.format(format, value));
		}

		void show(String format, int value) {
			show(String.format(format, value));
		}

		public void hide() {
			label.setVisible(false);
			value.setVisible(false);
			lights.setVisible(false);
		}

		public void dispose() {
			hide();
		}

		public void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		public void set_label(String text) {
			label.setText(text);
		}

		public LaunchStatus (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			lights = new AltosLights();
			c.gridx = 0; c.gridy = y;
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(lights, c);
			add(lights);

			label = new JLabel(text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);

		}
	}

	public static String ignitor_name(int i) {
		return String.format("Ignitor %c", 'A' + i);
	}

	class Ignitor extends LaunchStatus {
		int ignitor;

		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.ignitor_voltage[ignitor] == AltosLib.MISSING) {
				hide();
			} else {
				show("%4.2f V", state.ignitor_voltage[ignitor]);
				lights.set(state.ignitor_voltage[ignitor] >= AltosLib.ao_igniter_good);
			}
		}

		public Ignitor (GridBagLayout layout, int y) {
			super(layout, y, String.format ("%s Voltage", ignitor_name(y)));
			ignitor = y;
		}
	}

	Ignitor[] ignitors;

	public void reset() {
		if (ignitors == null)
			return;
		for (int i = 0; i < ignitors.length; i++)
			ignitors[i].reset();
	}

	public void set_font() {
		if (ignitors == null)
			return;
		for (int i = 0; i < ignitors.length; i++)
			ignitors[i].set_font();
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		make_ignitors(state);
		if (ignitors == null)
			return;
		for (int i = 0; i < ignitors.length; i++)
			ignitors[i].show(state, listener_state);
		return;
	}

	public boolean should_show(AltosState state) {
		if (state == null)
			return false;
		if (state.ignitor_voltage == null)
			return false;
		return state.ignitor_voltage.length > 0;
	}

	void make_ignitors(AltosState state) {
		int n = state == null ? 0 : state.ignitor_voltage.length;

		if (n > 0) {

			if (ignitors == null || ignitors.length != n) {
				layout = new GridBagLayout();

				setLayout(layout);
				ignitors = new Ignitor[n];
				for (int i = 0; i < n; i++)
					ignitors[i] = new Ignitor(layout, i);
			}
		} else {
			if (ignitors != null) {
				for (int i = 0; i < n; i++)
					ignitors[i].dispose();
				ignitors = null;
				setVisible(false);
			}
		}
	}

	public String getName() {
		return "Ignitors";
	}

	public AltosIgnitor() {
		make_ignitors(null);
	}
}
