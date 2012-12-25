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

public class MicroGraph {

	XYPlot		plot;
	JFreeChart	chart;
	ChartPanel	panel;
	NumberAxis	xAxis;
	XYSeries	heightSeries;
	XYSeries	speedSeries;
	XYSeries	accelSeries;

	MicroData	data;

	public JPanel panel() {
		return panel;
	}

	private void addSeries(XYSeries series, int index, String label, String units) {
		XYSeriesCollection	dataset = new XYSeriesCollection(series);
		NumberAxis		axis = new NumberAxis(String.format("%s (%s)", label, units));
		XYItemRenderer		renderer = new XYLineAndShapeRenderer(true, false);

		renderer.setPlot(plot);
		renderer.setBaseToolTipGenerator(new StandardXYToolTipGenerator(String.format("{1}s: {2}%s ({0})", units),
										new java.text.DecimalFormat("0.00"),
										new java.text.DecimalFormat("0.00")));
		plot.setRangeAxis(index, axis);
		plot.setDataset(index, dataset);
		plot.setRenderer(index, renderer);
		plot.mapDatasetToRangeAxis(index, index);
	}
	
	public void setData (MicroData data) {
		heightSeries.clear();
		speedSeries.clear();
		accelSeries.clear();
		for (int i = 0; i < data.pressures.length; i++) {
			double x = data.time(i);
			heightSeries.add(x, data.height(i));
			speedSeries.add(x, data.speed(i));
			accelSeries.add(x, data.acceleration(i));
		}
	}

	public MicroGraph(MicroData data) {

		this.data = data;

		heightSeries = new XYSeries("Height");
		speedSeries = new XYSeries("Speed");
		accelSeries = new XYSeries("Acceleration");

		xAxis = new NumberAxis("Time (s)");
		
		xAxis.setAutoRangeIncludesZero(true);

		plot = new XYPlot();
		plot.setDomainAxis(xAxis);
		plot.setOrientation(PlotOrientation.VERTICAL);
		plot.setDomainPannable(true);
		plot.setRangePannable(true);

		addSeries(heightSeries, 0, "Height", "m");
		addSeries(speedSeries, 1, "Speed", "m/s");
		addSeries(accelSeries, 2, "Acceleration", "m/s²");

		chart = new JFreeChart("Flight", JFreeChart.DEFAULT_TITLE_FONT,
				       plot, true);

		ChartUtilities.applyCurrentTheme(chart);
		panel = new ChartPanel(chart);
		panel.setMouseWheelEnabled(true);
		panel.setPreferredSize(new java.awt.Dimension(800, 500));
	}
}