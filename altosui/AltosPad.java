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
import javax.swing.*;
import org.altusmetrum.AltosLib.*;

public class AltosPad extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public class LaunchStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		void show(AltosState state, int crc_errors) {}
		void reset() {
			value.setText("");
			lights.set(false);
		}

		public void show() {
			label.setVisible(true);
			value.setVisible(true);
			lights.setVisible(true);
		}

		public void hide() {
			label.setVisible(false);
			value.setVisible(false);
			lights.setVisible(false);
		}

		public void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
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

	public class LaunchValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void show() {
			label.setVisible(true);
			value.setVisible(true);
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}

		public void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		void reset() {
			value.setText("");
		}
		public LaunchValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
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
			show();
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
			show();
			value.setText(String.format("%4.2f V", state.main_sense));
			lights.set(state.main_sense > 3.2);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class LoggingReady extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.data.flight != 0) {
				if (state.data.state <= Altos.ao_flight_pad)
					value.setText("Ready to record");
				else if (state.data.state < Altos.ao_flight_landed)
					value.setText("Recording data");
				else
					value.setText("Recorded data");
			}
			else
				value.setText("Storage full");
			lights.set(state.data.flight != 0);
		}
		public LoggingReady (GridBagLayout layout, int y) {
			super(layout, y, "On-board Data Logging");
		}
	}

	LoggingReady logging_ready;

	class GPSLocked extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			show();
			value.setText(String.format("%4d sats", state.gps.nsat));
			lights.set(state.gps.locked && state.gps.nsat >= 4);
		}
		public GPSLocked (GridBagLayout layout, int y) {
			super (layout, y, "GPS Locked");
		}
	}

	GPSLocked gps_locked;

	class GPSReady extends LaunchStatus {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.gps_ready)
				value.setText("Ready");
			else
				value.setText(String.format("Waiting %d", state.gps_waiting));
			lights.set(state.gps_ready);
		}
		public GPSReady (GridBagLayout layout, int y) {
			super (layout, y, "GPS Ready");
		}
	}

	GPSReady gps_ready;

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
			show();
			value.setText(pos(state.pad_lat,"N", "S"));
		}
		public PadLat (GridBagLayout layout, int y) {
			super (layout, y, "Pad Latitude");
		}
	}

	PadLat pad_lat;

	class PadLon extends LaunchValue {
		void show (AltosState state, int crc_errors) {
			show();
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
		logging_ready.reset();
		gps_locked.reset();
		gps_ready.reset();
		pad_lat.reset();
		pad_lon.reset();
		pad_alt.reset();
	}

	public void set_font() {
		battery.set_font();
		apogee.set_font();
		main.set_font();
		logging_ready.set_font();
		gps_locked.set_font();
		gps_ready.set_font();
		pad_lat.set_font();
		pad_lon.set_font();
		pad_alt.set_font();
	}
	
	public void show(AltosState state, int crc_errors) {
		battery.show(state, crc_errors);
		if (state.drogue_sense == AltosRecord.MISSING)
			apogee.hide();
		else
			apogee.show(state, crc_errors);
		if (state.main_sense == AltosRecord.MISSING)
			main.hide();
		else
			main.show(state, crc_errors);
		logging_ready.show(state, crc_errors);
		pad_alt.show(state, crc_errors);
		if (state.gps != null && state.gps.connected) {
			gps_locked.show(state, crc_errors);
			gps_ready.show(state, crc_errors);
			pad_lat.show(state, crc_errors);
			pad_lon.show(state, crc_errors);
		} else {
			gps_locked.hide();
			gps_ready.hide();
			pad_lat.hide();
			pad_lon.hide();
		}
	}

	public AltosPad() {
		layout = new GridBagLayout();

		setLayout(layout);

		/* Elements in pad display:
		 *
		 * Battery voltage
		 * Igniter continuity
		 * GPS lock status
		 * GPS ready status
		 * GPS location
		 * Pad altitude
		 * RSSI
		 */
		battery = new Battery(layout, 0);
		apogee = new Apogee(layout, 1);
		main = new Main(layout, 2);
		logging_ready = new LoggingReady(layout, 3);
		gps_locked = new GPSLocked(layout, 4);
		gps_ready = new GPSReady(layout, 5);
		pad_lat = new PadLat(layout, 6);
		pad_lon = new PadLon(layout, 7);
		pad_alt = new PadAlt(layout, 8);
	}
}
