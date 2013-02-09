/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altosuilib_1;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_1.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

public class AltosUISeries extends XYSeries {
	NumberAxis	axis;
	String		label;
	AltosUnits	units;
	Color		color;
	XYItemRenderer	renderer;
	int		fetch;
	
	void set_units() {
		String	units_string = units.show_units();
		axis.setLabel(String.format("%s (%s)", label, units_string));

		StandardXYToolTipGenerator	ttg;

		String  example = units.graph_format(4);

		ttg = new StandardXYToolTipGenerator(String.format("{1}s: {2}%s ({0})", units_string),
						     new java.text.DecimalFormat(example),
						     new java.text.DecimalFormat(example));
		renderer.setBaseToolTipGenerator(ttg);
	}

	void set_enable(boolean enable) {
		renderer.setSeriesVisible(0, enable);
		axis.setVisible(enable);
	}

	public void add(AltosUIDataPoint dataPoint) {
		super.add(dataPoint.x(), dataPoint.y(fetch));
	}

	public void set_axis(NumberAxis axis) {
		this.axis = axis;
	}

	public AltosUISeries (String label, int fetch, AltosUnits units, Color color) {
		super(label);
		this.label = label;
		this.fetch = fetch;
		this.units = units;
		this.color = color;

		axis = new NumberAxis();
		axis.setLabelPaint(color);
		axis.setTickLabelPaint(color);

		renderer = new XYLineAndShapeRenderer(true, false);
		renderer.setSeriesPaint(0, color);
	}
}
