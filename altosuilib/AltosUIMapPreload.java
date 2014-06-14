/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import java.util.*;
import java.text.*;
import java.lang.Math;
import java.net.URL;
import java.net.URLConnection;
import org.altusmetrum.altoslib_4.*;

class AltosUIMapPos extends Box {
	AltosUIFrame	owner;
	JLabel		label;
	JComboBox	hemi;
	JTextField	deg;
	JLabel		deg_label;
	JTextField	min;
	JLabel		min_label;

	public void set_value(double new_value) {
		double	d, m;
		int	h;

		h = 0;
		if (new_value < 0) {
			h = 1;
			new_value = -new_value;
		}
		d = Math.floor(new_value);
		deg.setText(String.format("%3.0f", d));
		m = (new_value - d) * 60.0;
		min.setText(String.format("%7.4f", m));
		hemi.setSelectedIndex(h);
	}

	public double get_value() throws NumberFormatException {
		int	h = hemi.getSelectedIndex();
		String	d_t = deg.getText();
		String	m_t = min.getText();
		double 	d, m, v;
		try {
			d = Double.parseDouble(d_t);
		} catch (NumberFormatException ne) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Invalid degrees \"%s\"",
								    d_t),
						      "Invalid number",
						      JOptionPane.ERROR_MESSAGE);
			throw ne;
		}
		try {
			if (m_t.equals(""))
				m = 0;
			else
				m = Double.parseDouble(m_t);
		} catch (NumberFormatException ne) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Invalid minutes \"%s\"",
								    m_t),
						      "Invalid number",
						      JOptionPane.ERROR_MESSAGE);
			throw ne;
		}
		v = d + m/60.0;
		if (h == 1)
			v = -v;
		return v;
	}

	public AltosUIMapPos(AltosUIFrame in_owner,
			   String label_value,
			   String[] hemi_names,
			   double default_value) {
		super(BoxLayout.X_AXIS);
		owner = in_owner;
		label = new JLabel(label_value);
		hemi = new JComboBox<String>(hemi_names);
		hemi.setEditable(false);
		deg = new JTextField(5);
		deg.setMinimumSize(deg.getPreferredSize());
		deg.setHorizontalAlignment(JTextField.RIGHT);
		deg_label = new JLabel("°");
		min = new JTextField(9);
		min.setMinimumSize(min.getPreferredSize());
		min_label = new JLabel("'");
		set_value(default_value);
		add(label);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(hemi);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(deg);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(deg_label);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(min);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(min_label);
	}
}

class AltosUISite {
	String	name;
	double	latitude;
	double	longitude;

	public String toString() {
		return name;
	}

	public AltosUISite(String in_name, double in_latitude, double in_longitude) {
		name = in_name;
		latitude = in_latitude;
		longitude = in_longitude;
	}

	public AltosUISite(String line) throws ParseException {
		String[]	elements = line.split(":");

		if (elements.length < 3)
			throw new ParseException(String.format("Invalid site line %s", line), 0);

		name = elements[0];

		try {
			latitude = Double.parseDouble(elements[1]);
			longitude = Double.parseDouble(elements[2]);
		} catch (NumberFormatException ne) {
			throw new ParseException(String.format("Invalid site line %s", line), 0);
		}
	}
}

class AltosUISites extends Thread {
	AltosUIMapPreload	preload;
	URL			url;
	LinkedList<AltosUISite>	sites;

	void notify_complete() {
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					preload.set_sites();
				}
			});
	}

	void add(AltosUISite site) {
		sites.add(site);
	}

	void add(String line) {
		try {
			add(new AltosUISite(line));
		} catch (ParseException pe) {
		}
	}

	public void run() {
		try {
			URLConnection uc = url.openConnection();
			//int length = uc.getContentLength();

			InputStreamReader in_stream = new InputStreamReader(uc.getInputStream(), AltosLib.unicode_set);
			BufferedReader in = new BufferedReader(in_stream);

			for (;;) {
				String line = in.readLine();
				if (line == null)
					break;
				add(line);
			}
		} catch (IOException e) {
		} finally {
			notify_complete();
		}
	}

	public AltosUISites(AltosUIMapPreload in_preload) {
		sites = new LinkedList<AltosUISite>();
		preload = in_preload;
		try {
			url = new URL(AltosLib.launch_sites_url);
		} catch (java.net.MalformedURLException e) {
			notify_complete();
		}
		start();
	}
}

