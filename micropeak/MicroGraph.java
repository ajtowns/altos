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
import org.altusmetrum.AltosLib.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

class MicroSeries extends XYSeries {
	NumberAxis	axis;
	String		label;
	String		units;
	Color		color;
	
	String label() {
		return String.format("%s (%s)", label, units);
	}

	void set_units(String units) {
		this.units = units;

		axis.setLabel(label());
	}

	public MicroSeries (String label, String units, Color color) {
		super(label);
		this.label = label;
		this.units = units;
		this.color = color;

		axis = new NumberAxis(label());
		axis.setLabelPaint(color);
		axis.setTickLabelPaint(color);
	}
}

public class MicroGraph implements AltosUnitsListener {

	XYPlot		plot;
	JFreeChart	chart;
	ChartPanel	panel;
	NumberAxis	xAxis;
	MicroSeries	heightSeries;
        MicroSeries	speedSeries;
	MicroSeries	accelSeries;

	static final private Color red = new Color(194,31,31);
	static final private Color green = new Color(31,194,31);
	static final private Color blue = new Color(31,31,194);

	MicroData	data;

	public JPanel panel() {
		return panel;
	}

	private MicroSeries addSeries(int index, String label, String units, Color color) {
		MicroSeries		series = new MicroSeries(label, units, color);
		XYSeriesCollection	dataset = new XYSeriesCollection(series);
		XYItemRenderer		renderer = new XYLineAndShapeRenderer(true, false);

		renderer.setSeriesPaint(0, color);
		renderer.setPlot(plot);
		renderer.setBaseToolTipGenerator(new StandardXYToolTipGenerator(String.format("{1}s: {2}%s ({0})", units),
										new java.text.DecimalFormat("0.00"),
										new java.text.DecimalFormat("0.00")));
		plot.setRangeAxis(index, series.axis);
		plot.setDataset(index, dataset);
		plot.setRenderer(index, renderer);
		plot.mapDatasetToRangeAxis(index, index);
		return series;
	}
	
	public void resetData() {
		heightSeries.clear();
		speedSeries.clear();
		accelSeries.clear();
		for (int i = 0; i < data.pressures.length; i++) {
			double x = data.time(i);
			heightSeries.add(x, AltosConvert.height.value(data.height(i)));
			speedSeries.add(x, AltosConvert.speed.value(data.speed(i)));
			accelSeries.add(x, AltosConvert.accel.value(data.acceleration(i)));
		}
	}

	public void setName (String name) {
		chart.setTitle(name);
	}

	public void setData (MicroData data) {
		this.data = data;
		chart.setTitle(data.name);
		resetData();
	}

	public void units_changed(boolean imperial_units) {
		if (data != null) {
			heightSeries.set_units(AltosConvert.height.show_units());
			speedSeries.set_units(AltosConvert.speed.show_units());
			accelSeries.set_units(AltosConvert.accel.show_units());
			resetData();
		}
	}

	public MicroGraph() {

		xAxis = new NumberAxis("Time (s)");
		
		xAxis.setAutoRangeIncludesZero(true);

		plot = new XYPlot();
		plot.setDomainAxis(xAxis);
		plot.setOrientation(PlotOrientation.VERTICAL);
		plot.setDomainPannable(true);
		plot.setRangePannable(true);

		heightSeries = addSeries(0, "Height", AltosConvert.height.show_units(), red);
		speedSeries = addSeries(1, "Speed", AltosConvert.speed.show_units(), green);
		accelSeries = addSeries(2, "Acceleration", AltosConvert.accel.show_units(), blue);

		chart = new JFreeChart("Flight", JFreeChart.DEFAULT_TITLE_FONT,
				       plot, true);

		ChartUtilities.applyCurrentTheme(chart);
		panel = new ChartPanel(chart);
		panel.setMouseWheelEnabled(true);
		panel.setPreferredSize(new java.awt.Dimension(800, 500));

		AltosPreferences.register_units_listener(this);
	}
}