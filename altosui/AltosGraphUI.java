
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_2.*;
import org.altusmetrum.altosuilib_1.*;

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