public class AltosUIMapPreload extends AltosUIFrame implements ActionListener, ItemListener, AltosUIMapTileListener {
	AltosUIFrame	owner;
	AltosUIMap	map;
	AltosUIMapCache	cache = new AltosUIMapCache();

	AltosUIMapPos	lat;
	AltosUIMapPos	lon;

	JProgressBar	pbar;
	int		pbar_max;
	int		pbar_cur;

	AltosUISites	sites;
	JLabel		site_list_label;
	JComboBox<AltosUISite>	site_list;

	JToggleButton	load_button;
	boolean		loading;
	JButton		close_button;

	JCheckBox[]	maptypes = new JCheckBox[AltosUIMap.maptype_terrain - AltosUIMap.maptype_hybrid + 1];

	JComboBox<Integer>	min_zoom;
	JComboBox<Integer>	max_zoom;
	JComboBox<Integer>	radius;

	Integer[]		zooms = { -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6 };
	Integer[]		radii = { 1, 2, 3, 4, 5 };

	static final String[]	lat_hemi_names = { "N", "S" };
	static final String[]	lon_hemi_names = { "E", "W" };

	class updatePbar implements Runnable {
		String		s;

		public updatePbar(String in_s) {
			s = in_s;
		}

		public void run() {
			int 	n = ++pbar_cur;

			pbar.setMaximum(pbar_max);
			pbar.setValue(n);
			pbar.setString(s);
		}
	}

	double	latitude, longitude;
	int	min_z;
	int	max_z;
	int	cur_z;
	int	all_types;
	int	cur_type;
	int	r;

	int	tiles_per_layer;
	int	tiles_loaded;
	int	layers_total;
	int	layers_loaded;


	private void do_load() {
		tiles_loaded = 0;
		map.set_zoom(cur_z + AltosUIMapView.default_zoom);
		map.set_maptype(cur_type);
		map.set_load_params(latitude, longitude, r, this);
	}

	private int next_type(int start) {
		int next_type;
		for (next_type = start;
		     next_type <= AltosUIMap.maptype_terrain && (all_types & (1 << next_type)) == 0;
		     next_type++)
			;
		return next_type;
	}

	private void next_load() {
		int next_type = next_type(cur_type + 1);

		if (next_type > AltosUIMap.maptype_terrain) {
			if (cur_z == max_z) {
				return;
			} else {
				cur_z++;
			}
			next_type = next_type(0);
		}
		cur_type = next_type;
		do_load();
	}

	private void start_load() {
		cur_z = min_z;
		int ntype = 0;
		all_types = 0;
		for (int t = AltosUIMap.maptype_hybrid; t <= AltosUIMap.maptype_terrain; t++)
			if (maptypes[t].isSelected()) {
				all_types |= (1 << t);
				ntype++;
			}
		if (ntype == 0) {
			all_types |= (1 << AltosUIMap.maptype_hybrid);
			ntype = 1;
		}

		cur_type = next_type(0);
		tiles_per_layer = (r * 2 + 1) * (r * 2 + 1);
		layers_total = (max_z - min_z + 1) * ntype;
		layers_loaded = 0;
		pbar_max = layers_total * tiles_per_layer;
		pbar_cur = 0;

		map.clear_marks();
		map.add_mark(latitude,longitude, AltosLib.ao_flight_boost);
		do_load();
	}

	/* AltosUIMapTileListener methods */

