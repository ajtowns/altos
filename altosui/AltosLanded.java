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
import java.io.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosLanded extends AltosUIFlightTab implements ActionListener {

	class Bearing extends AltosUIIndicator {
		public void show (AltosState state, AltosListenerState listener_state) {
			if (state.from_pad != null && state.from_pad.bearing != AltosLib.MISSING) {
				show( String.format("%3.0f°", state.from_pad.bearing),
				      state.from_pad.bearing_words(
					      AltosGreatCircle.BEARING_LONG));
			} else {
				show("Missing", "Missing");
			}
		}
		public Bearing (Container container, int y) {
			super (container, y, "Bearing", 2);
		}
	}

	class Distance extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) {
			if (state.from_pad != null)
				return state.from_pad.distance;
			else
				return AltosLib.MISSING;
		}

		public Distance(Container container, int y) {
			super(container, y, AltosConvert.distance, "Ground Distance", 2);
		}
	}

	class Lat extends AltosUIUnitsIndicator {

		public boolean hide (AltosState state, int i) { return state.gps == null || !state.gps.connected; }

		public double value(AltosState state, int i) {
			if (state.gps == null)
				return AltosLib.MISSING;
			if (!state.gps.connected)
				return AltosLib.MISSING;
			return state.gps.lat;
		}

		public Lat (Container container, int y) {
			super (container, y, AltosConvert.latitude, "Latitude", 2);
		}
	}

	class Lon extends AltosUIUnitsIndicator {
		public boolean hide (AltosState state, int i) { return state.gps == null || !state.gps.connected; }

		public double value(AltosState state, int i) {
			if (state.gps == null)
				return AltosLib.MISSING;
			if (!state.gps.connected)
				return AltosLib.MISSING;
			return state.gps.lon;
		}

		public Lon (Container container, int y) {
			super (container, y, AltosConvert.longitude, "Longitude", 2);
		}
	}

	class MaxHeight extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) { return state.max_height(); }

		public MaxHeight (Container container, int y) {
			super (container, y, AltosConvert.height, "Maximum Height", 2);
		}
	}

	class MaxSpeed extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) { return state.max_speed(); }

		public MaxSpeed (Container container, int y) {
			super (container, y, AltosConvert.speed, "Maximum Speed", 2);
		}
	}

	class MaxAccel extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) { return state.max_acceleration(); }

		public MaxAccel (Container container, int y) {
			super (container, y, AltosConvert.speed, "Maximum acceleration", 2);
		}
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
					AltosStateIterable states = null;
					if (filename.endsWith("eeprom")) {
						FileInputStream in = new FileInputStream(file);
						states = new AltosEepromFile(in);
					} else if (filename.endsWith("telem")) {
						FileInputStream in = new FileInputStream(file);
						states = new AltosTelemetryFile(in);
					} else {
						throw new FileNotFoundException(filename);
					}
					try {
						new AltosGraphUI(states, file);
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

	public String getName() {
		return "Landed";
	}

	public AltosLanded(AltosFlightReader in_reader) {
		reader = in_reader;

		/* Elements in descent display */
		add(new Bearing(this, 0));
		add(new Distance(this, 1));
		add(new Lat(this, 2));
		add(new Lon(this, 3));
		add(new MaxHeight(this, 4));
		add(new MaxSpeed(this, 5));
		add(new MaxAccel(this, 6));

		graph = new JButton ("Graph Flight");
		graph.setActionCommand("graph");
		graph.addActionListener(this);
		graph.setEnabled(false);

		GridBagConstraints	c = new GridBagConstraints();

		c.gridx = 1; c.gridy = 7;
		c.insets = new Insets(10, 10, 10, 10);
		c.anchor = GridBagConstraints.WEST;
		c.weightx = 0;
		c.weighty = 0;
		c.fill = GridBagConstraints.VERTICAL;
		add(graph, c);
		addHierarchyListener(this);
	}
}
