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
import java.util.concurrent.*;
import org.altusmetrum.AltosLib.*;

public class AltosIdleMonitorUI extends AltosFrame implements AltosFlightDisplay, AltosFontListener, AltosIdleMonitorListener {
	AltosDevice		device;
	JTabbedPane		pane;
	AltosPad		pad;
	AltosInfoTable		flightInfo;
	AltosFlightStatus	flightStatus;
	AltosIdleMonitor	thread;
	int			serial;
	boolean			remote;

	void stop_display() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
	}

	void disconnect() {
		stop_display();
	}

	public void reset() {
		pad.reset();
		flightInfo.clear();
	}

	public void set_font() {
		pad.set_font();
		flightInfo.set_font();
	}

	public void font_size_changed(int font_size) {
		set_font();
	}

	AltosFlightStatusUpdate	status_update;

	public void show(AltosState state, int crc_errors) {
		status_update.saved_state = state;
		try {
			pad.show(state, crc_errors);
			flightStatus.show(state, crc_errors);
			flightInfo.show(state, crc_errors);
		} catch (Exception e) {
			System.out.print("Show exception" + e);
		}
	}

	public void update(final AltosState state) {
		Runnable r = new Runnable() {
				public void run() {
					show(state, 0);
				}
			};
		SwingUtilities.invokeLater(r);
	}

	Container	bag;
	AltosFreqList	frequencies;

	public AltosIdleMonitorUI(JFrame in_owner)
		throws FileNotFoundException, AltosSerialInUseException, TimeoutException, InterruptedException {

		device = AltosDeviceDialog.show(in_owner, Altos.product_any);
		remote = false;
		if (!device.matchProduct(Altos.product_altimeter))
			remote = true;

		serial = device.getSerial();
		bag = getContentPane();
		bag.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();

		java.net.URL imgURL = AltosUI.class.getResource("/altus-metrum-16x16.jpg");
		if (imgURL != null)
			setIconImage(new ImageIcon(imgURL).getImage());

		setTitle(String.format("AltOS %s", device.toShortString()));

		/* Stick frequency selector at top of table for telemetry monitoring */
		if (remote && serial >= 0) {
			// Frequency menu
			frequencies = new AltosFreqList(AltosUIPreferences.frequency(serial));
			frequencies.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						double frequency = frequencies.frequency();
						thread.set_frequency(frequency);
						AltosUIPreferences.set_frequency(device.getSerial(),
									       frequency);
					}
			});
			c.gridx = 0;
			c.gridy = 0;
			c.insets = new Insets(3, 3, 3, 3);
			c.anchor = GridBagConstraints.WEST;
			bag.add (frequencies, c);
		}


		/* Flight status is always visible */
		flightStatus = new AltosFlightStatus();
		c.gridx = 0;
		c.gridy = 1;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridwidth = 2;
		bag.add(flightStatus, c);
		c.gridwidth = 1;

		/* The rest of the window uses a tabbed pane to
		 * show one of the alternate data views
		 */
		pane = new JTabbedPane();

		pad = new AltosPad();
		pane.add("Launch Pad", pad);

		flightInfo = new AltosInfoTable();
		pane.add("Table", new JScrollPane(flightInfo));

		/* Make the tabbed pane use the rest of the window space */
		c.gridx = 0;
		c.gridy = 2;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.gridwidth = 2;
		bag.add(pane, c);

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		AltosUIPreferences.register_font_listener(this);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					disconnect();
					setVisible(false);
					dispose();
					AltosUIPreferences.unregister_font_listener(AltosIdleMonitorUI.this);
				}
			});

		pack();
		setVisible(true);

		thread = new AltosIdleMonitor((AltosIdleMonitorListener) this, (AltosLink) new AltosSerial (device), (boolean) remote);

		status_update = new AltosFlightStatusUpdate(flightStatus);

		new javax.swing.Timer(100, status_update).start();

		thread.start();
	}
}
