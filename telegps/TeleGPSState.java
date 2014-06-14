/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

public class TeleGPSState extends AltosUIFlightTab {

	JLabel	cur, max;

	abstract class Value extends AltosUIUnitsIndicator {
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
		public ValueHold (Container container, int y, AltosUnits units, String text) {
			super(container, y, units, text);
		}
	}

	class Height extends ValueHold {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.height();
			else
				return state.max_height();
		}

		public Height(Container container, int y) {
			super(container, y, AltosConvert.height, "Height");
		}
	}

	class Speed extends ValueHold {
		public double value(AltosState state, int i) {
			if (i == 0)
				return state.gps_speed();
			else
				return state.max_gps_speed();
		}

		public Speed(Container container, int y) {
			super(container, y, AltosConvert.speed, "Speed");
		}
	}

	class Distance extends Value {
		public double value(AltosState state, int i) {
			if (state.from_pad != null)
				return state.from_pad.distance;
			else
				return AltosLib.MISSING;
		}

		public Distance(Container container, int y) {
			super(container, y, AltosConvert.distance, "Distance");
		}
	}

	class Range extends Value {
		public double value(AltosState state, int i) {
			return state.range;
		}
		public Range (Container container, int y) {
			super (container, y, AltosConvert.distance, "Range");
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
		public Bearing (Container container, int y) {
			super (container, y, "Bearing", 2, false, 1);
		}
	}

	class Elevation extends AltosUIIndicator {
		public void show (AltosState state, AltosListenerState listener_state) {
			if (state.elevation == AltosLib.MISSING)
				show("Missing");
			else
				show("%3.0f°", state.elevation);
		}
		public Elevation (Container container, int y) {
			super (container, y, "Elevation", 1, false, 2);
		}
	}

	class FirmwareVersion extends AltosUIIndicator {
		public void show(AltosState state, AltosListenerState listener_state) {
			if (state.firmware_version == null)
				show("Missing");
			else
				show(state.firmware_version);
		}

		public FirmwareVersion(Container container, int y) {
			super(container, y, "Firmware Version", 1, false, 2);
		}
	}

	class FlightLogMax extends AltosUIIndicator {
		public void show(AltosState state, AltosListenerState listener_state) {
			if (state.flight_log_max == AltosLib.MISSING)
				show("Missing");
			else
				show(String.format("%dkB", state.flight_log_max));
		}

		public FlightLogMax(Container container, int y) {
			super(container, y, "Flight Log Storage", 1, false, 2);
		}
	}

	class BatteryVoltage extends AltosUIVoltageIndicator {
		public double voltage(AltosState state) {
			return state.battery_voltage;
		}

		public double good() {
			return AltosLib.ao_battery_good;
		}

		public BatteryVoltage(Container container, int y) {
			super(container, y, "Battery Voltage", 2);
		}
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

	public void font_size_changed(int font_size) {
		cur.setFont(AltosUILib.label_font);
		max.setFont(AltosUILib.label_font);
		super.font_size_changed(font_size);
	}

	public String getName() {
		return "Status";
	}

	public TeleGPSState() {
		int y = 0;
		labels(this, y++);
		add(new Height(this, y++));
		add(new Speed(this, y++));
		add(new Distance(this, y++));
		add(new Range(this, y++));
		add(new Bearing(this, y++));
		add(new Elevation(this, y++));
		add(new FirmwareVersion(this, y++));
		add(new FlightLogMax(this, y++));
		add(new BatteryVoltage(this, y++));
	}
}
