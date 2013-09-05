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
import java.util.concurrent.*;
import org.altusmetrum.altoslib_2.*;
import org.altusmetrum.altosuilib_1.*;

public class AltosFlightUI extends AltosUIFrame implements AltosFlightDisplay, AltosFontListener {
	AltosVoice		voice;
	AltosFlightReader	reader;
	AltosDisplayThread	thread;

	JTabbedPane	pane;

	AltosPad	pad;
	AltosAscent	ascent;
	AltosDescent	descent;
	AltosLanded	landed;
	AltosCompanionInfo	companion;
	AltosSiteMap    sitemap;
	boolean		has_map;
	boolean		has_companion;
	boolean		has_state;

	private AltosFlightStatus flightStatus;
	private AltosInfoTable flightInfo;

	boolean exit_on_close = false;

	JComponent cur_tab = null;
	JComponent which_tab(AltosState state) {
		if (state.state < Altos.ao_flight_boost)
			return pad;
		if (state.state <= Altos.ao_flight_coast)
			return ascent;
		if (state.state <= Altos.ao_flight_main)
			return descent;
		return landed;
	}

	void stop_display() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
	}

	void disconnect() {
		stop_display();
	}

	public void reset() {
		pad.reset();
		ascent.reset();
		descent.reset();
		landed.reset();
		flightInfo.clear();
		sitemap.reset();
	}

	public void set_font() {
		pad.set_font();
		ascent.set_font();
		descent.set_font();
		landed.set_font();
		flightStatus.set_font();
		flightInfo.set_font();
		sitemap.set_font();
		companion.set_font();
	}

	public void font_size_changed(int font_size) {
		set_font();
	}


	AltosFlightStatusUpdate	status_update;

	public void show(AltosState state, AltosListenerState listener_state) {
		status_update.saved_state = state;

		if (state == null)
			state = new AltosState();

		pad.show(state, listener_state);

		if (state.state != Altos.ao_flight_startup) {
			if (!has_state) {
				pane.setTitleAt(0, "Launch Pad");
				pane.add(ascent, 1);
				pane.add(descent, 2);
				pane.add(landed, 3);
				has_state = true;
			}
		}

		ascent.show(state, listener_state);
		descent.show(state, listener_state);
		landed.show(state, listener_state);

		JComponent tab = which_tab(state);
		if (tab != cur_tab) {
			if (cur_tab == pane.getSelectedComponent()) {
				pane.setSelectedComponent(tab);
			}
			cur_tab = tab;
		}
		flightStatus.show(state, listener_state);
		flightInfo.show(state, listener_state);

		if (state.companion != null) {
			if (!has_companion) {
				pane.add("Companion", companion);
				has_companion= true;
			}
			companion.show(state, listener_state);
		} else {
			if (has_companion) {
				pane.remove(companion);
				has_companion = false;
			}
		}
		if (state.gps != null && state.gps.connected) {
			if (!has_map) {
				pane.add("Site Map", sitemap);
				has_map = true;
			}
			sitemap.show(state, listener_state);
		} else {
			if (has_map) {
				pane.remove(sitemap);
				has_map = false;
			}
		}
	}

	public void set_exit_on_close() {
		exit_on_close = true;
	}

	Container	bag;
	AltosFreqList	frequencies;
	JComboBox	telemetries;
	JLabel		telemetry;

	ActionListener	show_timer;

	public AltosFlightUI(AltosVoice in_voice, AltosFlightReader in_reader, final int serial) {
		AltosUIPreferences.set_component(this);

		voice = in_voice;
		reader = in_reader;

		bag = getContentPane();
		bag.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();

		setTitle(String.format("AltOS %s", reader.name));

		/* Stick channel selector at top of table for telemetry monitoring */
		if (serial >= 0) {
			// Channel menu
			frequencies = new AltosFreqList(AltosUIPreferences.frequency(serial));
			frequencies.set_product("Monitor");
			frequencies.set_serial(serial);
			frequencies.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						double frequency = frequencies.frequency();
						try {
							reader.set_frequency(frequency);
						} catch (TimeoutException te) {
						} catch (InterruptedException ie) {
						}
						reader.save_frequency();
					}
			});
			c.gridx = 0;
			c.gridy = 0;
			c.weightx = 0;
			c.weighty = 0;
			c.insets = new Insets(3, 3, 3, 3);
			c.fill = GridBagConstraints.NONE;
			c.anchor = GridBagConstraints.WEST;
			bag.add (frequencies, c);

			// Telemetry format menu
			if (reader.supports_telemetry(Altos.ao_telemetry_standard)) {
				telemetries = new JComboBox();
				for (int i = 1; i <= Altos.ao_telemetry_max; i++) 
					telemetries.addItem(Altos.telemetry_name(i));
				int telemetry = AltosPreferences.telemetry(serial);
				if (telemetry <= Altos.ao_telemetry_off ||
				    telemetry > Altos.ao_telemetry_max)
					telemetry = Altos.ao_telemetry_standard;
				telemetries.setSelectedIndex(telemetry - 1);
				telemetries.setMaximumRowCount(Altos.ao_telemetry_max);
				telemetries.setPreferredSize(null);
				telemetries.revalidate();
				telemetries.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							int telemetry = telemetries.getSelectedIndex() + 1;
							reader.set_telemetry(telemetry);
							reader.save_telemetry();
						}
					});
				c.gridx = 1;
				c.gridy = 0;
				c.weightx = 0;
				c.weighty = 0;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.WEST;
				bag.add (telemetries, c);
				c.insets = new Insets(0, 0, 0, 0);
			} else {
				String	version;

				if (reader.supports_telemetry(Altos.ao_telemetry_0_9))
					version = "Telemetry: 0.9";
				else if (reader.supports_telemetry(Altos.ao_telemetry_0_8))
					version = "Telemetry: 0.8";
				else
					version = "Telemetry: None";

				telemetry = new JLabel(version);
				c.gridx = 1;
				c.gridy = 0;
				c.weightx = 0;
				c.weighty = 0;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.WEST;
				bag.add (telemetry, c);
				c.insets = new Insets(0, 0, 0, 0);
			}
		}

		/* Flight status is always visible */
		flightStatus = new AltosFlightStatus();
		c.gridx = 0;
		c.gridy = 1;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridwidth = 2;
		bag.add(flightStatus, c);
		c.gridwidth = 1;

		/* The rest of the window uses a tabbed pane to
		 * show one of the alternate data views
		 */
		pane = new JTabbedPane();

		pad = new AltosPad();
		pane.add("Status", pad);

		ascent = new AltosAscent();
		descent = new AltosDescent();
		landed = new AltosLanded(reader);

		flightInfo = new AltosInfoTable();
		pane.add("Table", new JScrollPane(flightInfo));

		companion = new AltosCompanionInfo();
		has_companion = false;
		has_state = false;

		sitemap = new AltosSiteMap();
		has_map = false;

		/* Make the tabbed pane use the rest of the window space */
		c.gridx = 0;
		c.gridy = 2;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.gridwidth = 2;
		bag.add(pane, c);

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		AltosUIPreferences.register_font_listener(this);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					disconnect();
					setVisible(false);
					dispose();
					AltosUIPreferences.unregister_font_listener(AltosFlightUI.this);
					if (exit_on_close)
						System.exit(0);
				}
			});

		pack();
		setVisible(true);

		thread = new AltosDisplayThread(this, voice, this, reader);

		status_update = new AltosFlightStatusUpdate(flightStatus);

		new javax.swing.Timer(100, status_update).start();

		thread.start();
	}

	public AltosFlightUI (AltosVoice in_voice, AltosFlightReader in_reader) {
		this(in_voice, in_reader, -1);
	}
}
