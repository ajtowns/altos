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
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.MouseInputAdapter;
import javax.imageio.ImageIO;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.lang.Math;
import java.awt.geom.Point2D;
import java.awt.geom.Line2D;

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
		deg = new JTextField("000");
		deg_label = new JLabel("°");
		min = new JTextField("00.0000");
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

public class AltosSiteMapPreload extends JDialog implements ActionListener {
	AltosUI		owner;
	AltosSiteMap	map;

	AltosMapPos	lat;
	AltosMapPos	lon;

	JProgressBar	pbar;

	final static int	radius = 4;
	final static int	width = (radius * 2 + 1);
	final static int	height = (radius * 2 + 1);

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
			System.out.printf("update pbar %d\n", n);
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
		c.gridy = 2;
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
		c.gridy = 2;
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
		c.gridy = 3;
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
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(close_button, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}