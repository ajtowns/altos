/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.lang.Math;
import java.awt.geom.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

public class AltosUIMap extends JComponent implements AltosFlightDisplay, AltosUIMapZoomListener {

	static final int px_size = 512;

	static final int maptype_hybrid = 0;
	static final int maptype_roadmap = 1;
	static final int maptype_satellite = 2;
	static final int maptype_terrain = 3;
	static final int maptype_default = maptype_hybrid;

	static final String[] maptype_names = {
		"hybrid",
		"roadmap",
		"satellite",
		"terrain"
	};

	public static final String[] maptype_labels = {
		"Hybrid",
		"Roadmap",
		"Satellite",
		"Terrain"
	};

	public static final Color stateColors[] = {
		Color.WHITE,  // startup
		Color.WHITE,  // idle
		Color.WHITE,  // pad
		Color.RED,    // boost
		Color.PINK,   // fast
		Color.YELLOW, // coast
		Color.CYAN,   // drogue
		Color.BLUE,   // main
		Color.BLACK,  // landed
		Color.BLACK,  // invalid
		Color.CYAN,   // stateless
	};

	public void reset() {
		// nothing
	}

	public void font_size_changed(int font_size) {
		view.set_font();
	}

	public void units_changed(boolean imperial_units) {
		view.set_units();
	}

	JLabel	zoom_label;

	private void set_zoom_label() {
		zoom_label.setText(String.format("Zoom %d", view.zoom() - view.default_zoom));
	}

	public void zoom_changed(int zoom) {
		set_zoom_label();
	}

	public void set_zoom(int zoom) {
		view.set_zoom(zoom);
	}

	public int get_zoom() {
		return view.zoom();
	}

	public void set_maptype(int type) {
		view.set_maptype(type);
		maptype_combo.setSelectedIndex(type);
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		view.show(state, listener_state);
	}

	public void centre(double lat, double lon) {
		view.centre(lat, lon);
	}

	public void centre(AltosState state) {
		if (!state.gps.locked && state.gps.nsat < 4)
			return;
		centre(state.gps.lat, state.gps.lon);
	}

	public void add_mark(double lat, double lon, int state) {
		view.add_mark(lat, lon, state);
	}

	public void clear_marks() {
		view.clear_marks();
	}

	AltosUIMapView	view;

	private GridBagLayout layout = new GridBagLayout();

	JComboBox<String>	maptype_combo;

	public void set_load_params(double lat, double lon, int radius, AltosUIMapTileListener listener) {
		view.set_load_params(lat, lon, radius, listener);
	}

	public boolean all_fetched() {
		return view.all_fetched();
	}

	public static void prefetch_maps(double lat, double lon) {
	}

	public String getName() {
		return "Map";
	}

	public AltosUIMap() {

		view = new AltosUIMapView();

		view.setPreferredSize(new Dimension(500,500));
		view.setVisible(true);
		view.setEnabled(true);
		view.add_zoom_listener(this);

		GridBagLayout	my_layout = new GridBagLayout();

		setLayout(my_layout);

		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.BOTH;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 10;
		c.weightx = 1;
		c.weighty = 1;
		add(view, c);

		int	y = 0;

		zoom_label = new JLabel("", JLabel.CENTER);
		set_zoom_label();

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_label, c);

		JButton zoom_reset = new JButton("0");
		zoom_reset.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(view.default_zoom);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_reset, c);

		JButton zoom_in = new JButton("+");
		zoom_in.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(get_zoom() + 1);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_in, c);

		JButton zoom_out = new JButton("-");
		zoom_out.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					set_zoom(get_zoom() - 1);
				}
			});
		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_out, c);

		maptype_combo = new JComboBox<String>(maptype_labels);

		maptype_combo.setEditable(false);
		maptype_combo.setMaximumRowCount(maptype_combo.getItemCount());
		maptype_combo.addItemListener(new ItemListener() {
				public void itemStateChanged(ItemEvent e) {
					view.set_maptype(maptype_combo.getSelectedIndex());
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(maptype_combo, c);
	}
}
