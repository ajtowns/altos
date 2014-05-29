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
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosAscent extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	JLabel			cur, max;

	public class AscentStatus implements AltosFontListener, AltosUnitsListener {
		JLabel		label;
		JTextField	value;
		AltosLights	lights;
		double		v;
		AltosUnits	units;

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

		void show(double v) {
			this.v = v;
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		void reset() {
			value.setText("");
			lights.set(false);
		}

		public void font_size_changed(int font_size) {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		public void units_changed(boolean imperial_units) {
			if (units != null)
				show(v);
		}

		public AscentStatus (GridBagLayout layout, int y, AltosUnits units, String text) {
			this.units = units;
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

	public abstract class AscentValue implements AltosFontListener, AltosUnitsListener {
		JLabel		label;
		JTextField	value;
		double		v;
		AltosUnits	units;

		abstract void show(AltosState state, AltosListenerState listener_state);

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

		void show(double v) {
			this.v = v;
			show(units.show(8, v));
		}

		void show(String format, double v) {
			show(String.format(format, v));
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}

		public void font_size_changed(int font_size) {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
		}

		public void units_changed(boolean imperial_units) {
			if (units != null)
				show(v);
		}

		public AscentValue (GridBagLayout layout, int y, AltosUnits units, String text) {
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

		public AscentValue (GridBagLayout layout, int y, String text) {
			this(layout, y, null, text);
		}
	}

	public class AscentValueHold implements AltosFontListener, AltosUnitsListener {
		JLabel		label;
		JTextField	value;
		JTextField	max_value;
		double		max;
		AltosUnits	units;
		double		v;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
			max_value.setText("");
			max = AltosLib.MISSING;
		}

		public void font_size_changed(int font_size) {
			label.setFont(Altos.label_font);
			value.setFont(Altos.value_font);
			max_value.setFont(Altos.value_font);
		}

		public void units_changed(boolean imperial_units) {
			show(v);
		}

		void show(double v) {
			this.v = v;
			if (v == AltosLib.MISSING) {
				value.setText("Missing");
			} else {
				value.setText(units.show(8, v));
				if (v > max || max == AltosLib.MISSING)
					max = v;
			}
			if (max == AltosLib.MISSING)
				max_value.setText("Missing");
			else
				max_value.setText(units.show(8, v));
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
			max_value.setVisible(false);
		}

		public AscentValueHold (GridBagLayout layout, int y, AltosUnits units, String text) {
			this.units = units;
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
			show(state.height());
		}
		public Height (GridBagLayout layout, int y) {
			super (layout, y, AltosConvert.height, "Height");
		}
	}

	Height	height;

	class Speed extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(state.speed());
		}
		public Speed (GridBagLayout layout, int y) {
			super (layout, y, AltosConvert.speed, "Speed");
		}
	}

	Speed	speed;

	class Accel extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(state.acceleration());
		}
		public Accel (GridBagLayout layout, int y) {
			super (layout, y, AltosConvert.accel, "Acceleration");
		}
	}

	Accel	accel;

	class Orient extends AscentValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(state.orient());
		}
		public Orient (GridBagLayout layout, int y) {
			super (layout, y, AltosConvert.orient, "Tilt Angle");
		}
	}

	Orient	orient;

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
 			lights.set(state.apogee_voltage >= AltosLib.ao_igniter_good);
		}
		public Apogee (GridBagLayout layout, int y) {
			super(layout, y, null, "Apogee Igniter Voltage");
		}
	}

	Apogee apogee;

	class Main extends AscentStatus {
		void show (AltosState state, AltosListenerState listener_state) {
			show("%4.2f V", state.main_voltage);
			lights.set(state.main_voltage >= AltosLib.ao_igniter_good);
		}
		public Main (GridBagLayout layout, int y) {
			super(layout, y, null, "Main Igniter Voltage");
		}
	}

	Main main;

	class Lat extends AscentValue {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lat != AltosLib.MISSING)
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
			if (state.gps != null && state.gps.connected && state.gps.lon != AltosLib.MISSING)
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
		orient.reset();
	}

	public void font_size_changed(int font_size) {
		cur.setFont(Altos.label_font);
		max.setFont(Altos.label_font);
		lat.font_size_changed(font_size);
		lon.font_size_changed(font_size);
		main.font_size_changed(font_size);
		apogee.font_size_changed(font_size);
		height.font_size_changed(font_size);
		speed.font_size_changed(font_size);
		accel.font_size_changed(font_size);
		orient.font_size_changed(font_size);
	}

	public void units_changed(boolean imperial_units) {
		lat.units_changed(imperial_units);
		lon.units_changed(imperial_units);
		main.units_changed(imperial_units);
		apogee.units_changed(imperial_units);
		height.units_changed(imperial_units);
		speed.units_changed(imperial_units);
		accel.units_changed(imperial_units);
		orient.units_changed(imperial_units);
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
		if (state.main_voltage != AltosLib.MISSING)
			main.show(state, listener_state);
		else
			main.hide();
		if (state.apogee_voltage != AltosLib.MISSING)
			apogee.show(state, listener_state);
		else
			apogee.hide();
		speed.show(state, listener_state);
		accel.show(state, listener_state);
		if (state.orient() != AltosLib.MISSING)
			orient.show(state, listener_state);
		else
			orient.hide();
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
		int y = 0;
		labels(layout, y++);
		height = new Height(layout, y++);
		speed = new Speed(layout, y++);
		accel = new Accel(layout, y++);
		orient = new Orient(layout, y++);
		lat = new Lat(layout, y++);
		lon = new Lon(layout, y++);
		apogee = new Apogee(layout, y++);
		main = new Main(layout, y++);
	}
}
