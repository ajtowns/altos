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
import org.altusmetrum.altoslib_1.*;

public class AltosAscent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	JLabel			cur, max;

	public class AscentStatus {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;

		void show() {
			value.setVisible(true);
			lights.setVisible(true);
			label.setVisible(true);
		}

		void hide() {
			value.setVisible(false);
			lights.setVisible(false);
			label.setVisible(false);
		}

		void show(AltosState state, AltosListenerState listener_state) {}

		void show(String s) {
			show();
			value.setText(s);
		}
		
		void show(AltosUnits units, double v) {
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		void reset() {
			value.setText("");
			lights.set(false);
		}

		void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		public AscentStatus (GridBagLayout layout, int y, String text) {
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

			value = new JTextField(Altos.text_width);
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

	public class AscentValue {
		JLabel		label;
		JTextField	value;
		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
		}

		void show() {
			label.setVisible(true);
			value.setVisible(true);
		}

		void show(String s) {
			show();
			value.setText(s);
		}
		
		void show(AltosUnits units, double v) {
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}
		void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		public AscentValue (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

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

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.gridwidth = 2;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	public class AscentValueHold {
		JLabel		label;
		JTextField	value;
		JTextField	max_value;
		double		max;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
			max_value.setText("");
			max = AltosRecord.MISSING;
		}

		void set_font() {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
			max_value.setFont(Altos.value_font);
		}

		void show(AltosUnits units, double v) {
			if (v == AltosRecord.MISSING) {
				value.setText("Missing");
				max_value.setText("Missing");
			} else {
				value.setText(units.show(8, v));
				if (v > max || max == AltosRecord.MISSING) {
					max_value.setText(units.show(8, v));
					max = v;
				}
			}
		}
		public AscentValueHold (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

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

			value = new JTextField(Altos.text_width);
			value.setFont(Altos.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);

			max_value = new JTextField(Altos.text_width);
			max_value.setFont(Altos.value_font);
			max_value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 3; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(max_value, c);
			add(max_value);
		}
	}


	class Height extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.height, state.height);
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, "Height");
		}
	}

	Height	height;

	class Speed extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.speed, state.speed);
		}
		public Speed (GridBagLayout layout, int y) {
			super (layout, y, "Speed");
		}
	}

	Speed	speed;

	class Accel extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.accel, state.acceleration);
		}
		public Accel (GridBagLayout layout, int y) {
			super (layout, y, "Acceleration");
		}
	}

	Accel	accel;

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

	class Apogee extends AscentStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%4.2f V", state.apogee_voltage);
			lights.set(state.apogee_voltage > 3.7);
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends AscentStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%4.2f V", state.main_voltage);
			lights.set(state.main_voltage > 3.7);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, "Main Igniter Voltage");
		}
	}

	Main main;

	class Lat extends AscentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lat != AltosRecord.MISSING)
				show(pos(state.gps.lat,"N", "S"));
			else
				show("???");
		}
		public Lat (GridBagLayout layout, int y) {
			super (layout, y, "Latitude");
		}
	}

	Lat lat;

	class Lon extends AscentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lon != AltosRecord.MISSING)
				show(pos(state.gps.lon,"E", "W"));
			else
				show("???");
		}
		public Lon (GridBagLayout layout, int y) {
			super (layout, y, "Longitude");
		}
	}

	Lon lon;

	public void reset() {
		lat.reset();
		lon.reset();
		main.reset();
		apogee.reset();
		height.reset();
		speed.reset();
		accel.reset();
	}

	public void set_font() {
		cur.setFont(Altos.label_font);
		max.setFont(Altos.label_font);
		lat.set_font();
		lon.set_font();
		main.set_font();
		apogee.set_font();
		height.set_font();
		speed.set_font();
		accel.set_font();
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		if (state.gps != null && state.gps.connected) {
			lat.show(state, listener_state);
			lon.show(state, listener_state);
		} else {
			lat.hide();
			lon.hide();
		}
		height.show(state, listener_state);
		if (state.main_voltage != AltosRecord.MISSING)
			main.show(state, listener_state);
		else
			main.hide();
		if (state.apogee_voltage != AltosRecord.MISSING)
			apogee.show(state, listener_state);
		else
			apogee.hide();
		speed.show(state, listener_state);
		accel.show(state, listener_state);
	}

	public void labels(GridBagLayout layout, int y) {
		GridBagConstraints	c;

		cur = new JLabel("Current");
		cur.setFont(Altos.label_font);
		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = y;
		c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
		layout.setConstraints(cur, c);
		add(cur);

		max = new JLabel("Maximum");
		max.setFont(Altos.label_font);
		c.gridx = 3; c.gridy = y;
		layout.setConstraints(max, c);
		add(max);
	}

	public String getName() {
		return "Ascent";
	}

	public AltosAscent() {
		layout = new GridBagLayout();

		setLayout(layout);

		/* Elements in ascent display:
		 *
		 * lat
		 * lon
		 * height
		 */
		labels(layout, 0);
		height = new Height(layout, 1);
		speed = new Speed(layout, 2);
		accel = new Accel(layout, 3);
		lat = new Lat(layout, 4);
		lon = new Lon(layout, 5);
		apogee = new Apogee(layout, 6);
		main = new Main(layout, 7);
	}
}
