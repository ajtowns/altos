
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

	AltosGraphUI(AltosRecordIterable records, String file) throws InterruptedException, IOException {
		pane = new JTabbedPane();

		enable = new AltosUIEnable();

		AltosGraph graph = new AltosGraph(enable);

		graph.setDataSet(new AltosGraphDataSet(records));

		pane.add("Flight Graph", graph.panel);
		pane.add("Configure Graph", enable);

		AltosFlightStatsTable stats = new AltosFlightStatsTable(new AltosFlightStats(records));
		pane.add("Flight Statistics", stats);

		setContentPane (pane);

		pack();

		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		setVisible(true);
	}
}
