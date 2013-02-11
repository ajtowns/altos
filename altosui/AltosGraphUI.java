
// Copyright (c) 2010 Anthony Towns
// GPL v2 or later

package altosui;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_1.*;
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

	boolean fill_map(AltosRecordIterable records) {
		boolean		any_gps = false;
		for (AltosRecord record : records) {
			state = new AltosState(record, state);
			if (state.gps.locked && state.gps.nsat >= 4) {
				map.show(state, 0);
				any_gps = true;
			}
		}
		return any_gps;
	}

	AltosGraphUI(AltosRecordIterable records, File file) throws InterruptedException, IOException {
		super(file.getName());
		state = null;

		pane = new JTabbedPane();

		enable = new AltosUIEnable();

		stats = new AltosFlightStats(records);
		graphDataSet = new AltosGraphDataSet(records);

		graph = new AltosGraph(enable, stats, graphDataSet);

		statsTable = new AltosFlightStatsTable(stats);

		map = new AltosSiteMap();

		pane.add("Flight Graph", graph.panel);
		pane.add("Configure Graph", enable);
		pane.add("Flight Statistics", statsTable);

		if (fill_map(records))
			pane.add("Map", map);

		setContentPane (pane);

		pack();

		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		setVisible(true);
		if (state != null)
			map.centre(state);
	}
}
