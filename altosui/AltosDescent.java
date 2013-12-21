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
import org.altusmetrum.altoslib_3.*;

public class AltosDescent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public abstract class DescentStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		abstract void show(AltosState state, AltosListenerState listener_state);

		void show() {
			label.setVisible(true);
			value.setVisible(true);
			lights.setVisible(true);
		}

		void show(String s) {
			show();
			value.setText(s);
		}

		void show(String format, double value) {
			show(String.format(format, value));
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
			lights.setVisible(false);
		}

		void reset() {
			value.setText("");
			lights.set(false);
		}

		void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
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
			c.gridwidth = 3;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 4; c.gridy = y;
			c.gridwidth = 1;
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

		abstract void show(AltosState state, AltosListenerState listener_state);

		void show() {
			label.setVisible(true);
			value.setVisible(true);
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}

		void show(String v) {
			show();
			value.setText(v);
		}

		void show(AltosUnits units, double v) {
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
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
			add(label, c);

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 2; c.gridy = y;
			c.gridwidth = 1;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			add(value, c);
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

		void show() {
			label.setVisible(true);
			value1.setVisible(true);
			value2.setVisible(true);
		}

		void hide() {
			label.setVisible(false);
			value1.setVisible(false);
			value2.setVisible(false);
		}

		void set_font() {
			label.setFont(Altos.label_font);
			value1.setFont(Altos.value_font);
			value2.setFont(Altos.value_font);
		}

		abstract void show(AltosState state, AltosListenerState listener_state);

		void show(String v1, String v2) {
			show();
			value1.setText(v1);
			value2.setText(v2);
		}
		void show(String f1, double v1, String f2, double v2) {
			show();
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

			value1 = new JTextField(Altos.text_width);
			value1.setFont(Altos.value_font);
			value1.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value1, c);
			add(value1);

			value2 = new JTextField(Altos.text_width);
			value2.setFont(Altos.value_font);
			value2.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 4; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.gridwidth = 1;
			layout.setConstraints(value2, c);
			add(value2);
		}
	}

	class Height extends DescentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.height, state.height());
		}
		public Height (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Height");
		}
	}

	Height	height;

	class Speed extends DescentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.speed, state.speed());
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
		return String.format("%s %d° %9.6f", h, deg, min);
	}

	class Lat extends DescentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lat != AltosLib.MISSING)
				show(pos(state.gps.lat,"N", "S"));
			else
				show("???");
		}
		public Lat (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Latitude");
		}
	}

	Lat lat;

	class Lon extends DescentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lon != AltosLib.MISSING)
				show(pos(state.gps.lon,"W", "E"));
			else
				show("???");
		}
		public Lon (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Longitude");
		}
	}

	Lon lon;

	class Distance extends DescentValue {
		void show(AltosState state, AltosListenerState listener_state) {
			if (state.from_pad != null)
				show(AltosConvert.distance, state.from_pad.distance);
			else
				show("???");
		}

		public Distance (GridBagLayout layout, int x, int y) {
			super(layout, x, y, "Ground Distance");
		}
	}

	Distance distance;
		

	class Apogee extends DescentStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%4.2f V", state.apogee_voltage);
			lights.set(state.apogee_voltage >= AltosLib.ao_igniter_good);
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends DescentStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%4.2f V", state.main_voltage);
			lights.set(state.main_voltage >= AltosLib.ao_igniter_good);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class Bearing extends DescentDualValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.from_pad != null) {
				show( String.format("%3.0f°", state.from_pad.bearing),
				      state.from_pad.bearing_words(
					      AltosGreatCircle.BEARING_LONG));
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
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.distance, state.range);
		}
		public Range (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Range");
		}
	}

	Range range;

	class Elevation extends DescentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%3.0f°", state.elevation);
		}
		public Elevation (GridBagLayout layout, int x, int y) {
			super (layout, x, y, "Elevation");
		}
	}

	Elevation elevation;

	public void reset() {
		lat.reset();
		lon.reset();
		height.reset();
		speed.reset();
		bearing.reset();
		range.reset();
		distance.reset();
		elevation.reset();
		main.reset();
		apogee.reset();
	}

	public void set_font() {
		lat.set_font();
		lon.set_font();
		height.set_font();
		speed.set_font();
		bearing.set_font();
		range.set_font();
		distance.set_font();
		elevation.set_font();
		main.set_font();
		apogee.set_font();
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		height.show(state, listener_state);
		speed.show(state, listener_state);
		if (state.gps != null && state.gps.connected) {
			bearing.show(state, listener_state);
			range.show(state, listener_state);
			distance.show(state, listener_state);
			elevation.show(state, listener_state);
			lat.show(state, listener_state);
			lon.show(state, listener_state);
		} else {
			bearing.hide();
			range.hide();
			distance.hide();
			elevation.hide();
			lat.hide();
			lon.hide();
		}
		if (state.main_voltage != AltosLib.MISSING)
			main.show(state, listener_state);
		else
			main.hide();
		if (state.apogee_voltage != AltosLib.MISSING)
			apogee.show(state, listener_state);
		else
			apogee.hide();
	}

	public String getName() {
		return "Descent";
	}

	public AltosDescent() {
		layout = new GridBagLayout();

		setLayout(layout);

		/* Elements in descent display */
		speed = new Speed(layout, 0, 0);
		height = new Height(layout, 2, 0);
		elevation = new Elevation(layout, 0, 1);
		range = new Range(layout, 2, 1);
		bearing = new Bearing(layout, 0, 2);
		distance = new Distance(layout, 0, 3);
		lat = new Lat(layout, 0, 4);
		lon = new Lon(layout, 2, 4);

		apogee = new Apogee(layout, 5);
		main = new Main(layout, 6);
	}
}
