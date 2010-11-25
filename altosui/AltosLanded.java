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

public class AltosLanded extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	Font		label_font;
	Font		value_font;

	public class LandedValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
		}

		void show(String format, double v) {
			value.setText(String.format(format, v));
		}

		public LandedValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			c.weightx = 0;
			c.fill = GridBagConstraints.VERTICAL;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(Altos.text_width);
			value.setFont(value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.weightx = 1;
			c.fill = GridBagConstraints.BOTH;
			layout.setConstraints(value, c);
			add(value);
		}
	}

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

	class Lat extends LandedValue {
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

	class Lon extends LandedValue {
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

	class Bearing extends LandedValue {
		void show (AltosState state, int crc_errors) {
			if (state.from_pad != null)
				show("%3.0f°", state.from_pad.bearing);
			else
				value.setText("???");
		}
		public Bearing (GridBagLayout layout, int y) {
			super (layout, y, "Bearing");
		}
	}

	Bearing bearing;

	class Distance extends LandedValue {
		void show (AltosState state, int crc_errors) {
			if (state.from_pad != null)
				show("%6.0f m", state.from_pad.distance);
			else
				value.setText("???");
		}
		public Distance (GridBagLayout layout, int y) {
			super (layout, y, "Distance");
		}
	}

	Distance distance;

	class Height extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.max_height);
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Height");
		}
	}

	Height	height;

	class Speed extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m/s", state.max_speed);
		}
		public Speed (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Speed");
		}
	}

	Speed	speed;

	class Accel extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m/s²", state.max_acceleration);
		}
		public Accel (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Acceleration");
		}
	}

	Accel	accel;

	public void reset() {
		lat.reset();
		lon.reset();
		bearing.reset();
		distance.reset();
		height.reset();
		speed.reset();
		accel.reset();
	}

	public void show(AltosState state, int crc_errors) {
		bearing.show(state, crc_errors);
		distance.show(state, crc_errors);
		lat.show(state, crc_errors);
		lon.show(state, crc_errors);
		height.show(state, crc_errors);
		speed.show(state, crc_errors);
		accel.show(state, crc_errors);
	}

	public AltosLanded() {
		layout = new GridBagLayout();

		label_font = new Font("Dialog", Font.PLAIN, 22);
		value_font = new Font("Monospaced", Font.PLAIN, 22);
		setLayout(layout);

		/* Elements in descent display */
		bearing = new Bearing(layout, 0);
		distance = new Distance(layout, 1);
		lat = new Lat(layout, 2);
		lon = new Lon(layout, 3);
		height = new Height(layout, 4);
		speed = new Speed(layout, 5);
		accel = new Accel(layout, 6);
	}
}
