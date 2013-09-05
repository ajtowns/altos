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
import org.altusmetrum.altoslib_2.*;
import org.altusmetrum.altosuilib_1.*;

public class MicroDownload extends AltosUIDialog implements Runnable, ActionListener {
	MicroPeak	owner;
	Container	pane;
	AltosDevice	device;
	JButton		cancel;
	MicroData	data;
	MicroSerial	serial;

	private void done_internal() {
		setVisible(false);
		if (data != null) {
			if (data.crc_valid) {
				owner = owner.SetData(data);
				MicroSave save = new MicroSave(owner, data);
				if (save.runDialog())
					owner.SetName(data.name);
			} else {
				JOptionPane.showMessageDialog(owner,
							      "Flight data corrupted",
							      "Download Failed",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
		dispose();
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
			data = new MicroData(serial, device.toShortString());
			serial.close();
		} catch (FileNotFoundException fe) {
		} catch (IOException ioe) {
		} catch (InterruptedException ie) {
		}
		done();
	}

	Thread	serial_thread;

	public void start() {
		try {
			serial = new MicroSerial(device);
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
		c.fill = GridBagConstraints.HORIZONTAL;
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
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		JTextArea help_text = new JTextArea(

			"Locate the photo transistor on the MicroPeak USB adapter\n" +
			"and place the LED on the MicroPeak directly in contact\n" +
			"with it.\n" +
			"\n" +
			"The MicroPeak LED and the MicroPeak USB adapter\n" +
			"photo need to be touching—even a millimeters of space\n" +
			"between them will reduce the light intensity from the LED\n" +
			"enough that the phototransistor will not sense it.\n" +
			"\n" +
			"Turn on the MicroPeak board and adjust the position until\n" +
			"the blue LED on the MicroPeak USB adapter blinks in time\n" +
			"with the orange LED on the MicroPeak board.");

		pane.add(help_text, c);
		y++;

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = y;
		c.gridwidth = 1;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		JLabel waiting_value = new JLabel("Waiting for MicroPeak data...");
		pane.add(waiting_value, c);

		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 1; c.gridy = y;
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
