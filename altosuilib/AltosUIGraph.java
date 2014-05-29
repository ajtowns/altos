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

public class AltosUIGraph implements AltosUnitsListener {

	XYPlot				plot;
	JFreeChart			chart;
	public ChartPanel		panel;
	NumberAxis			xAxis;
	AltosUIEnable			enable;
	ArrayList<AltosUIGrapher>	graphers;
	AltosUIDataSet			dataSet;
	int				axis_index;
	int				series_index;

	static final private Color gridline_color = new Color(0, 0, 0);
	static final private Color border_color = new Color(255, 255, 255);
	static final private Color background_color = new Color(255, 255, 255);

	public JPanel panel() {
		return panel;
	}

	public AltosUIAxis newAxis(String label, AltosUnits units, Color color, int flags) {
		AltosUIAxis axis = new AltosUIAxis(label, units, color, axis_index++, flags);
		plot.setRangeAxis(axis.index, axis);
		return axis;
	}

	public AltosUIAxis newAxis(String label, AltosUnits units, Color color) {
		return newAxis(label, units, color, AltosUIAxis.axis_default);
	}

	public void addSeries(String label, int fetch, AltosUnits units, Color color,
			      boolean enabled, AltosUIAxis axis) {
		AltosUISeries		series = new AltosUISeries(label, fetch, units, color, enabled, axis);
		XYSeriesCollection	dataset = new XYSeriesCollection(series);

		series.renderer.setPlot(plot);
		plot.setDataset(series_index, dataset);
		plot.setRenderer(series_index, series.renderer);
		plot.mapDatasetToRangeAxis(series_index, axis.index);
		if (enable != null)
			enable.add(label, series, enabled);
		this.graphers.add(series);
		series_index++;
	}

	public void addSeries(String label, int fetch, AltosUnits units, Color color) {
		addSeries(label, fetch, units, color, true, newAxis(label, units, color));
	}

	public void addMarker(String label, int fetch, Color color) {
		AltosUIMarker		marker = new AltosUIMarker(fetch, color, plot);
		this.graphers.add(marker);
	}

	public void resetData() {
		for (AltosUIGrapher g : graphers) {
			g.clear();
			g.setNotify(false);
		}
		if (dataSet != null) {
			for (AltosUIDataPoint dataPoint : dataSet.dataPoints())
				for (AltosUIGrapher g : graphers)
					g.add(dataPoint);
		}
		for (AltosUIGrapher g : graphers) {
			g.setNotify(true);
			g.fireSeriesChanged();
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
		resetData();
		if (dataSet != null)
			setName(dataSet.name());
	}

	public AltosUIGraph(AltosUIEnable enable) {

		this.enable = enable;
		this.graphers = new ArrayList<AltosUIGrapher>();
		this.series_index = 0;
		this.axis_index = 0;

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
