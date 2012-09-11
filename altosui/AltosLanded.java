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
import org.altusmetrum.AltosLib.*;

public class AltosLanded extends JComponent implements AltosFlightDisplay, ActionListener {
	GridBagLayout	layout;

	public class LandedValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, int crc_errors) {}

		void reset() {
			value.setText("");
		}

		void show() {
			label.setVisible(true);
			value.setVisible(true);
		}

		public void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}

		void show(String format, double v) {
			show();
			value.setText(String.format(format, v));
		}


		public LandedValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.insets = new Insets(10, 10, 10, 10);
			c.anchor = GridBagConstraints.WEST;
			c.weightx = 0;
			c.fill = GridBagConstraints.VERTICAL;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 1; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.weightx = 1;
			c.fill = GridBagConstraints.BOTH;
			layout.setConstraints(value, c);
			add(value);
		}
	}

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

	class Lat extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.gps != null && state.gps.connected)
				value.setText(pos(state.gps.lat,"N", "S"));
			else
				value.setText("???");
		}
		public Lat (GridBagLayout layout, int y) {
			super (layout, y, "Latitude");
		}
	}

	Lat lat;

	class Lon extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.gps != null && state.gps.connected)
				value.setText(pos(state.gps.lon,"E", "W"));
			else
				value.setText("???");
		}
		public Lon (GridBagLayout layout, int y) {
			super (layout, y, "Longitude");
		}
	}

	Lon lon;

	class Bearing extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.from_pad != null)
				show("%3.0f°", state.from_pad.bearing);
			else
				value.setText("???");
		}
		public Bearing (GridBagLayout layout, int y) {
			super (layout, y, "Bearing");
		}
	}

	Bearing bearing;

	class Distance extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show();
			if (state.from_pad != null)
				show("%6.0f m", state.from_pad.distance);
			else
				value.setText("???");
		}
		public Distance (GridBagLayout layout, int y) {
			super (layout, y, "Distance");
		}
	}

	Distance distance;

	class Height extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.max_height);
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Height");
		}
	}

	Height	height;

	class Speed extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m/s", state.max_speed);
		}
		public Speed (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Speed");
		}
	}

	Speed	speed;

	class Accel extends LandedValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m/s²", state.max_acceleration);
		}
		public Accel (GridBagLayout layout, int y) {
			super (layout, y, "Maximum Acceleration");
		}
	}

	Accel	accel;

	public void reset() {
		lat.reset();
		lon.reset();
		bearing.reset();
		distance.reset();
		height.reset();
		speed.reset();
		accel.reset();
	}

	public void set_font() {
		lat.set_font();
		lon.set_font();
		bearing.set_font();
		distance.set_font();
		height.set_font();
		speed.set_font();
		accel.set_font();
	}

	public void show(AltosState state, int crc_errors) {
		if (state.gps != null && state.gps.connected) {
			bearing.show(state, crc_errors);
			distance.show(state, crc_errors);
			lat.show(state, crc_errors);
			lon.show(state, crc_errors);
		} else {
			bearing.hide();
			distance.hide();
			lat.hide();
			lon.hide();
		}
		height.show(state, crc_errors);
		speed.show(state, crc_errors);
		accel.show(state, crc_errors);
		if (reader.backing_file() != null)
			graph.setEnabled(true);
	}

	JButton	graph;
	AltosFlightReader reader;

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("graph")) {
			File	file = reader.backing_file();
			if (file != null) {
				String	filename = file.getName();
				try {
					AltosRecordIterable records = null;
					if (filename.endsWith("eeprom")) {
						FileInputStream in = new FileInputStream(file);
						records = new AltosEepromIterable(in);
					} else if (filename.endsWith("telem")) {
						FileInputStream in = new FileInputStream(file);
						records = new AltosTelemetryIterable(in);
					} else {
						throw new FileNotFoundException(filename);
					}
					try {
						new AltosGraphUI(records, filename);
					} catch (InterruptedException ie) {
					} catch (IOException ie) {
					}
				} catch (FileNotFoundException fe) {
					JOptionPane.showMessageDialog(null,
								      fe.getMessage(),
								      "Cannot open file",
								      JOptionPane.ERROR_MESSAGE);
				}
			}
		}
	}

	public AltosLanded(AltosFlightReader in_reader) {
		layout = new GridBagLayout();

		reader = in_reader;

		setLayout(layout);

		/* Elements in descent display */
		bearing = new Bearing(layout, 0);
		distance = new Distance(layout, 1);
		lat = new Lat(layout, 2);
		lon = new Lon(layout, 3);
		height = new Height(layout, 4);
		speed = new Speed(layout, 5);
		accel = new Accel(layout, 6);

		graph = new JButton ("Graph Flight");
		graph.setActionCommand("graph");
		graph.addActionListener(this);
		graph.setEnabled(false);

		GridBagConstraints	c = new GridBagConstraints();

		c.gridx = 0; c.gridy = 7;
		c.insets = new Insets(10, 10, 10, 10);
		c.anchor = GridBagConstraints.WEST;
		c.weightx = 0;
		c.weighty = 0;
		c.fill = GridBagConstraints.VERTICAL;
		add(graph, c);
	}
}
