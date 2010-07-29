/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.LinkedBlockingQueue;

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;

public class AltosEepromMonitor extends JDialog {

	JPanel		panel;
	Box		box;
	JLabel		serial_label;
	JLabel		flight_label;
	JLabel		file_label;
	JProgressBar	pbar;
	int		min_state, max_state;

	public AltosEepromMonitor(JFrame owner, int in_min_state, int in_max_state) {
		super (owner, "Download Flight Data");

		box = Box.createVerticalBox();

		serial_label = new JLabel("Serial:");
		box.add(serial_label);
		flight_label = new JLabel("Flight:");
		box.add(flight_label);
		file_label = new JLabel("File:");
		box.add(file_label);

		min_state = in_min_state;
		max_state = in_max_state;
		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum((max_state - min_state) * 100);
		pbar.setValue(0);
		pbar.setString("startup");
		pbar.setStringPainted(true);
		box.add(pbar);

		panel = new JPanel();
		panel.add(box);

		add(panel);

		setMinimumSize(new Dimension(600, 0));
		setContentPane(panel);
		pack();
		setVisible(true);
	}

	public void set_value(String state_name, int in_state, int in_block) {
		int block = in_block;
		int state = in_state;

		if (block > 100)
			block = 100;
		if (state < min_state) state = min_state;
		if (state >= max_state) state = max_state - 1;
		state -= min_state;

		int pos = state * 100 + block;

		System.out.printf ("State %s (%d + %d) = %d\n",
				   state_name, in_state, in_block, pos);

		pbar.setString(state_name);
		pbar.setValue(pos);
	}

	public void set_serial(int serial) {
		serial_label.setText(String.format("Serial: %d", serial));
	}

	public void set_flight(int flight) {
		flight_label.setText(String.format("Flight: %d", flight));
	}

	public void set_file(String file) {
		file_label.setText(String.format("File: %s", file));
	}

	public void done() {
		setVisible(false);
		dispose();
	}
}
