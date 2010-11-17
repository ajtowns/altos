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

public class AltosEepromMonitor extends JDialog {

	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		flight_label;
	JLabel		file_label;
	JLabel		serial_value;
	JLabel		flight_value;
	JLabel		file_value;
	JButton		cancel;
	JProgressBar	pbar;
	int		min_state, max_state;

	public AltosEepromMonitor(JFrame owner, int in_min_state, int in_max_state) {
		super (owner, "Download Flight Data", false);

		GridBagConstraints c;
		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 0;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		serial_value = new JLabel("");
		pane.add(serial_value, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		flight_label = new JLabel("Flight:");
		pane.add(flight_label, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		flight_value = new JLabel("");
		pane.add(flight_value, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0; c.gridy = 2;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		file_label = new JLabel("File:");
		pane.add(file_label, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1; c.gridy = 2;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		file_value = new JLabel("");
		pane.add(file_value, c);

		min_state = in_min_state;
		max_state = in_max_state;
		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum((max_state - min_state) * 100);
		pbar.setValue(0);
		pbar.setString("startup");
		pbar.setStringPainted(true);
		pbar.setPreferredSize(new Dimension(600, 20));
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 3;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ib = new Insets(4,4,4,4);
		c.insets = ib;
		pane.add(pbar, c);


		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 4;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ic = new Insets(4,4,4,4);
		c.insets = ic;
		pane.add(cancel, c);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}

	public void addActionListener (ActionListener l) {
		cancel.addActionListener(l);
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

		pbar.setString(state_name);
		pbar.setValue(pos);
	}

	public void set_serial(int serial) {
		serial_value.setText(String.format("%d", serial));
	}

	public void set_flight(int flight) {
		flight_value.setText(String.format("%d", flight));
	}

	public void set_file(String file) {
		file_value.setText(String.format("%s", file));
	}

	public void done() {
		setVisible(false);
		dispose();
	}
}
