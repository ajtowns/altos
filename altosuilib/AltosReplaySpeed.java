/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 * Copyright © 2014 Anthony Towns <aj@erisian.com.au>
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

import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public class AltosReplaySpeed extends JComboBox<Double> {
	static final Double[] defaultSpeeds = {
		0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 15.0
	};

	public AltosReplaySpeed() {
		super(defaultSpeeds);
		setMaximumRowCount(getItemCount());
		setEditable(true);
	}

	public double replayspeed() {
		return ((Double) getSelectedItem()).doubleValue();
	}

	public void set_replayspeed(double speed) {
		setSelectedItem(new Double(speed));
	}
}
