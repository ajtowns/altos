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
import org.altusmetrum.altoslib_2.*;

public class AltosPad extends JComponent implements AltosFlightDisplay {
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

	public class LaunchValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, AltosListenerState listener_state) {}

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

		void show(String s) {
			show();
			value.setText(s);
		}

		void show(AltosUnits units, double v) {
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		public void set_label(String text) {
			label.setText(text);
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
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.battery_voltage == AltosLib.MISSING)
				hide();
			else {
				show("%4.2f V", state.battery_voltage);
				lights.set(state.battery_voltage >= AltosLib.ao_battery_good);
			}
		}
		public Battery (GridBagLayout layout, int y) {
			super(layout, y, "Battery Voltage");
		}
	}

	Battery	battery;

	class Apogee extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.apogee_voltage == AltosLib.MISSING)
				hide();
			else {
				show("%4.2f V", state.apogee_voltage);
				lights.set(state.apogee_voltage >= AltosLib.ao_igniter_good);
			}
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.main_voltage == AltosLib.MISSING)
				hide();
			else {
				show("%4.2f V", state.main_voltage);
				lights.set(state.main_voltage >= AltosLib.ao_igniter_good);
			}
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class LoggingReady extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.flight == AltosLib.MISSING) {
				hide();
			} else {
				if (state.flight != 0) {
					if (state.state <= Altos.ao_flight_pad)
						show("Ready to record");
					else if (state.state < Altos.ao_flight_landed)
						show("Recording data");
					else
						show("Recorded data");
				} else
					show("Storage full");
				lights.set(state.flight != 0);
			}
		}
		public LoggingReady (GridBagLayout layout, int y) {
			super(layout, y, "On-board Data Logging");
		}
	}

	LoggingReady logging_ready;

	class GPSLocked extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.gps == null)
				hide();
			else {
				show("%4d sats", state.gps.nsat);
				lights.set(state.gps.locked && state.gps.nsat >= 4);
			}
		}
		public GPSLocked (GridBagLayout layout, int y) {
			super (layout, y, "GPS Locked");
		}
	}

	GPSLocked gps_locked;

	class GPSReady extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.gps == null)
				hide();
			else {
				if (state.gps_ready)
					show("Ready");
				else
					show("Waiting %d", state.gps_waiting);
				lights.set(state.gps_ready);
			}
		}
		public GPSReady (GridBagLayout layout, int y) {
			super (layout, y, "GPS Ready");
		}
	}

	GPSReady gps_ready;

	class ReceiverBattery extends LaunchStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			if (listener_state == null || listener_state.battery == AltosLib.MISSING)
				hide();
			else {
				show("%4.2f V", listener_state.battery);
				lights.set(listener_state.battery > AltosLib.ao_battery_good);
			}
		}
		public ReceiverBattery (GridBagLayout layout, int y) {
			super(layout, y, "Receiver Battery");
		}
	}

	ReceiverBattery	receiver_battery;

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
		void show (AltosState state, AltosListenerState listener_state) {
			double lat = AltosLib.MISSING;
			String label = null;

			if (state != null) {
				if (state.state < AltosLib.ao_flight_pad && state.gps != null && state.gps.lat != AltosLib.MISSING) {
					lat = state.gps.lat;
					label = "Latitude";
				} else {
					lat = state.pad_lat;
					label = "Pad Latitude";
				}
			}
			if (lat != AltosLib.MISSING) {
				show(pos(lat,"N", "S"));
				set_label(label);
			} else
				hide();
		}
		public PadLat (GridBagLayout layout, int y) {
			super (layout, y, "Pad Latitude");
		}
	}

	PadLat pad_lat;

	class PadLon extends LaunchValue {
		void show (AltosState state, AltosListenerState listener_state) {
			double lon = AltosLib.MISSING;
			String label = null;

			if (state != null) {
				if (state.state < AltosLib.ao_flight_pad && state.gps != null && state.gps.lon != AltosLib.MISSING) {
					lon = state.gps.lon;
					label = "Longitude";
				} else {
					lon = state.pad_lon;
					label = "Pad Longitude";
				}
			}
			if (lon != AltosLib.MISSING) {
				show(pos(lon,"E", "W"));
				set_label(label);
			} else
				hide();
		}
		public PadLon (GridBagLayout layout, int y) {
			super (layout, y, "Pad Longitude");
		}
	}

	PadLon pad_lon;

	class PadAlt extends LaunchValue {
		void show (AltosState state, AltosListenerState listener_state) {
			double alt = AltosLib.MISSING;
			String label = null;

			if (state != null) {
				if (state.state < AltosLib.ao_flight_pad && state.gps != null && state.gps.alt != AltosLib.MISSING) {
					alt = state.gps.alt;
					label = "Altitude";
				} else {
					alt = state.pad_alt;
					label = "Pad Altitude";
				}
			}
			if (alt != AltosLib.MISSING) {
				show("%4.0f m", state.gps.alt);
				set_label(label);
			} else
				hide();
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
		receiver_battery.reset();
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
		receiver_battery.set_font();
		pad_lat.set_font();
		pad_lon.set_font();
		pad_alt.set_font();
	}
	
	public void show(AltosState state, AltosListenerState listener_state) {
		battery.show(state, listener_state);
		apogee.show(state, listener_state);
		main.show(state, listener_state);
		logging_ready.show(state, listener_state);
		pad_alt.show(state, listener_state);
		receiver_battery.show(state, listener_state);
		gps_locked.show(state, listener_state);
		gps_ready.show(state, listener_state);
		pad_lat.show(state, listener_state);
		pad_lon.show(state, listener_state);
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
		receiver_battery = new ReceiverBattery(layout, 6);
		pad_lat = new PadLat(layout, 7);
		pad_lon = new PadLon(layout, 8);
		pad_alt = new PadAlt(layout, 9);
		show(null, null);
	}
}
