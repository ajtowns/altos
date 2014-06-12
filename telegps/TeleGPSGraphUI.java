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

package org.altusmetrum.telegps;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.ui.RefineryUtilities;

public class TeleGPSGraphUI extends AltosUIFrame
{
	JTabbedPane		pane;
	AltosGraph		graph;
	AltosUIEnable		enable;
	AltosUIMap		map;
	AltosState		state;
	AltosFlightStats	stats;
	AltosGraphDataSet	graphDataSet;
	AltosFlightStatsTable	statsTable;

	void fill_map(AltosStateIterable states) {
		for (AltosState state : states) {
			if (state.gps != null && state.gps.locked && state.gps.nsat >= 4)
				map.show(state, null);
		}
	}

	private void close() {
		setVisible(false);
		dispose();
		TeleGPS.subtract_window();
	}

	TeleGPSGraphUI(AltosStateIterable states, File file) throws InterruptedException, IOException {
		super(file.getName());
		state = null;

		pane = new JTabbedPane();

		enable = new AltosUIEnable();
		stats = new AltosFlightStats(states);
		graphDataSet = new AltosGraphDataSet(states);
		graph = new AltosGraph(enable, stats, graphDataSet);
		statsTable = new AltosFlightStatsTable(stats);

		map = new AltosUIMap();

		pane.add("Graph", graph.panel);
		pane.add("Configure Graph", enable);
		pane.add("Statistics", statsTable);
		fill_map(states);
		pane.add("Map", map);

		setContentPane (pane);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					close();
				}
			});

		pack();

		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);

		TeleGPS.add_window();

		setVisible(true);

		if (state != null)
			map.centre(state);

	}
}
