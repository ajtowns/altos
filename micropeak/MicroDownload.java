/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.micropeak;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_4.*;
import org.altusmetrum.altosuilib_2.*;

public class MicroDownload extends AltosUIDialog implements Runnable, ActionListener, MicroSerialLog, WindowListener {
	MicroPeak	owner;
	Container	pane;
	AltosDevice	device;
	JButton		cancel;
	MicroData	data;
	MicroSerial	serial;
	LinkedList<Integer> log_queue = new LinkedList<Integer>();
	Runnable	log_run;
	JTextArea	serial_log;
	JLabel		status_value;
	int		log_column;

	public void windowActivated(WindowEvent e) {
	}

	public void windowClosed(WindowEvent e) {
		setVisible(false);
		dispose();
	}

	public void windowClosing(WindowEvent e) {
	}

	public void windowDeactivated(WindowEvent e) {
	}

	public void windowDeiconified(WindowEvent e) {
	}

	public void windowIconified(WindowEvent e) {
	}

	public void windowOpened(WindowEvent e) {
	}

	private void done_internal() {
		setVisible(false);
		dispose();

		if (data != null && data.crc_valid) {
			status_value.setText("Received MicroPeak Data");
			owner = owner.SetData(data);
			MicroSave save = new MicroSave(owner, data);
			if (save.runDialog())
				owner.SetName(data.name);
		} else {
			JOptionPane.showMessageDialog(owner,
						      "Download Failed",
						      "Flight data corrupted",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	public void drain_queue() {
		for (;;) {
			int	c;
			synchronized(this) {
				if (log_queue.isEmpty()) {
					log_run = null;
					break;
				}
				c = log_queue.remove();
			}
			if (c == '\r')
				continue;
			if (c == '\0')
				continue;
			String s;
			if (c == '\n') {
				s = "\n";
				log_column = 0;
			} else if (' ' <= c && c <= '~') {
				byte[] bytes = new byte[1];
				bytes[0] = (byte) c;
				s = new String(bytes, AltosLib.unicode_set);
				log_column += 1;
			} else {
				s = String.format("\\0x%02x", c & 0xff);
				log_column += 5;
			}
			serial_log.append(s);
			if (log_column > 40) {
				serial_log.append("\n");
				log_column = 0;
			}
		}
	}

	public void log_char(int c) {
		synchronized(this) {
			log_queue.add(c);
			if (log_run == null) {
				log_run = new Runnable() {
						public void run() {
							drain_queue();
						}
					};
				SwingUtilities.invokeLater(log_run);
			}
		}
	}

	public void done() {
		Runnable r = new Runnable() {
				public void run() {
					try {
						done_internal();
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	public void run() {
		try {
			for (;;) {
				try {
					data = new MicroData(serial, device.toShortString());
					if (data != null && data.crc_valid)
						break;
				} catch (MicroData.NonHexcharException nhe) {
				}
			}
		} catch (FileNotFoundException fe) {
		} catch (IOException ioe) {
		} catch (InterruptedException ie) {
		} catch (MicroData.FileEndedException fee) {
		}
		serial.close();
		done();
	}

	Thread	serial_thread;

	public void start() {
		try {
			serial = new MicroSerial(device);
			serial.set_log(this);
		} catch (FileNotFoundException fe) {
			return;
		}
		serial_thread = new Thread(this);
		serial_thread.start();
	}

	public void actionPerformed(ActionEvent ae) {
		if (serial_thread != null) {
			serial.close();
			serial_thread.interrupt();
		}
		setVisible(false);
	}

	public MicroDownload(MicroPeak owner, AltosDevice device) {
		super (owner, "Download MicroPeak Data", false);

		int y = 0;

		GridBagConstraints c;
		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		this.owner = owner;
		this.device = device;

		pane = getContentPane();
		pane.setLayout(new GridBagLayout());

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		JLabel device_label = new JLabel("Device:");
		pane.add(device_label, c);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = y;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		JLabel device_value = new JLabel(device.toString());
		pane.add(device_value, c);
		y++;

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 0;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		JLabel help_text = new JLabel(
			"<html><i>Turn on the MicroPeak and place the LED inside the<br>" +
			"opening in the top of the MicroPeak USB adapter.<br> " +
			"Verify that the blue LED in the side of the USB adapter<br>" +
			"is blinking along with the orange LED on the MicroPeak.</i></html>");
//		help_text.setEditable(false);

		pane.add(help_text, c);
		y++;

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		status_value = new JLabel("Waiting for MicroPeak data...");
		pane.add(status_value, c);
		y++;

		serial_log = new JTextArea(10, 20);
		serial_log.setEditable(false);

		JScrollPane serial_scroll = new JScrollPane(serial_log);
		serial_scroll.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;

		pane.add(serial_scroll, c);
		y++;

		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.EAST;
		c.gridx = 0; c.gridy = y;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ic = new Insets(4,4,4,4);
		c.insets = ic;
		pane.add(cancel, c);
		y++;

		cancel.addActionListener(this);

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
		start();
	}
}
