
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

	boolean fill_map(AltosRecordIterable records) {
		boolean		any_gps = false;
		for (AltosRecord record : records) {
			state = new AltosState(record, state);
			if (state.data.gps != null) {
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

		AltosGraph graph = new AltosGraph(enable);

		graph.setDataSet(new AltosGraphDataSet(records));

		map = new AltosSiteMap();

		pane.add("Flight Graph", graph.panel);
		pane.add("Configure Graph", enable);

		AltosFlightStatsTable stats = new AltosFlightStatsTable(new AltosFlightStats(records));
		pane.add("Flight Statistics", stats);

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
