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

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosAscent extends AltosUIFlightTab {
	JLabel	cur, max;

	class Height extends AltosUIUnitsIndicator {

		public double value(AltosState state, int i) {
			if (i == 0)
				return state.height();
			else
				return state.max_height();
		}

		public Height(Container container, int y) {
			super(container, y, AltosConvert.height, "Height", 2, false, 1);
		}
	}

	class Speed extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.speed();
			else
				return state.max_speed();
		}

		public Speed(Container container, int y) {
			super(container, y, AltosConvert.speed, "Speed", 2, false, 1);
		}
	}

	class Accel extends AltosUIUnitsIndicator {
		public boolean hide(double v) { return v == AltosLib.MISSING; }

		public double value(AltosState state, int i) {
			if (i == 0)
				return state.acceleration();
			else
				return state.max_acceleration();
		}

		public Accel(Container container, int y) {
			super(container, y, AltosConvert.accel, "Acceleration", 2, false, 1);
		}
	}

	class Orient extends AltosUIUnitsIndicator {

		public boolean hide(double v) { return v == AltosLib.MISSING; }

		public double value(AltosState state, int i) {
			if (i == 0)
				return state.orient();
			else
				return state.max_orient();
		}

		public Orient(Container container, int y) {
			super(container, y, AltosConvert.orient, "Tilt Angle", 2, false, 1);
		}

	}

	class Apogee extends AltosUIUnitsIndicator {

		public double value(AltosState state, int i) {
			return state.apogee_voltage;
		}

		public boolean good(double v) { return v >= AltosLib.ao_igniter_good; }
		public boolean hide(double v) { return v == AltosLib.MISSING; }

		public Apogee (Container container, int y) {
			super(container, y, AltosConvert.voltage, "Apogee Igniter Voltage", 1, true, 2);
		}
	}

	class Main extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) {
			return state.main_voltage;
		}

		public boolean good(double v) { return v >= AltosLib.ao_igniter_good; }
		public boolean hide(double v) { return v == AltosLib.MISSING; }

		public Main (Container container, int y) {
			super(container, y, AltosConvert.voltage, "Main Igniter Voltage", 1, true, 2);
		}
	}

	class Lat extends AltosUIUnitsIndicator {

		public boolean hide(AltosState state, int i) {
			return state.gps == null || !state.gps.connected;
		}

		public double value(AltosState state, int i) {
			if (state.gps == null)
				return AltosLib.MISSING;
			if (!state.gps.connected)
				return AltosLib.MISSING;
			return state.gps.lat;
		}

		Lat (Container container, int y) {
			super (container, y, AltosConvert.latitude, "Latitude", 1, false, 2);
		}
	}

	class Lon extends AltosUIUnitsIndicator {

		public boolean hide(AltosState state, int i) {
			return state.gps == null || !state.gps.connected;
		}

		public double value(AltosState state, int i) {
			if (state.gps == null)
				return AltosLib.MISSING;
			if (!state.gps.connected)
				return AltosLib.MISSING;
			return state.gps.lon;
		}

		Lon (Container container, int y) {
			super (container, y, AltosConvert.longitude, "Longitude", 1, false, 2);
		}
	}

	public void font_size_changed(int font_size) {
		super.font_size_changed(font_size);
		cur.setFont(AltosUILib.label_font);
		max.setFont(AltosUILib.label_font);
	}

	public void labels(GridBagLayout layout, int y) {
		GridBagConstraints	c;

		cur = new JLabel("Current");
		cur.setFont(AltosUILib.label_font);
		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = y;
		c.insets = new Insets(Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad, Altos.tab_elt_pad);
		layout.setConstraints(cur, c);
		add(cur);

		max = new JLabel("Maximum");
		max.setFont(AltosUILib.label_font);
		c.gridx = 3; c.gridy = y;
		layout.setConstraints(max, c);
		add(max);
	}

	public String getName() {
		return "Ascent";
	}

	public AltosAscent() {
		int y = 0;
		labels(layout, y++);
		add(new Height(this, y++));
		add(new Speed(this, y++));
		add(new Accel(this, y++));
		add(new Orient(this, y++));
		add(new Lat(this, y++));
		add(new Lon(this, y++));
		add(new Apogee(this, y++));
		add(new Main(this, y++));
	}
}
