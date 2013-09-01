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

import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_1.*;

public class AltosEepromDownload implements Runnable {

	JFrame			frame;
	AltosSerial		serial_line;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	boolean			want_file;
	FileWriter		eeprom_file;
	LinkedList<String>	eeprom_pending;

	AltosEepromList		flights;
	ActionListener		listener;
	boolean			success;
	ParseException		parse_exception;
	AltosState		state;

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
		if (force || (state.flight != 0 && want_file)) {
			AltosFile		eeprom_name;
			AltosGPS		gps = state.gps;

			if (gps != null &&
			    gps.year != AltosRecord.MISSING &&
			    gps.month != AltosRecord.MISSING &&
			    gps.day != AltosRecord.MISSING)
			{
				eeprom_name = new AltosFile(gps.year, gps.month, gps.day,
							    state.serial, state.flight, "eeprom");
			} else
				eeprom_name = new AltosFile(state.serial, state.flight, "eeprom");

			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				monitor.set_file(eeprom_name.getName());
				FlushPending();
				eeprom_pending = null;
			}
		}
	}

	boolean			done;
	boolean			start;

	void LogEeprom(AltosEeprom r) throws IOException {
		if (r.cmd != Altos.AO_LOG_INVALID) {
			String line = r.string();
			if (eeprom_file != null)
				eeprom_file.write(line);
			else
				eeprom_pending.add(line);
		}
	}

	void CaptureEeprom(AltosEepromChunk eechunk, int log_format) throws IOException {
		boolean any_valid = false;

		int record_length = 8;

		state.set_serial(flights.config_data.serial);

		for (int i = 0; i < AltosEepromChunk.chunk_size && !done; i += record_length) {
			AltosEeprom r = eechunk.eeprom(i, log_format);

			record_length = r.record_length();

			r.update_state(state);

			/* Monitor state transitions to update display */
			if (state.state != AltosLib.ao_flight_invalid &&
			    state.state <= AltosLib.ao_flight_landed)
			{
				if (state.state > Altos.ao_flight_pad)
					want_file = true;
				if (state.state == AltosLib.ao_flight_landed)
					done = true;
			}

			if (state.gps != null)
				want_file = true;

			if (r.valid) {
				any_valid = true;
				LogEeprom(r);
			}
		}
		if (!any_valid)
			done = true;

		CheckFile(false);
	}
	
	void CaptureLog(AltosEepromLog log) throws IOException, InterruptedException, TimeoutException {
		int			block, state_block = 0;
		int			log_format = flights.config_data.log_format;

		state = new AltosState();

		done = false;
		start = true;

		if (flights.config_data.serial < 0)
			throw new IOException("no serial number found");

		/* Reset per-capture variables */
		want_file = false;
		eeprom_file = null;
		eeprom_pending = new LinkedList<String>();

		/* Set serial number in the monitor dialog window */
		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		state_block = log.start_block;
		for (block = log.start_block; !done && block < log.end_block; block++) {
			monitor.set_value(state.state_name(),
					  state.state,
					  block - state_block,
					  block - log.start_block);

			AltosEepromChunk	eechunk = new AltosEepromChunk(serial_line, block, block == log.start_block);

			/*
			 * Guess what kind of data is there if the device
			 * didn't tell us
			 */

			if (log_format == Altos.AO_LOG_FORMAT_UNKNOWN) {
				if (block == log.start_block) {
					if (eechunk.data(0) == Altos.AO_LOG_FLIGHT)
						log_format = Altos.AO_LOG_FORMAT_FULL;
					else
						log_format = Altos.AO_LOG_FORMAT_TINY;
				}
			}

			CaptureEeprom (eechunk, log_format);
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
		try {
			boolean	failed = false;
			if (remote)
				serial_line.start_remote();

			for (AltosEepromLog log : flights) {
				parse_exception = null;
				if (log.selected) {
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
			System.out.printf("download interrupted\n");
		} catch (TimeoutException te) {
			show_message(String.format("Connection to \"%s\" failed",
						   serial_line.device.toShortString()),
				     "Connection Failed",
				     JOptionPane.ERROR_MESSAGE);
		} finally {
			if (remote) {
				try {
					serial_line.stop_remote();
				} catch (InterruptedException ie) {
				}
			}
			serial_line.flush_output();
		}
		monitor.done();
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
		serial_line.set_frame(frame);
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
