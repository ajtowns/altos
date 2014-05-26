/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.micropeak;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

public class MicroGraph extends AltosUIGraph {

	static final private Color height_color = new Color(194,31,31);
	static final private Color speed_color = new Color(31,194,31);
	static final private Color accel_color = new Color(31,31,194);
	static final private Color state_color = new Color(3,3,3);

	public MicroGraph(AltosUIEnable enable) {
		super(enable);

		addSeries("Height", MicroDataPoint.data_height, AltosConvert.height, height_color);
		addSeries("Speed", MicroDataPoint.data_speed, AltosConvert.speed, speed_color);
		addSeries("Acceleration", MicroDataPoint.data_accel, AltosConvert.accel, accel_color);
		addMarker("State", MicroDataPoint.data_state, state_color);
	}
}