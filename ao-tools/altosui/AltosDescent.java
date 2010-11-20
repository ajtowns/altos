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

public class AltosDescent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public abstract class DescentStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		abstract void show(AltosState state, int crc_errors);
		void reset() {
			value.setText("");
			lights.set(false);
		}

		public DescentStatus (GridBagLayout layout, int y, String text) {
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

			value = new JTextField(15);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
            c.gridwidth = 2;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);

		}
	}

	public abstract class DescentValue {
		JLabel		label;
		JTextField	value;

		void reset() {
			value.setText("");
		}

		abstract void show(AltosState state, int crc_errors);

		void show(String format, double v) {
			value.setText(String.format(format, v));
		}

		public DescentValue (GridBagLayout layout, int x, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = x + 1; c.gridy = y;
			c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(17);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 2; c.gridy = y;
            c.gridwidth = 2;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	public abstract class DescentDualValue {
		JLabel		label;
		JTextField	value1;
		JTextField	value2;

		void reset() {
			value1.setText("");
			value2.setText("");
		}

		abstract void show(AltosState state, int crc_errors);
		void show(String v1, String v2) {
            value1.setText(v1);
            value2.setText(v2);
        }
		void show(String f1, double v1, String f2, double v2) {
			value1.setText(String.format(f1, v1));
			value2.setText(String.format(f2, v2));
		}

		public DescentDualValue (GridBagLayout layout, int x, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = x + 1; c.gridy = y;
			c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value1 = new JTextField(17);
			value1.setFont(Altos.value_font);
			value1.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value1, c);
			add(value1);

			value2 = new JTextField(17);
			value2.setFont(Altos.value_font);
			value2.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 3; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value2, c);
			add(value2);
		}
	}

	class Height extends DescentDualValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.height,
				 "%3.0f°", state.elevation);
		}
		public Height (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Height/Elevation");
		}
	}

	Height	height;

	class Speed extends DescentValue {
		void show (AltosState state, int crc_errors) {
			double speed = state.speed;
			if (!state.ascent)
				speed = state.baro_speed;
			show("%6.0f m/s", speed);
		}
		public Speed (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Speed");
		}
	}

	Speed	speed;

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

	class LatLon extends DescentDualValue {
		void show (AltosState state, int crc_errors) {
			if (state.gps != null)
                show(pos(state.gps.lat,"N", "S"),
                        pos(state.gps.lon,"W", "E"));
			else
				show("???", "???");
		}
		public LatLon (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Lat/Long");
		}
	}

	LatLon latlon;

	class Apogee extends DescentStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.2f V", state.drogue_sense));
			lights.set(state.drogue_sense > 3.2);
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends DescentStatus {
		void show (AltosState state, int crc_errors) {
			value.setText(String.format("%4.2f V", state.main_sense));
			lights.set(state.main_sense > 3.2);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class Bearing extends DescentDualValue {
		void show (AltosState state, int crc_errors) {
			if (state.from_pad != null) {
                show( state.from_pad.bearing_words(
                            AltosGreatCircle.BEARING_LONG),
				      String.format("%3.0f°", state.from_pad.bearing));
			} else {
				show("???", "???");
			}
		}
		public Bearing (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Bearing");
        }
	}

	Bearing bearing;

	class Range extends DescentValue {
		void show (AltosState state, int crc_errors) {
			show("%6.0f m", state.range);
		}
		public Range (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Range");
		}
	}

	Range range;

	public void reset() {
		latlon.reset();
		height.reset();
		speed.reset();
		bearing.reset();
		range.reset();
		main.reset();
		apogee.reset();
	}

	public void show(AltosState state, int crc_errors) {
		height.show(state, crc_errors);
		speed.show(state, crc_errors);
		bearing.show(state, crc_errors);
		range.show(state, crc_errors);
		latlon.show(state, crc_errors);
		main.show(state, crc_errors);
		apogee.show(state, crc_errors);
	}

	public AltosDescent() {
		layout = new GridBagLayout();

		setLayout(layout);

		/* Elements in descent display */
        speed = new Speed(layout, 0, 0);    
        height = new Height(layout, 0, 1);
        range = new Range(layout, 0, 2);
        bearing = new Bearing(layout, 0, 3);
        latlon = new LatLon(layout, 0, 4);

		apogee = new Apogee(layout, 5);
		main = new Main(layout, 6);
	}
}
