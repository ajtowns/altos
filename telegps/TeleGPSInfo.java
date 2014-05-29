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

package org.altusmetrum.telegps;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class TeleGPSInfo extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;
	JLabel			cur, max;

	public class Info {
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
			lights.set(false);
			value.setText("");
		}

		void set_font() {
			label.setFont(AltosUILib.label_font);
			value.setFont(AltosUILib.value_font);
		}

		public Info (GridBagLayout layout, int y, String text) {
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
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(AltosUILib.text_width);
			value.setFont(AltosUILib.value_font);
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

	public class Value {
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
			label.setFont(AltosUILib.label_font);
			value.setFont(AltosUILib.value_font);
		}

		public Value (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(AltosUILib.text_width);
			value.setFont(AltosUILib.value_font);
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

	public abstract class DualValue {
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
			label.setFont(AltosUILib.label_font);
			value1.setFont(AltosUILib.value_font);
			value2.setFont(AltosUILib.value_font);
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

		public DualValue (GridBagLayout layout, int x, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = x + 1; c.gridy = y;
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value1 = new JTextField(AltosUILib.text_width);
			value1.setFont(AltosUILib.value_font);
			value1.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 2; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value1, c);
			add(value1);

			value2 = new JTextField(AltosUILib.text_width);
			value2.setFont(AltosUILib.value_font);
			value2.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = x + 3; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.gridwidth = 1;
			layout.setConstraints(value2, c);
			add(value2);
		}
	}

	public class ValueHold {
		JLabel		label;
		JTextField	value;
		JTextField	max_value;
		double		max;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
			max_value.setText("");
			max = AltosLib.MISSING;
		}

		void set_font() {
			label.setFont(AltosUILib.label_font);
			value.setFont(AltosUILib.value_font);
			max_value.setFont(AltosUILib.value_font);
		}

		void show(AltosUnits units, double v) {
			if (v == AltosLib.MISSING) {
				value.setText("Missing");
				max_value.setText("Missing");
			} else {
				value.setText(units.show(8, v));
				if (v > max || max == AltosLib.MISSING) {
					max_value.setText(units.show(8, v));
					max = v;
				}
			}
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
			max_value.setVisible(false);
		}

		public ValueHold (GridBagLayout layout, int y, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 1; c.gridy = y;
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField(AltosUILib.text_width);
			value.setFont(AltosUILib.value_font);
			value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 2; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(value, c);
			add(value);

			max_value = new JTextField(AltosUILib.text_width);
			max_value.setFont(AltosUILib.value_font);
			max_value.setHorizontalAlignment(SwingConstants.RIGHT);
			c.gridx = 3; c.gridy = y;
			c.anchor = GridBagConstraints.EAST;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			layout.setConstraints(max_value, c);
			add(max_value);
		}
	}


	class Altitude extends ValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.height, state.altitude());
		}
		public Altitude (GridBagLayout layout, int y) {
			super (layout, y, "Altitude");
		}
	}

	Altitude	altitude;

	class AscentRate extends ValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.speed, state.gps_ascent_rate());
		}
		public AscentRate (GridBagLayout layout, int y) {
			super (layout, y, "Ascent Rate");
		}
	}

	AscentRate	ascent_rate;

	class GroundSpeed extends ValueHold {
		void show (AltosState state, AltosListenerState listener_state) {
			show(AltosConvert.speed, state.gps_ground_speed());
		}
		public GroundSpeed (GridBagLayout layout, int y) {
			super (layout, y, "Ground Speed");
		}
	}

	GroundSpeed	ground_speed;

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

	class Course extends DualValue {
		void show (AltosState state, AltosListenerState listener_state) {
			double	course = state.gps_course();
			if (course != AltosLib.MISSING)
				show( String.format("%3.0f°", course),
				      AltosConvert.bearing_to_words(
					      AltosConvert.BEARING_LONG,
					      course));
		}
		public Course (GridBagLayout layout, int y) {
			super (layout, 0, y, "Course");
		}
	}

	Course		course;

	class Lat extends Value {
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

	class Lon extends Value {
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

	class GPSLocked extends Info {
		void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.gps == null)
				hide();
			else {
				show("%4d sats", state.gps.nsat);
				lights.set(state.gps.locked && state.gps.nsat >= 4);
			}
		}
		public GPSLocked (GridBagLayout layout, int y) {
			super (layout, y, "GPS Locked");
		}
	}

	GPSLocked gps_locked;

	public void reset() {
		lat.reset();
		lon.reset();
		altitude.reset();
		ground_speed.reset();
		ascent_rate.reset();
		course.reset();
		gps_locked.reset();
	}

	public void set_font() {
		cur.setFont(AltosUILib.label_font);
		max.setFont(AltosUILib.label_font);
		lat.set_font();
		lon.set_font();
		altitude.set_font();
		ground_speed.set_font();
		ascent_rate.set_font();
		course.set_font();
		gps_locked.set_font();
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		if (state.gps != null && state.gps.connected) {
			lat.show(state, listener_state);
			lon.show(state, listener_state);
		} else {
			lat.hide();
			lon.hide();
		}
		altitude.show(state, listener_state);
		ground_speed.show(state, listener_state);
		ascent_rate.show(state, listener_state);
		course.show(state, listener_state);
		gps_locked.show(state, listener_state);
	}

	public void labels(GridBagLayout layout, int y) {
		GridBagConstraints	c;

		cur = new JLabel("Current");
		cur.setFont(AltosUILib.label_font);
		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = y;
		c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
		layout.setConstraints(cur, c);
		add(cur);

		max = new JLabel("Maximum");
		max.setFont(AltosUILib.label_font);
		c.gridx = 3; c.gridy = y;
		layout.setConstraints(max, c);
		add(max);
	}

	public String getName() {
		return "Info";
	}

	public TeleGPSInfo() {
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
		altitude = new Altitude(layout, y++);
		ground_speed = new GroundSpeed(layout, y++);
		ascent_rate = new AscentRate(layout, y++);
		course = new Course(layout, y++);
		lat = new Lat(layout, y++);
		lon = new Lon(layout, y++);
		gps_locked = new GPSLocked(layout, y++);
	}
}
