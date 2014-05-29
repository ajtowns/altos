/*
 * Copyright © 2010 Anthony Towns
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 or any later version of the License.
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

package altosui;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.ui.RefineryUtilities;

public class AltosGraphUI extends AltosUIFrame
{
	JTabbedPane		pane;
	AltosGraph		graph;
	AltosUIEnable		enable;
	AltosSiteMap		map;
	AltosState		state;
	AltosGraphDataSet	graphDataSet;
	AltosFlightStats	stats;
	AltosFlightStatsTable	statsTable;
	boolean			has_gps;

	void fill_map(AltosStateIterable states) {
		boolean		any_gps = false;
		for (AltosState state : states) {
			if (state.gps != null && state.gps.locked && state.gps.nsat >= 4) {
				if (map == null)
					map = new AltosSiteMap();
				map.show(state, null);
				has_gps = true;
			}
		}
	}

	AltosGraphUI(AltosStateIterable states, File file) throws InterruptedException, IOException {
		super(file.getName());
		state = null;

		pane = new JTabbedPane();

		enable = new AltosUIEnable();

		stats = new AltosFlightStats(states);
		graphDataSet = new AltosGraphDataSet(states);

		graph = new AltosGraph(enable, stats, graphDataSet);

		statsTable = new AltosFlightStatsTable(stats);

		pane.add("Flight Graph", graph.panel);
		pane.add("Configure Graph", enable);
		pane.add("Flight Statistics", statsTable);

		has_gps = false;
		fill_map(states);
		if (has_gps)
			pane.add("Map", map);

		setContentPane (pane);

		pack();

		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		setVisible(true);
		if (state != null && has_gps)
			map.centre(state);
	}
}
