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

public abstract class AltosUIIndicator implements AltosFontListener, AltosUnitsListener {
	JLabel		label;
	JTextField[]	values;
	AltosLights	lights;
	int		number_values;
	boolean		has_lights;
	int		value_width;

	abstract public void show(AltosState state, AltosListenerState listener_state);

	public void set_lights(boolean on) {
		lights.set(on);
	}

	public void setVisible(boolean visible) {
		if (lights != null)
			lights.setVisible(visible);
		label.setVisible(visible);
		for (int i = 0; i < values.length; i++)
			values[i].setVisible(visible);
	}

	public void reset() {
		for (int i = 0; i < values.length; i++)
			values[i].setText("");
		if (lights != null)
			lights.set(false);
	}

	public void show() {
		if (lights != null)
			lights.setVisible(true);
		label.setVisible(true);
		for (int i = 0; i < values.length; i++)
			values[i].setVisible(true);
	}

	public void show(String... s) {
		int	n = Math.min(s.length, values.length);

		show();
		for (int i = 0; i < n; i++)
			values[i].setText(s[i]);
	}

	public void show(String format, double value) {
		show(String.format(format, value));
	}

	public void show(String format, int value) {
		show(String.format(format, value));
	}

	public void show(String format1, double value1, String format2, double value2) {
		show(String.format(format1, value1), String.format(format2, value2));
	}

	public void show(String format1, int value1, String format2, int value2) {
		show(String.format(format1, value1), String.format(format2, value2));
	}

	public void hide() {
		if (lights != null)
			lights.setVisible(false);
		label.setVisible(false);
		for (int i = 0; i < values.length; i++)
			values[i].setVisible(false);
	}

	public void font_size_changed(int font_size) {
		label.setFont(AltosUILib.label_font);
		for (int i = 0; i < values.length; i++)
			values[i].setFont(AltosUILib.value_font);
	}

	public void units_changed(boolean imperial_units) {
	}

	public void set_label(String text) {
		label.setText(text);
	}

	public void remove(Container container) {
		if (lights != null)
			container.remove(lights);
		container.remove(label);
		for (int i = 0; i < values.length; i++)
			container.remove(values[i]);
	}

	public AltosUIIndicator (Container container, int x, int y, int label_width, String text, int number_values, boolean has_lights, int value_width, int value_space) {
		GridBagLayout		layout = (GridBagLayout)(container.getLayout());

		GridBagConstraints	c = new GridBagConstraints();
		c.weighty = 1;

		if (has_lights) {
			lights = new AltosLights();
			c.gridx = x; c.gridy = y;
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(lights, c);
			container.add(lights);
		}

		label = new JLabel(text);
		label.setFont(AltosUILib.label_font);
		label.setHorizontalAlignment(SwingConstants.LEFT);
		c.gridx = x + 1; c.gridy = y;
		c.gridwidth = label_width;
		c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.VERTICAL;
		c.weightx = 0;
		layout.setConstraints(label, c);
		container.add(label);

		values = new JTextField[number_values];
		for (int i = 0; i < values.length; i++) {
			values[i] = new JTextField(AltosUILib.text_width);
			values[i].setEditable(false);
			values[i].setFont(AltosUILib.value_font);
			values[i].setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1 + label_width + x + i * value_space; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.gridwidth = value_width;
			layout.setConstraints(values[i], c);
			container.add(values[i]);
		}
	}

	public AltosUIIndicator (Container container, int x, int y, int label_width, String text, int number_values, boolean has_lights, int value_width) {
		this(container, x, y, label_width, text, number_values, has_lights, value_width, 1);
	}

	public AltosUIIndicator (Container container, int x, int y, String text, int number_values, boolean has_lights, int value_width) {
		this(container, x, y, 1, text, number_values, has_lights, value_width);
	}

	public AltosUIIndicator (Container container, int y, String text, int number_values, boolean has_lights, int value_width) {
		this(container, 0, y, text, number_values, has_lights, value_width);
	}

	public AltosUIIndicator (Container container, int y, String text, int number_values, boolean has_lights) {
		this(container, 0, y, text, number_values, has_lights, 1);
	}

	public AltosUIIndicator (Container container, int y, String text, int number_values) {
		this(container, 0, y, text, number_values, false, 1);
	}

	public AltosUIIndicator (Container container, int y, String text) {
		this(container, 0, y, text, 1, false, 1);
	}
}
