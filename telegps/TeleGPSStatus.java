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

package org.altusmetrum.telegps;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class TeleGPSStatus extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public class Value {
		JLabel		label;
		JTextField	value;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
		}

		void set_font() {
			label.setFont(AltosUILib.status_font);
			value.setFont(AltosUILib.status_font);
		}

		void setVisible(boolean visible) {
			label.setVisible(visible);
			value.setVisible(visible);
		}

		public Value (GridBagLayout layout, int x, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(5, 5, 5, 5);
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(AltosUILib.status_font);
			label.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField("");
			value.setEditable(false);
			value.setFont(AltosUILib.status_font);
			value.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Call extends Value {
		String	call;

		void show(AltosState state, AltosListenerState listener_state) {
			if (state.callsign != call) {
				value.setText(state.callsign);
				call = state.callsign;
			}
			if (state.callsign == null)
				setVisible(false);
			else
				setVisible(true);
		}

		public void reset() {
			super.reset();
			call = "";
		}

		public Call (GridBagLayout layout, int x) {
			super (layout, x, "Callsign");
		}
	}

	Call call;

	class Serial extends Value {
		int	serial = -1;
		void show(AltosState state, AltosListenerState listener_state) {
			if (state.serial != serial) {
				if (state.serial == AltosLib.MISSING)
					value.setText("none");
				else
					value.setText(String.format("%d", state.serial));
				serial = state.serial;
			}
		}

		public void reset() {
			super.reset();
			serial = -1;
		}

		public Serial (GridBagLayout layout, int x) {
			super (layout, x, "Serial");
		}
	}

	Serial serial;

	class Flight extends Value {

		int	last_flight = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			if (state.flight != last_flight) {
				if (state.flight == AltosLib.MISSING)
					value.setText("none");
				else
					value.setText(String.format("%d", state.flight));
				last_flight = state.flight;
			}
		}

		public void reset() {
			super.reset();
			last_flight = -1;
		}

		public Flight (GridBagLayout layout, int x) {
			super (layout, x, "Flight");
		}
	}

	Flight flight;

	class RSSI extends Value {
		int	rssi = 10000;

		void show(AltosState state, AltosListenerState listener_state) {
			int	new_rssi = state.rssi();

			if (new_rssi != rssi) {
				value.setText(String.format("%d", new_rssi));
				if (state.rssi == AltosLib.MISSING)
					setVisible(false);
				else
					setVisible(true);
				rssi = new_rssi;
			}
		}

		public void reset() {
			super.reset();
			rssi = 10000;
		}

		public RSSI (GridBagLayout layout, int x) {
			super (layout, x, "RSSI");
		}
	}

	RSSI rssi;

	class LastPacket extends Value {

		long	last_secs = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			long secs = (System.currentTimeMillis() - state.received_time + 500) / 1000;

			if (secs != last_secs) {
				value.setText(String.format("%d", secs));
				last_secs = secs;
			}
		}

		void reset() {
			super.reset();
			last_secs = -1;
		}

		void disable() {
			value.setText("");
		}

		public LastPacket(GridBagLayout layout, int x) {
			super (layout, x, "Age");
		}
	}

	LastPacket last_packet;

	public void disable_receive() {
		last_packet.disable();
	}

	public void reset () {
		call.reset();
		serial.reset();
		flight.reset();
		rssi.reset();
		last_packet.reset();
	}

	public void font_size_changed(int font_size) {
		call.set_font();
		serial.set_font();
		flight.set_font();
		rssi.set_font();
		last_packet.set_font();
	}

	public void units_changed(boolean imperial_units) {
	}

	public void show (AltosState state, AltosListenerState listener_state) {
		call.show(state, listener_state);
		serial.show(state, listener_state);
		flight.show(state, listener_state);
		rssi.show(state, listener_state);
		last_packet.show(state, listener_state);
	}

	public int height() {
		Dimension d = layout.preferredLayoutSize(this);
		return d.height;
	}

	public TeleGPSStatus() {
		layout = new GridBagLayout();

		setLayout(layout);

		call = new Call(layout, 0);
		serial = new Serial(layout, 1);
		flight = new Flight(layout, 2);
		rssi = new RSSI(layout, 4);
		last_packet = new LastPacket(layout, 5);
	}
}