	public void notify_tile(AltosUIMapTile tile, int status) {
		if (status == AltosUIMapStore.loading)
			return;

		SwingUtilities.invokeLater(new updatePbar(tile.store.file.toString()));
		++tiles_loaded;
		if (tiles_loaded == tiles_per_layer) {
			++layers_loaded;
			if (layers_loaded == layers_total) {
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							pbar.setValue(0);
							pbar.setString("");
							load_button.setSelected(false);
							loading = false;
						}
					});
			} else {
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							next_load();
						}
					});
			}
		}
	}

	public AltosUIMapCache cache() { return cache; }

	public void set_sites() {
		int	i = 1;
		for (AltosUISite site : sites.sites) {
			site_list.insertItemAt(site, i);
			i++;
		}
	}

	public void itemStateChanged(ItemEvent e) {
		int		state = e.getStateChange();

		if (state == ItemEvent.SELECTED) {
			Object	o = e.getItem();
			if (o instanceof AltosUISite) {
				AltosUISite	site = (AltosUISite) o;
				lat.set_value(site.latitude);
				lon.set_value(site.longitude);
			}
		}
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("close"))
			setVisible(false);

		if (cmd.equals("load")) {
			if (!loading) {
				try {
					latitude = lat.get_value();
					longitude = lon.get_value();
					min_z = (Integer) min_zoom.getSelectedItem();
					max_z = (Integer) max_zoom.getSelectedItem();
					if (max_z < min_z)
						max_z = min_z;
					r = (Integer) radius.getSelectedItem();
					loading = true;
				} catch (NumberFormatException ne) {
					load_button.setSelected(false);
				}
				start_load();
			}
		}
	}

	public AltosUIMapPreload(AltosUIFrame in_owner) {
		System.out.printf("start creating preload ui\n");

		owner = in_owner;

		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		setTitle("AltOS Load Maps");

		pane.setLayout(new GridBagLayout());

		map = new AltosUIMap();

		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 1;

		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 10;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(map, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(1);
		pbar.setValue(0);
		pbar.setString("");
		pbar.setStringPainted(true);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 10;

		pane.add(pbar, c);

		site_list_label = new JLabel ("Known Launch Sites:");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;

		pane.add(site_list_label, c);

		site_list = new JComboBox<AltosUISite>(new AltosUISite[] { new AltosUISite("Site List", 0, 0) });
		site_list.addItemListener(this);

		sites = new AltosUISites(this);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;

		pane.add(site_list, c);

		lat = new AltosUIMapPos(owner,
					"Latitude:",
					lat_hemi_names,
					37.167833333);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 0;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(lat, c);

		lon = new AltosUIMapPos(owner,
					"Longitude:",
					lon_hemi_names,
					-97.73975);

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 0;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(lon, c);

		load_button = new JToggleButton("Load Map");
		load_button.addActionListener(this);
		load_button.setActionCommand("load");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(load_button, c);

		close_button = new JButton("Close");
		close_button.addActionListener(this);
		close_button.setActionCommand("close");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(close_button, c);

		JLabel	types_label = new JLabel("Map Types");
		c.gridx = 2;
		c.gridwidth = 2;
		c.gridy = 2;
		pane.add(types_label, c);

		c.gridwidth = 1;

		for (int type = AltosUIMap.maptype_hybrid; type <= AltosUIMap.maptype_terrain; type++) {
			maptypes[type] = new JCheckBox(AltosUIMap.maptype_labels[type],
						       type == AltosUIMap.maptype_hybrid);
			c.gridx = 2 + (type >> 1);
			c.fill = GridBagConstraints.HORIZONTAL;
			c.gridy = (type & 1) + 3;
			pane.add(maptypes[type], c);
		}

		JLabel	min_zoom_label = new JLabel("Minimum Zoom");
		c.gridx = 4;
		c.gridy = 2;
		pane.add(min_zoom_label, c);

		min_zoom = new JComboBox<Integer>(zooms);
		min_zoom.setSelectedItem(zooms[10]);
		min_zoom.setEditable(false);
		c.gridx = 5;
		c.gridy = 2;
		pane.add(min_zoom, c);

		JLabel	max_zoom_label = new JLabel("Maximum Zoom");
		c.gridx = 4;
		c.gridy = 3;
		pane.add(max_zoom_label, c);

		max_zoom = new JComboBox<Integer>(zooms);
		max_zoom.setSelectedItem(zooms[14]);
		max_zoom.setEditable(false);
		c.gridx = 5;
		c.gridy = 3;
		pane.add(max_zoom, c);

		JLabel radius_label = new JLabel("Tile Radius");
		c.gridx = 4;
		c.gridy = 4;
		pane.add(radius_label, c);

		radius = new JComboBox<Integer>(radii);
		radius.setSelectedItem(radii[4]);
		radius.setEditable(true);
		c.gridx = 5;
		c.gridy = 4;
		pane.add(radius, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);

		System.out.printf("done creating preload ui\n");
	}
}
