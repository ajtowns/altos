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

class AltosADC {
	int	tick;
	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;

	public AltosADC(AltosSerial serial) throws InterruptedException, TimeoutException {
		serial.printf("a\n");
		for (;;) {
			String line = serial.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (!line.startsWith("tick:"))
				continue;
			String[] items = line.split("\\s+");
			for (int i = 0; i < items.length;) {
				if (items[i].equals("tick:")) {
					tick = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("accel:")) {
					accel = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("pres:")) {
					pres = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("temp:")) {
					temp = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("batt:")) {
					batt = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("drogue:")) {
					drogue = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("main:")) {
					main = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
			}
			break;
		}
	}
}

class AltosGPSQuery extends AltosGPS {
	public AltosGPSQuery (AltosSerial serial, AltosConfigData config_data)
		throws TimeoutException, InterruptedException {
		boolean says_done = config_data.compare_version("1.0") >= 0;
		serial.printf("g\n");
		for (;;) {
			String line = serial.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();
			String[] bits = line.split("\\s+");
			if (bits.length == 0)
				continue;
			if (line.startsWith("Date:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split(":");
				if (d.length < 3)
					continue;
				year = Integer.parseInt(d[0]) + 2000;
				month = Integer.parseInt(d[1]);
				day = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Time:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split("/");
				if (d.length < 3)
					continue;
				hour = Integer.parseInt(d[0]);
				minute = Integer.parseInt(d[1]);
				second = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Lat/Lon:")) {
				if (bits.length < 3)
					continue;
				lat = Integer.parseInt(bits[1]) * 1.0e-7;
				lon = Integer.parseInt(bits[2]) * 1.0e-7;
				continue;
			}
			if (line.startsWith("Alt:")) {
				if (bits.length < 2)
					continue;
				alt = Integer.parseInt(bits[1]);
				continue;
			}
			if (line.startsWith("Flags:")) {
				if (bits.length < 2)
					continue;
				int status = Integer.decode(bits[1]);
				connected = (status & Altos.AO_GPS_RUNNING) != 0;
				locked = (status & Altos.AO_GPS_VALID) != 0;
				if (!says_done)
					break;
				continue;
			}
			if (line.startsWith("Sats:")) {
				if (bits.length < 2)
					continue;
				nsat = Integer.parseInt(bits[1]);
				cc_gps_sat = new AltosGPSSat[nsat];
				for (int i = 0; i < nsat; i++) {
					int	svid = Integer.parseInt(bits[2+i*2]);
					int	cc_n0 = Integer.parseInt(bits[3+i*2]);
					cc_gps_sat[i] = new AltosGPSSat(svid, cc_n0);
				}
			}
			if (line.startsWith("done"))
				break;
			if (line.startsWith("Syntax error"))
				break;
		}
	}
}

class AltosIdleMonitor extends Thread {
	AltosDevice		device;
	AltosSerial		serial;
	AltosIdleMonitorUI	ui;
	AltosState		state;
	boolean			remote;
	double			frequency;
	AltosState		previous_state;
	AltosConfigData		config_data;
	AltosADC		adc;
	AltosGPS		gps;

	int AltosRSSI() throws TimeoutException, InterruptedException {
		serial.printf("s\n");
		String line = serial.get_reply_no_dialog(5000);
		if (line == null)
			throw new TimeoutException();
		String[] items = line.split("\\s+");
		if (items.length < 2)
			return 0;
		if (!items[0].equals("RSSI:"))
			return 0;
		int rssi = Integer.parseInt(items[1]);
		return rssi;
	}

	void update_state() throws InterruptedException, TimeoutException {
		AltosRecordTM	record = new AltosRecordTM();
		int		rssi;

		try {
			if (remote) {
				serial.set_radio_frequency(frequency);
				serial.start_remote();
			} else
				serial.flush_input();
			config_data = new AltosConfigData(serial);
			adc = new AltosADC(serial);
			gps = new AltosGPSQuery(serial, config_data);
		} finally {
			if (remote) {
				serial.stop_remote();
				rssi = AltosRSSI();
			} else
				rssi = 0;
		}

		record.version = 0;
		record.callsign = config_data.callsign;
		record.serial = config_data.serial;
		record.flight = config_data.log_available() > 0 ? 255 : 0;
		record.rssi = rssi;
		record.status = 0;
		record.state = Altos.ao_flight_idle;

		record.tick = adc.tick;

		record.accel = adc.accel;
		record.pres = adc.pres;
		record.batt = adc.batt;
		record.temp = adc.temp;
		record.drogue = adc.drogue;
		record.main = adc.main;

		record.ground_accel = record.accel;
		record.ground_pres = record.pres;
		record.accel_plus_g = config_data.accel_cal_plus;
		record.accel_minus_g = config_data.accel_cal_minus;
		record.acceleration = 0;
		record.speed = 0;
		record.height = 0;
		record.gps = gps;
		state = new AltosState (record, state);
	}

	void set_frequency(double in_frequency) {
		frequency = in_frequency;
	}

	public void post_state() {
		Runnable r = new Runnable() {
				public void run() {
					ui.update(state);
				}
			};
		SwingUtilities.invokeLater(r);
	}

	public void run() {
		try {
			for (;;) {
				try {
					update_state();
					post_state();
				} catch (TimeoutException te) {
					if (AltosSerial.debug)
						System.out.printf ("monitor idle data timeout\n");
				}
				Thread.sleep(1000);
			}
		} catch (InterruptedException ie) {
			serial.close();
		}
	}

	public AltosIdleMonitor(AltosIdleMonitorUI in_ui, AltosDevice in_device, boolean in_remote)
		throws FileNotFoundException, AltosSerialInUseException, InterruptedException, TimeoutException {
		device = in_device;
		ui = in_ui;
		serial = new AltosSerial(device);
		remote = in_remote;
		state = null;
	}
}

public class AltosIdleMonitorUI extends AltosFrame implements AltosFlightDisplay, AltosFontListener {
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

	public void update(AltosState state) {
		show (state, 0);
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

		thread = new AltosIdleMonitor(this, device, remote);

		status_update = new AltosFlightStatusUpdate(flightStatus);

		new javax.swing.Timer(100, status_update).start();

		thread.start();
	}
}
