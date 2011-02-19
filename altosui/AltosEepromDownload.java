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
	AltosSerial		serial_line;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	int			flight;
	int			year, month, day;
	boolean			want_file;
	FileWriter		eeprom_file;
	LinkedList<String>	eeprom_pending;

	AltosEepromList		flights;
	ActionListener		listener;
	boolean			success;
	ParseException		parse_exception;

	private void FlushPending() throws IOException {
		for (String s : flights.config_data) {
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
				eeprom_name = new AltosFile(year, month, day, flights.config_data.serial, flight, "eeprom");
			else
				eeprom_name = new AltosFile(flights.config_data.serial, flight, "eeprom");

			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				monitor.set_file(eeprom_name.getName());
				FlushPending();
				eeprom_pending = null;
			}
		}
	}

	void CaptureLog(AltosEepromLog log) throws IOException, InterruptedException, TimeoutException {
		int			block, state_block = 0;
		int			state = 0;
		boolean			done = false;
		int			record;

		if (flights.config_data.serial == 0)
			throw new IOException("no serial number found");

		/* Reset per-capture variables */
		flight = 0;
		year = 0;
		month = 0;
		day = 0;
		want_file = false;
		eeprom_file = null;
		eeprom_pending = new LinkedList<String>();

		/* Set serial number in the monitor dialog window */
		monitor.set_serial(flights.config_data.serial);
		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		state = 0; state_block = log.start_block;
		for (block = log.start_block; !done && block < log.end_block; block++) {
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

			if (parse_exception == null && eeblock.parse_exception != null)
				parse_exception = eeblock.parse_exception;

			CheckFile(false);

			for (record = 0; record < eeblock.size(); record++) {
				AltosEepromRecord r = eeblock.get(record);

				if (r.cmd != Altos.AO_LOG_INVALID) {
					String log_line = String.format("%c %4x %4x %4x\n",
									r.cmd, r.tick, r.a, r.b);
					if (eeprom_file != null)
						eeprom_file.write(log_line);
					else
						eeprom_pending.add(log_line);
				}
			}
		}
		CheckFile(true);
		if (eeprom_file != null) {
			eeprom_file.flush();
			eeprom_file.close();
		}
	}

	private void show_message_internal(String message, String title, int message_type) {
		JOptionPane.showMessageDialog(frame,
					      message,
					      title,
					      message_type);
	}

	private void show_message(String in_message, String in_title, int in_message_type) {
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

	public void run () {
		if (remote)
			serial_line.start_remote();

		try {
			boolean	failed = false;
			for (AltosEepromLog log : flights) {
				parse_exception = null;
				if (log.download) {
					monitor.reset();
					CaptureLog(log);
				}
				if (parse_exception != null) {
					failed = true;
					show_message(String.format("Flight %d download error\n%s\nValid log data saved",
								   log.flight,
								   parse_exception.getMessage()),
						     serial_line.device.toShortString(),
						     JOptionPane.WARNING_MESSAGE);
				}
			}
			success = !failed;
		} catch (IOException ee) {
			show_message(ee.getLocalizedMessage(),
				     serial_line.device.toShortString(),
				     JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
		} catch (TimeoutException te) {
			show_message(String.format("Connection to \"%s\" failed",
						   serial_line.device.toShortString()),
				     "Connection Failed",
				     JOptionPane.ERROR_MESSAGE);
		}
		if (remote)
			serial_line.stop_remote();
		monitor.done();
		serial_line.flush_output();
		if (listener != null) {
			Runnable r = new Runnable() {
					public void run() {
						try {
							listener.actionPerformed(new ActionEvent(this,
												 success ? 1 : 0,
												 "download"));
						} catch (Exception ex) {
						}
					}
				};
			SwingUtilities.invokeLater(r);
		}
	}

	public void start() {
		eeprom_thread = new Thread(this);
		eeprom_thread.start();
	}

	public void addActionListener(ActionListener l) {
		listener = l;
	}

	public AltosEepromDownload(JFrame given_frame,
				   AltosSerial given_serial_line,
				   boolean given_remote,
				   AltosEepromList given_flights) {

		frame = given_frame;
		serial_line = given_serial_line;
		remote = given_remote;
		flights = given_flights;
		success = false;

		monitor = new AltosEepromMonitor(frame, Altos.ao_flight_boost, Altos.ao_flight_landed);
		monitor.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					if (eeprom_thread != null)
						eeprom_thread.interrupt();
				}
			});
	}
}
