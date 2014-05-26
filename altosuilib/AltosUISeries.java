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

package org.altusmetrum.altosuilib_2;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

class AltosUITime extends AltosUnits {
	public double value(double v, boolean imperial_units) { return v; }

	public double inverse(double v, boolean imperial_unis) { return v; }

	public String show_units(boolean imperial_units) { return "s"; }

	public String say_units(boolean imperial_units) { return "seconds"; }

	public int show_fraction(int width, boolean imperial_units) {
		if (width < 5)
			return 0;
		return width - 5;
	}

	public int say_fraction(boolean imperial_units) { return 0; }
}

public class AltosUISeries extends XYSeries implements AltosUIGrapher {
	AltosUIAxis	axis;
	String		label;
	AltosUnits	units;
	Color		color;
	XYItemRenderer	renderer;
	int		fetch;
	boolean		enable;

	public void set_units() {
		axis.set_units();
		StandardXYToolTipGenerator	ttg;

		String	time_example = (new AltosUITime()).graph_format(7);
		String  example = units.graph_format(7);

		ttg = new StandardXYToolTipGenerator(String.format("{1}s: {2}%s ({0})",
								   units.show_units()),
						     new java.text.DecimalFormat(time_example),
						     new java.text.DecimalFormat(example));
		renderer.setBaseToolTipGenerator(ttg);
	}

	public void set_enable(boolean enable) {
		if (this.enable != enable) {
			this.enable = enable;
			renderer.setSeriesVisible(0, enable);
			axis.set_enable(enable);
		}
	}

	public void add(AltosUIDataPoint dataPoint) {
		try {
			super.add(dataPoint.x(), units.value(dataPoint.y(fetch)));
		} catch (AltosUIDataMissing dm) {
		}
	}

	public AltosUISeries (String label, int fetch, AltosUnits units, Color color,
			      boolean enable, AltosUIAxis axis) {
		super(label);
		this.label = label;
		this.fetch = fetch;
		this.units = units;
		this.color = color;
		this.enable = enable;
		this.axis = axis;

		axis.ref(this.enable);

		renderer = new XYLineAndShapeRenderer(true, false);
		renderer.setSeriesPaint(0, color);
		renderer.setSeriesStroke(0, new BasicStroke(2, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
		renderer.setSeriesVisible(0, enable);
		set_units();
	}
}
