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

public class AltosUIGraph implements AltosUnitsListener {

	XYPlot				plot;
	JFreeChart			chart;
	public ChartPanel		panel;
	NumberAxis			xAxis;
	AltosUIEnable			enable;
	ArrayList<AltosUIGrapher>	graphers;
	AltosUIDataSet			dataSet;
	int				index;

	static final private Color gridline_color = new Color(0, 0, 0);
	static final private Color border_color = new Color(255, 255, 255);
	static final private Color background_color = new Color(255, 255, 255);

	public JPanel panel() {
		return panel;
	}

	public void addSeries(String label, int fetch, AltosUnits units, Color color) {
		AltosUISeries		series = new AltosUISeries(label, fetch, units, color);
		XYSeriesCollection	dataset = new XYSeriesCollection(series);

		series.renderer.setPlot(plot);
		plot.setRangeAxis(index, series.axis);
		plot.setDataset(index, dataset);
		plot.setRenderer(index, series.renderer);
		plot.mapDatasetToRangeAxis(index, index);
		if (enable != null)
			enable.add(label, series, true);
		this.graphers.add(series);
		index++;
	}
	
	public void addMarker(String label, int fetch, Color color) {
		AltosUIMarker		marker = new AltosUIMarker(fetch, color, plot);
		if (enable != null)
			enable.add(label, marker, true);
		this.graphers.add(marker);
	}

	public void resetData() {
		for (AltosUIGrapher g : graphers)
			g.clear();
		if (dataSet != null) {
			for (AltosUIDataPoint dataPoint : dataSet.dataPoints())
				for (AltosUIGrapher g : graphers)
					g.add(dataPoint);
		}
	}

	public void units_changed(boolean imperial_units) {
		for (AltosUIGrapher g : graphers)
			g.set_units();
		resetData();
	}

	public void setName (String name) {
		chart.setTitle(name);
	}

	public void setDataSet (AltosUIDataSet dataSet) {
		this.dataSet = dataSet;
		if (dataSet != null)
			setName(dataSet.name());
		resetData();
	}

	public AltosUIGraph(AltosUIEnable enable) {

		this.enable = enable;
		this.graphers = new ArrayList<AltosUIGrapher>();
		this.index = 0;

		xAxis = new NumberAxis("Time (s)");
		
		xAxis.setAutoRangeIncludesZero(true);

		plot = new XYPlot();
		plot.setDomainAxis(xAxis);
		plot.setOrientation(PlotOrientation.VERTICAL);
		plot.setDomainPannable(true);
		plot.setRangePannable(true);

		chart = new JFreeChart("Flight", JFreeChart.DEFAULT_TITLE_FONT,
				       plot, true);

		ChartUtilities.applyCurrentTheme(chart);

		plot.setDomainGridlinePaint(gridline_color);
		plot.setRangeGridlinePaint(gridline_color);
		plot.setBackgroundPaint(background_color);
		plot.setBackgroundAlpha((float) 1);

		chart.setBackgroundPaint(background_color);
		chart.setBorderPaint(border_color);
		panel = new ChartPanel(chart);
		panel.setMouseWheelEnabled(true);
		panel.setPreferredSize(new java.awt.Dimension(800, 500));

		AltosPreferences.register_units_listener(this);
	}
}