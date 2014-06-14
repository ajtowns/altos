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

public class AltosDescent extends AltosUIFlightTab {

	class Height extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) { return state.height(); }

		public Height (Container container, int x, int y) {
			super (container, x, y, AltosConvert.height, "Height");
		}
	}

	class Speed extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) { return state.speed(); }

		public Speed (Container container, int x, int y) {
			super (container, x, y, AltosConvert.speed, "Speed");
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

		public Lat (Container container, int x, int y) {
			super (container, x, y, AltosConvert.latitude, "Latitude");
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

		public Lon (Container container, int x, int y) {
			super (container, x, y, AltosConvert.longitude, "Longitude");
		}
	}

	class Apogee extends AltosUIUnitsIndicator {
		public boolean hide(double v) { return v == AltosLib.MISSING; }
		public double value(AltosState state, int i) { return state.apogee_voltage; }
		public double good() { return AltosLib.ao_igniter_good; }

		public Apogee (Container container, int y) {
			super(container, 0, y, 3, AltosConvert.voltage, "Apogee Igniter Voltage", 1, true, 3);
		}
	}

	class Main extends AltosUIUnitsIndicator {
		public boolean hide(double v) { return v == AltosLib.MISSING; }
		public double value(AltosState state, int i) { return state.main_voltage; }
		public double good() { return AltosLib.ao_igniter_good; }

		public Main (Container container, int y) {
			super(container, 0, y, 3, AltosConvert.voltage, "Main Igniter Voltage", 1, true, 3);
		}
	}

	class Distance extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) {
			if (state.from_pad != null)
				return state.from_pad.distance;
			else
				return AltosLib.MISSING;
		}

		public Distance(Container container, int x, int y) {
			super(container, x, y, AltosConvert.distance, "Ground Distance");
		}
	}

	class Range extends AltosUIUnitsIndicator {
		public double value(AltosState state, int i) {
			return state.range;
		}
		public Range (Container container, int x, int y) {
			super (container, x, y, AltosConvert.distance, "Range");
		}
	}

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
		public Bearing (Container container, int x, int y) {
			super (container, x, y, 1, "Bearing", 2, false, 1, 2);
		}
	}

	class Elevation extends AltosUIIndicator {
		public void show (AltosState state, AltosListenerState listener_state) {
			if (state.elevation == AltosLib.MISSING)
				show("Missing");
			else
				show("%3.0f°", state.elevation);
		}
		public Elevation (Container container, int x, int y) {
			super (container, x, y, "Elevation", 1, false, 1);
		}
	}

	public String getName() {
		return "Descent";
	}

	public AltosDescent() {
		/* Elements in descent display */
		add(new Speed(this, 0, 0));
		add(new Height(this, 2, 0));
		add(new Elevation(this, 0, 1));
		add(new Range(this, 2, 1));
		add(new Bearing(this, 0, 2));
		add(new Distance(this, 0, 3));
		add(new Lat(this, 0, 4));
		add(new Lon(this, 2, 4));
		add(new Apogee(this, 5));
		add(new Main(this, 6));
	}
}
