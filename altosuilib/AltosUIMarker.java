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

public class AltosUIMarker implements AltosUIGrapher {
	ArrayList<ValueMarker>	markers;
	int			last_id;
	XYPlot			plot;
	boolean			enabled;
	int			fetch;
	Color			color;

	private void remove_markers() {
		for (ValueMarker marker : markers)
			plot.removeDomainMarker(marker);
	}

	private void add_markers() {
		for (ValueMarker marker : markers)
			plot.addDomainMarker(marker);
	}

	public void set_units() {
	}

	public void set_enable(boolean enable) {
		if (enabled == enable)
			return;
		if (enable)
			add_markers();
		else
			remove_markers();
		enabled = enable;
	}

	public void clear() {
		if (enabled)
			remove_markers();
		markers = new ArrayList<ValueMarker>();
	}

	public void add(AltosUIDataPoint dataPoint) {
		try {
			int id = dataPoint.id(fetch);
			if (id < 0)
				return;
			if (id == last_id)
				return;
			ValueMarker marker = new ValueMarker(dataPoint.x());
			marker.setLabel(dataPoint.id_name(fetch));
			marker.setLabelAnchor(RectangleAnchor.TOP_RIGHT);
			marker.setLabelTextAnchor(TextAnchor.TOP_LEFT);
			marker.setPaint(color);
			if (enabled)
				plot.addDomainMarker(marker);
			markers.add(marker);
			last_id = id;
		} catch (AltosUIDataMissing m) {
		}
	}

	public AltosUIMarker (int fetch, Color color, XYPlot plot, boolean enable) {
		markers = new ArrayList<ValueMarker>();
		last_id = -1;
		this.fetch = fetch;
		this.color = color;
		this.plot = plot;
		this.enabled = enable;
	}

	public void setNotify(boolean notify) {
	}

	public void fireSeriesChanged() {
	}

	public AltosUIMarker (int fetch, Color color, XYPlot plot) {
		this(fetch, color, plot, true);
	}
}