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

public class AltosFlightStatus extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public class FlightValue {
		JLabel		label;
		JTextField	value;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
		}

		void set_font() {
			label.setFont(Altos.status_font);
			value.setFont(Altos.status_font);
		}

		public FlightValue (GridBagLayout layout, int x, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(5, 5, 5, 5);
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.status_font);
			label.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField("");
			value.setFont(Altos.status_font);
			value.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Call extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			value.setText(state.callsign);
		}
		public Call (GridBagLayout layout, int x) {
			super (layout, x, "Callsign");
		}
	}

	Call call;

	class Serial extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			if (state.serial == AltosLib.MISSING)
				value.setText("none");
			else
				value.setText(String.format("%d", state.serial));
		}
		public Serial (GridBagLayout layout, int x) {
			super (layout, x, "Serial");
		}
	}

	Serial serial;

	class Flight extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			if (state.flight == AltosLib.MISSING)
				value.setText("none");
			else
				value.setText(String.format("%d", state.flight));
		}
		public Flight (GridBagLayout layout, int x) {
			super (layout, x, "Flight");
		}
	}

	Flight flight;

	class FlightState extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			value.setText(state.state_name());
		}
		public FlightState (GridBagLayout layout, int x) {
			super (layout, x, "State");
		}
	}

	FlightState flight_state;

	class RSSI extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			value.setText(String.format("%d", state.rssi()));
		}
		public RSSI (GridBagLayout layout, int x) {
			super (layout, x, "RSSI");
		}
	}

	RSSI rssi;

	class LastPacket extends FlightValue {
		void show(AltosState state, AltosListenerState listener_state) {
			long secs = (System.currentTimeMillis() - state.received_time + 500) / 1000;
			value.setText(String.format("%d", secs));
		}
		public LastPacket(GridBagLayout layout, int x) {
			super (layout, x, "Age");
		}
	}

	LastPacket last_packet;

	public void reset () {
		call.reset();
		serial.reset();
		flight.reset();
		flight_state.reset();
		rssi.reset();
		last_packet.reset();
	}

	public void set_font () {
		call.set_font();
		serial.set_font();
		flight.set_font();
		flight_state.set_font();
		rssi.set_font();
		last_packet.set_font();
	}

	public void show (AltosState state, AltosListenerState listener_state) {
		call.show(state, listener_state);
		serial.show(state, listener_state);
		flight.show(state, listener_state);
		flight_state.show(state, listener_state);
		rssi.show(state, listener_state);
		last_packet.show(state, listener_state);
	}

	public int height() {
		Dimension d = layout.preferredLayoutSize(this);
		return d.height;
	}

	public AltosFlightStatus() {
		layout = new GridBagLayout();

		setLayout(layout);

		call = new Call(layout, 0);
		serial = new Serial(layout, 1);
		flight = new Flight(layout, 2);
		flight_state = new FlightState(layout, 3);
		rssi = new RSSI(layout, 4);
		last_packet = new LastPacket(layout, 5);
	}
}
