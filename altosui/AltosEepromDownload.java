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

import libaltosJNI.*;

public class AltosEepromDownload implements Runnable {

	JFrame			frame;
	AltosDevice		device;
	AltosSerial		serial_line;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;
	int			serial = 0;
	int			flight = 0;
	int			year = 0, month = 0, day = 0;
	boolean			want_file = false;
	FileWriter		eeprom_file = null;
	LinkedList<String>	eeprom_pending = new LinkedList<String>();
	AltosConfigData		config_data;

	private void FlushPending() throws IOException {
		for (String s : config_data) {
			eeprom_file.write(s);
			eeprom_file.write('\n');
		}

		for (String s : eeprom_pending)
			eeprom_file.write(s);
	}

	private void CheckFile(boolean force) throws IOException {
		if (eeprom_file != null)
			return;
		if (force || (flight != 0 && want_file)) {
			AltosFile		eeprom_name;
			if (year != 0 && month != 0 && day != 0)
				eeprom_name = new AltosFile(year, month, day, serial, flight, "eeprom");
			else
				eeprom_name = new AltosFile(serial, flight, "eeprom");

			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				monitor.set_file(eeprom_name.getName());
				FlushPending();
				eeprom_pending = null;
			}
		}
	}

	void CaptureLog(int start_block, int end_block) throws IOException, InterruptedException, TimeoutException {
		int			block, state_block = 0;
		int			state = 0;
		boolean			done = false;
		int			record;

		config_data = new AltosConfigData(serial_line);
		serial = config_data.serial;
		if (serial == 0)
			throw new IOException("no serial number found");

		monitor.set_serial(serial);
		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		state = 0; state_block = start_block;
		for (block = start_block; !done && block < end_block; block++) {
			monitor.set_value(Altos.state_to_string[state], state, block - state_block);

			AltosEepromBlock	eeblock = new AltosEepromBlock(serial_line, block);
			if (eeblock.has_flight) {
				flight = eeblock.flight;
				monitor.set_flight(flight);
			}
			if (eeblock.has_date) {
				year = eeblock.year;
				month = eeblock.month;
				day = eeblock.day;
				want_file = true;
			}

			if (eeblock.size() == 0 ||
			    eeblock.has_state && eeblock.state == Altos.ao_flight_landed)
					done = true;

			/* Monitor state transitions to update display */
			if (eeblock.has_state) {
				if (eeblock.state > Altos.ao_flight_pad)
					want_file = true;
				if (eeblock.state > state)
					state = eeblock.state;
			}

			CheckFile(true);

			for (record = 0; record < eeblock.size(); record++) {
				AltosEepromRecord r = eeblock.get(record);

				String log_line = String.format("%c %4x %4x %4x\n",
								r.cmd, r.tick, r.a, r.b);
				if (eeprom_file != null)
					eeprom_file.write(log_line);
				else
					eeprom_pending.add(log_line);
			}
		}
		CheckFile(true);
		if (eeprom_file != null) {
			eeprom_file.flush();
			eeprom_file.close();
		}
	}

	private void show_error_internal(String message, String title) {
		JOptionPane.showMessageDialog(frame,
					      message,
					      title,
					      JOptionPane.ERROR_MESSAGE);
	}

	private void show_error(String in_message, String in_title) {
		final String message = in_message;
		final String title = in_title;
		Runnable r = new Runnable() {
				public void run() {
					try {
						show_error_internal(message, title);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	int	start_block, end_block;

	public void run () {
		if (remote) {
			serial_line.set_radio();
			serial_line.printf("p\nE 0\n");
			serial_line.flush_input();
		}

		try {
			CaptureLog(start_block, end_block);
		} catch (IOException ee) {
			show_error (device.toShortString(),
				    ee.getLocalizedMessage());
		} catch (InterruptedException ie) {
		} catch (TimeoutException te) {
			show_error (String.format("Connection to \"%s\" failed",
						  device.toShortString()),
				    "Connection Failed");
		}
		if (remote)
			serial_line.printf("~");
		monitor.done();
		serial_line.flush_output();
		serial_line.close();
	}

	public AltosEepromDownload(JFrame given_frame) {
		frame = given_frame;
		device = AltosDeviceDialog.show(frame, AltosDevice.product_any);

		remote = false;

		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				if (!device.matchProduct(AltosDevice.product_telemetrum))
					remote = true;

				monitor = new AltosEepromMonitor(frame, Altos.ao_flight_boost, Altos.ao_flight_landed);
				monitor.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							if (eeprom_thread != null)
								eeprom_thread.interrupt();
						}
					});

				eeprom_thread = new Thread(this);
				start_block = 0;
				end_block = 0xfff;
				eeprom_thread.start();
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(frame,
							      String.format("Cannot open device \"%s\"",
									    device.toShortString()),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (AltosSerialInUseException si) {
				JOptionPane.showMessageDialog(frame,
							      String.format("Device \"%s\" already in use",
									    device.toShortString()),
							      "Device in use",
							      JOptionPane.ERROR_MESSAGE);
			} catch (IOException ee) {
				JOptionPane.showMessageDialog(frame,
							      device.toShortString(),
							      ee.getLocalizedMessage(),
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
