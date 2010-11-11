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

public class AltosPad extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	Font		label_font;
	Font		value_font;

	public class LaunchStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		void show(AltosState state, int crc_errors) {}
		void reset() {
			value.setText("");
			lights.set(false);
		}

		public LaunchStatus (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();

			lights = new AltosLights();
			c.gridx = 0; c.gridy = y;
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.CENTER;
			layout.setConstraints(lights, c);
			add(lights);

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.WEST;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(15);
			value.setFont(value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			layout.setConstraints(value, c);
			add(value);

		}
	}

	public class LaunchValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
		}
		public LaunchValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();

			label = new JLabel(text);
			label.setFont(label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(30);
			value.setFont(value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.HORIZONTAL;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Battery extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.2f V", state.battery));
			lights.set(state.battery > 3.7);
		}
		public Battery (GridBagLayout layout, int y) {
			super(layout, y, "Battery Voltage");
		}
	}

	Battery	battery;

	class Apogee extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.2f V", state.drogue_sense));
			lights.set(state.drogue_sense > 3.2);
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.2f V", state.main_sense));
			lights.set(state.main_sense > 3.2);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class GPS extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4d sats", state.gps.nsat));
			lights.set(state.gps_ready);
		}
		public GPS (GridBagLayout layout, int y) {
			super (layout, y, "GPS Status");
		}
	}

	GPS gps;

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

	class PadLat extends LaunchValue {
		void show (AltosState state, int crc_errors) {
			value.setText(pos(state.pad_lat,"N", "S"));
		}
		public PadLat (GridBagLayout layout, int y) {
			super (layout, y, "Pad Latitude");
		}
	}

	PadLat pad_lat;

	class PadLon extends LaunchValue {
		void show (AltosState state, int crc_errors) {
			value.setText(pos(state.pad_lon,"E", "W"));
		}
		public PadLon (GridBagLayout layout, int y) {
			super (layout, y, "Pad Longitude");
		}
	}

	PadLon pad_lon;

	class PadAlt extends LaunchValue {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.0f m", state.pad_alt));
		}
		public PadAlt (GridBagLayout layout, int y) {
			super (layout, y, "Pad Altitude");
		}
	}

	PadAlt pad_alt;

	public void reset() {
		battery.reset();
		apogee.reset();
		main.reset();
		gps.reset();
		pad_lat.reset();
		pad_lon.reset();
		pad_alt.reset();
	}

	public void show(AltosState state, int crc_errors) {
		battery.show(state, crc_errors);
		apogee.show(state, crc_errors);
		main.show(state, crc_errors);
		gps.show(state, crc_errors);
		pad_lat.show(state, crc_errors);
		pad_lon.show(state, crc_errors);
		pad_alt.show(state, crc_errors);
	}

	public AltosPad() {
		layout = new GridBagLayout();

		GridBagConstraints	c;

		label_font = new Font("Dialog", Font.PLAIN, 24);
		value_font = new Font("Monospaced", Font.PLAIN, 24);
		setLayout(layout);

		c = new GridBagConstraints();
		/* Elements in pad display:
		 *
		 * Battery voltage
		 * Igniter continuity
		 * GPS lock status and location
		 * Pad altitude
		 * RSSI
		 */
		battery = new Battery(layout, 0);
		apogee = new Apogee(layout, 1);
		main = new Main(layout, 2);
		gps = new GPS(layout, 3);
		pad_lat = new PadLat(layout, 4);
		pad_lon = new PadLon(layout, 5);
		pad_alt = new PadAlt(layout, 6);
	}
}
