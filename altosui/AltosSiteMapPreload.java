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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.lang.Math;
import java.net.URL;
import java.net.URLConnection;
import org.altusmetrum.altosuilib_1.*;

class AltosMapPos extends Box {
	AltosUI		owner;
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

	public AltosMapPos(AltosUI in_owner,
			   String label_value,
			   String[] hemi_names,
			   double default_value) {
		super(BoxLayout.X_AXIS);
		owner = in_owner;
		label = new JLabel(label_value);
		hemi = new JComboBox(hemi_names);
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

class AltosSite {
	String	name;
	double	latitude;
	double	longitude;

	public String toString() {
		return name;
	}

	public AltosSite(String in_name, double in_latitude, double in_longitude) {
		name = in_name;
		latitude = in_latitude;
		longitude = in_longitude;
	}

	public AltosSite(String line) throws ParseException {
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

class AltosSites extends Thread {
	AltosSiteMapPreload	preload;
	URL			url;
	LinkedList<AltosSite>	sites;

	void notify_complete() {
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					preload.set_sites();
				}
			});
	}

	void add(AltosSite site) {
		sites.add(site);
	}

	void add(String line) {
		try {
			add(new AltosSite(line));
		} catch (ParseException pe) {
		}
	}

	public void run() {
		try {
			URLConnection uc = url.openConnection();
			//int length = uc.getContentLength();
			
			InputStreamReader in_stream = new InputStreamReader(uc.getInputStream(), Altos.unicode_set);
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

	public AltosSites(AltosSiteMapPreload in_preload) {
		sites = new LinkedList<AltosSite>();
		preload = in_preload;
		try {
			url = new URL(Altos.launch_sites_url);
		} catch (java.net.MalformedURLException e) {
			notify_complete();
		}
		start();
	}
}

public class AltosSiteMapPreload extends AltosUIDialog implements ActionListener, ItemListener {
	AltosUI		owner;
	AltosSiteMap	map;

	AltosMapPos	lat;
	AltosMapPos	lon;

	final static int	radius = 4;
	final static int	width = (radius * 2 + 1);
	final static int	height = (radius * 2 + 1);

	JProgressBar	pbar;

	AltosSites	sites;
	JLabel		site_list_label;
	JComboBox	site_list;
		
	JToggleButton	load_button;
	boolean		loading;
	JButton		close_button;

	static final String[]	lat_hemi_names = { "N", "S" };
	static final String[]	lon_hemi_names = { "E", "W" };

	class updatePbar implements Runnable {
		int		n;
		String		s;

		public updatePbar(int x, int y, String in_s) {
			n = (x + radius) + (y + radius) * width + 1;
			s = in_s;
		}

		public void run() {
			pbar.setValue(n);
			pbar.setString(s);
			if (n < width * height) {
				pbar.setValue(n);
				pbar.setString(s);
			} else {
				pbar.setValue(0);
				pbar.setString("");
				load_button.setSelected(false);
				loading = false;
			}
		}
	}

	class bgLoad extends Thread {

		AltosSiteMap	map;

		public bgLoad(AltosSiteMap in_map) {
			map = in_map;
		}

		public void run() {
			for (int y = -map.radius; y <= map.radius; y++) {
				for (int x = -map.radius; x <= map.radius; x++) {
					String	pngfile;
					pngfile = map.initMap(new Point(x,y));
					SwingUtilities.invokeLater(new updatePbar(x, y, pngfile));
				}
			}
		}
	}

	public void set_sites() {
		int	i = 1;
		for (AltosSite site : sites.sites) {
			site_list.insertItemAt(site, i);
			i++;
		}
	}

	public void itemStateChanged(ItemEvent e) {
		int		state = e.getStateChange();

		if (state == ItemEvent.SELECTED) {
			Object	o = e.getItem();
			if (o instanceof AltosSite) {
				AltosSite	site = (AltosSite) o;
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
					final double	latitude = lat.get_value();
					final double	longitude = lon.get_value();
					map.setBaseLocation(latitude,longitude);
					map.draw_circle(latitude,longitude);
					loading = true;
					bgLoad thread = new bgLoad(map);
					thread.start();
				} catch (NumberFormatException ne) {
					load_button.setSelected(false);
				}
			}
		}
	}

	public AltosSiteMapPreload(AltosUI in_owner) {
		owner = in_owner;

		Container		pane = getContentPane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		pane.setLayout(new GridBagLayout());

		map = new AltosSiteMap(4);

		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 1;

		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 2;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(map, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(width * height);
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
		c.gridwidth = 2;

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
		
		site_list = new JComboBox(new String[] { "Site List" });
		site_list.addItemListener(this);

		sites = new AltosSites(this);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;

		pane.add(site_list, c);
		
		lat = new AltosMapPos(owner,
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
		
		lon = new AltosMapPos(owner,
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

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}