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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

public class AltosAscent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	Font		label_font;
	Font		value_font;

	public class AscentValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
		}
		public AscentValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(20);
			value.setFont(label_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.HORIZONTAL;
			c.gridwidth = 2;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	public class AscentValueHold {
		JLabel		label;
		JTextField	value;
		JTextField	max_value;
		double		max;

		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
			max_value.setText("");
			max = 0;
		}

		void show(String format, double v) {
			value.setText(String.format(format, v));
			if (v > max) {
				max_value.setText(String.format(format, v));
				max = v;
			}
		}
		public AscentValueHold (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(10);
			value.setFont(label_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			layout.setConstraints(value, c);
			add(value);

			max_value = new JTextField(10);
			max_value.setFont(label_font);
			max_value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			layout.setConstraints(max_value, c);
			add(max_value);
		}
	}


	class Height extends AscentValueHold {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.height);
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, "Height");
		}
	}

	Height	height;

	class Speed extends AscentValueHold {
		void show (AltosState state, int crc_errors) {
			double speed = state.speed;
			if (!state.ascent)
				speed = state.baro_speed;
			show("%6.0f m/s", speed);
		}
		public Speed (GridBagLayout layout, int y) {
			super (layout, y, "Speed");
		}
	}

	Speed	speed;

	class Accel extends AscentValueHold {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m/s²", state.acceleration);
		}
		public Accel (GridBagLayout layout, int y) {
			super (layout, y, "Acceleration");
		}
	}

	Accel	accel;

	String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%s %4d° %9.6f", h, deg, min);
	}

	class Lat extends AscentValue {
		void show (AltosState state, int crc_errors) {
			if (state.gps != null)
				value.setText(pos(state.gps.lat,"N", "S"));
			else
				value.setText("???");
		}
		public Lat (GridBagLayout layout, int y) {
			super (layout, y, "Latitude");
		}
	}

	Lat lat;

	class Lon extends AscentValue {
		void show (AltosState state, int crc_errors) {
			if (state.gps != null)
				value.setText(pos(state.gps.lon,"E", "W"));
			else
				value.setText("???");
		}
		public Lon (GridBagLayout layout, int y) {
			super (layout, y, "Longitude");
		}
	}

	Lon lon;

	public void reset() {
		lat.reset();
		lon.reset();
		height.reset();
		speed.reset();
		accel.reset();
	}

	public void show(AltosState state, int crc_errors) {
		lat.show(state, crc_errors);
		lon.show(state, crc_errors);
		height.show(state, crc_errors);
		speed.show(state, crc_errors);
		accel.show(state, crc_errors);
	}

	public void labels(GridBagLayout layout, int y) {
		GridBagConstraints	c;
		JLabel			cur, max;

		cur = new JLabel("Current");
		cur.setFont(label_font);
		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = y;
		c.insets = new Insets(10, 10, 10, 10);
		layout.setConstraints(cur, c);
		add(cur);

		max = new JLabel("Maximum");
		max.setFont(label_font);
		c.gridx = 2; c.gridy = y;
		layout.setConstraints(max, c);
		add(max);
	}

	public AltosAscent() {
		layout = new GridBagLayout();

		label_font = new Font("Dialog", Font.PLAIN, 24);
		value_font = new Font("Monospace", Font.PLAIN, 24);
		setLayout(layout);

		/* Elements in ascent display:
		 *
		 * lat
		 * lon
		 * height
		 */
		labels(layout, 0);
		height = new Height(layout, 1);
		speed = new Speed(layout, 2);
		accel = new Accel(layout, 3);
		lat = new Lat(layout, 4);
		lon = new Lon(layout, 5);
	}
}
