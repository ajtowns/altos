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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public abstract class AltosUIVoltageIndicator extends AltosUIUnitsIndicator {

	abstract public double voltage(AltosState state);
	abstract public double good();

	public double value(AltosState state, int i) {
		return voltage(state);
	}

	double last_voltage = -1;

	public AltosUIVoltageIndicator (Container container, int x, int y, String name, int width) {
		super(container, x, y, AltosConvert.voltage, name, 1, true, width);
	}

	public AltosUIVoltageIndicator (Container container, int y, String name, int width) {
		this(container, 0, y, name, width);
	}
}
