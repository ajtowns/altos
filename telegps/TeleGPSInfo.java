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

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class TeleGPSInfo extends AltosUIFlightTab {

	JLabel				cur, max;

	abstract class Value extends AltosUIUnitsIndicator {
		public abstract void show(AltosState state, AltosListenerState listener_state);

		public Value (Container container, int y, AltosUnits units, String text) {
			super(container, y, units, text, 1, false, 2);
		}
	}

	abstract class DualValue extends AltosUIUnitsIndicator {
		public DualValue (Container container, int y, AltosUnits units, String text) {
			super(container, y, units, text, 2, false, 1);
		}
	}

	abstract class ValueHold extends DualValue {
		public void reset() {
			super.reset();
		}
		public ValueHold (Container container, int y, AltosUnits units, String text) {
			super(container, y, units, text);
		}
	}

	class Altitude extends ValueHold {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.altitude();
			else
				return state.max_altitude();
		}

		public Altitude (Container container, int y) {
			super (container, y, AltosConvert.height, "Altitude");
		}
	}

	class AscentRate extends ValueHold {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.gps_ascent_rate();
			else
				return state.max_gps_ascent_rate();
		}
		public AscentRate (Container container, int y) {
			super (container, y, AltosConvert.speed, "Ascent Rate");
		}
	}

	class GroundSpeed extends ValueHold {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.gps_ground_speed();
			else
				return state.max_gps_ground_speed();
		}
		public GroundSpeed (Container container, int y) {
			super (container, y, AltosConvert.speed, "Ground Speed");
		}
	}

	class Course extends AltosUIIndicator {

		public void show (AltosState state, AltosListenerState listener_state) {
			double	course = state.gps_course();
			if (course == AltosLib.MISSING)
				show("Missing", "Missing");
			else
				show( String.format("%3.0f°", course),
				      AltosConvert.bearing_to_words(
					      AltosConvert.BEARING_LONG,
					      course));
		}
		public Course (Container container, int y) {
			super (container, y, "Course", 2, false, 1);
		}
	}

	class Lat extends AltosUIIndicator {

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

		public void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lat != AltosLib.MISSING)
				show(pos(state.gps.lat,"N", "S"));
			else
				show("Missing");
		}
		public Lat (Container container, int y) {
			super (container, y, "Latitude", 1, false, 2);
		}
	}

	class Lon extends AltosUIIndicator {

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

		public void show (AltosState state, AltosListenerState listener_state) {
			if (state.gps != null && state.gps.connected && state.gps.lon != AltosLib.MISSING)
				show(pos(state.gps.lon,"E", "W"));
			else
				show("Missing");
		}
		public Lon (Container container, int y) {
			super (container, y, "Longitude", 1, false, 2);
		}
	}

	class GPSLocked extends AltosUIIndicator {

		public void show (AltosState state, AltosListenerState listener_state) {
			if (state == null || state.gps == null)
				hide();
			else {
				int soln = state.gps.nsat;
				int nsat = state.gps.cc_gps_sat != null ? state.gps.cc_gps_sat.length : 0;
				show("%4d in solution", soln,
				     "%4d in view", nsat);
				set_lights(state.gps.locked && soln >= 4);
			}
		}
		public GPSLocked (Container container, int y) {
			super (container, y, "GPS Locked", 2, true, 1);
		}
	}

	public void font_size_changed(int font_size) {
		cur.setFont(AltosUILib.label_font);
		max.setFont(AltosUILib.label_font);
		super.font_size_changed(font_size);
	}

	public void labels(Container container, int y) {
		GridBagLayout		layout = (GridBagLayout)(container.getLayout());
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
		return "Location";
	}

	public TeleGPSInfo() {
		int y = 0;
		labels(this, y++);
		add(new Altitude(this, y++));
		add(new GroundSpeed(this, y++));
		add(new AscentRate(this, y++));
		add(new Course(this, y++));
		add(new Lat(this, y++));
		add(new Lon(this, y++));
		add(new GPSLocked(this, y++));
	}
}
