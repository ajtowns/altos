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

package org.altusmetrum.altosuilib_2;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;

public class AltosEepromMonitorUI extends AltosUIDialog implements AltosEepromMonitor {
	JFrame		owner;
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
	ActionListener	listener;

	public AltosEepromMonitorUI(JFrame owner) {
		super (owner, "Download Flight Data", false);

		this.owner = owner;

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

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(1000);
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
	}

	public void addActionListener(ActionListener l) {
		listener = l;
	}

	public void set_states(int min_state, int max_state) {
		this.min_state = min_state;
		this.max_state = max_state;
	}

	public void set_thread(Thread in_eeprom_thread) {
		final Thread eeprom_thread = in_eeprom_thread;
		cancel.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					if (eeprom_thread != null)
						eeprom_thread.interrupt();
				}
			});
	}

	public void start() {
		setVisible(true);
	}

	private void set_value_internal(String state_name, int state, int state_block, int block) {
		if (state_block > 100)
			state_block = 100;
		if (state < min_state) state = min_state;
		if (state >= max_state) state = max_state - 1;
		state -= min_state;

		int pos = state * 100 + state_block;

		pbar.setString(String.format("block %d state %s", block, state_name));
		pbar.setValue(pos);
	}

	public void set_value(String in_state_name, int in_state, int in_state_block, int in_block) {
		final String state_name = in_state_name;
		final int state = in_state;
		final int state_block = in_state_block;
		final int block = in_block;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_value_internal(state_name, state, state_block, block);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void set_serial_internal(int serial) {
		serial_value.setText(String.format("%d", serial));
	}

	public void set_serial(int in_serial) {
		final int serial = in_serial;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_serial_internal(serial);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void set_flight_internal(int flight) {
		flight_value.setText(String.format("%d", flight));
	}

	public void set_flight(int in_flight) {
		final int flight = in_flight;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_flight_internal(flight);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void set_filename_internal(String filename) {
		file_value.setText(String.format("%s", filename));
	}

	public void set_filename(String in_filename) {
		final String filename = in_filename;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_filename_internal(filename);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void done_internal(boolean success) {
		listener.actionPerformed(new ActionEvent(this,
							 success ? 1 : 0,
							 "download"));
		setVisible(false);
		dispose();
	}

	public void done(boolean in_success) {
		final boolean success = in_success;
		Runnable r = new Runnable() {
				public void run() {
					try {
						done_internal(success);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void reset_internal() {
		set_value_internal("startup",min_state,0, 0);
		set_flight_internal(0);
		set_filename_internal("");
	}

	public void reset() {
		Runnable r = new Runnable() {
				public void run() {
					try {
						reset_internal();
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void show_message_internal(String message, String title, int message_type) {
		int joption_message_type = JOptionPane.ERROR_MESSAGE;

		switch (message_type) {
		case INFO_MESSAGE:
			joption_message_type = JOptionPane.INFORMATION_MESSAGE;
			break;
		case WARNING_MESSAGE:
			joption_message_type = JOptionPane.WARNING_MESSAGE;
			break;
		case ERROR_MESSAGE:
			joption_message_type = JOptionPane.ERROR_MESSAGE;
			break;
		}
		JOptionPane.showMessageDialog(owner,
					      message,
					      title,
					      joption_message_type);
	}

	public void show_message(String in_message, String in_title, int in_message_type) {
		final String message = in_message;
		final String title = in_title;
		final int message_type = in_message_type;
		Runnable r = new Runnable() {
				public void run() {
					try {
						show_message_internal(message, title, message_type);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}
}
