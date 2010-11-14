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

public class AltosDescent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	Font		label_font;
	Font		value_font;

	public class DescentValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
		}

		void show(String format, double v) {
			value.setText(String.format(format, v));
		}

		public DescentValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(30);
			value.setFont(value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.gridwidth = 2;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Height extends DescentValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.height);
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, "Height");
		}
	}

	Height	height;

	class Speed extends DescentValue {
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

	class Lat extends DescentValue {
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

	class Lon extends DescentValue {
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

	class Bearing {
		JLabel		label;
		JTextField	value;
		JTextField	value_deg;
		void reset () {
			value.setText("");
			value_deg.setText("");
		}
		void show (AltosState state, int crc_errors) {
			if (state.from_pad != null) {
				value.setText(state.from_pad.bearing_words(
						      AltosGreatCircle.BEARING_LONG));
				value_deg.setText(String.format("%3.0f°", state.from_pad.bearing));
			} else {
				value.setText("???");
				value_deg.setText("???");
			}
		}
		public Bearing (GridBagLayout layout, int y) {
			GridBagConstraints      c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel("Bearing");
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			c.weightx = 0;
			c.fill = GridBagConstraints.VERTICAL;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(30);
			value.setFont(value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.weightx = 1;
			c.fill = GridBagConstraints.BOTH;
			layout.setConstraints(value, c);
			add(value);

			value_deg = new JTextField(5);
			value_deg.setFont(value_font);
			value_deg.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.weightx = 1;
			c.fill = GridBagConstraints.BOTH;
			layout.setConstraints(value_deg, c);
			add(value_deg);
		}
	}

	Bearing bearing;

	class Elevation extends DescentValue {
		void show (AltosState state, int crc_errors) {
			if (state.from_pad != null)
				show("%3.0f°", state.elevation);
			else
				value.setText("???");
		}
		public Elevation (GridBagLayout layout, int y) {
			super (layout, y, "Elevation");
		}
	}

	Elevation elevation;

	class Range extends DescentValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.range);
		}
		public Range (GridBagLayout layout, int y) {
			super (layout, y, "Range");
		}
	}

	Range range;

	public void reset() {
		lat.reset();
		lon.reset();
		height.reset();
		speed.reset();
		bearing.reset();
		elevation.reset();
		range.reset();
	}

	public void show(AltosState state, int crc_errors) {
		height.show(state, crc_errors);
		speed.show(state, crc_errors);
		bearing.show(state, crc_errors);
		elevation.show(state, crc_errors);
		range.show(state, crc_errors);
		lat.show(state, crc_errors);
		lon.show(state, crc_errors);
	}

	public AltosDescent() {
		layout = new GridBagLayout();

		label_font = new Font("Dialog", Font.PLAIN, 24);
		value_font = new Font("Monospaced", Font.PLAIN, 24);
		setLayout(layout);

		/* Elements in descent display */
		speed = new Speed(layout, 0);
		height = new Height(layout, 1);
		bearing = new Bearing(layout, 2);
		elevation = new Elevation(layout, 3);
		range = new Range(layout, 4);
		lat = new Lat(layout, 5);
		lon = new Lon(layout, 6);
	}
}
