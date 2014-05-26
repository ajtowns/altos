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

package org.altusmetrum.altoslib_4;

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

public class AltosEepromDownload implements Runnable {

	AltosLink		link;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	boolean			want_file;
	FileWriter		eeprom_file;
	LinkedList<String>	eeprom_pending;

	AltosEepromList		flights;
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
			    gps.year != AltosLib.MISSING &&
			    gps.month != AltosLib.MISSING &&
			    gps.day != AltosLib.MISSING)
			{
				eeprom_name = new AltosFile(gps.year, gps.month, gps.day,
							    state.serial, state.flight, "eeprom");
			} else
				eeprom_name = new AltosFile(state.serial, state.flight, "eeprom");

			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				monitor.set_filename(eeprom_name.getName());
				FlushPending();
				eeprom_pending = null;
			}
		}
	}

	boolean			done;
	boolean			start;

	void LogEeprom(AltosEeprom r) throws IOException {
		if (r.cmd != AltosLib.AO_LOG_INVALID) {
			String line = r.string();
			if (eeprom_file != null)
				eeprom_file.write(line);
			else
				eeprom_pending.add(line);
		}
	}

	void CaptureEeprom(AltosEepromChunk eechunk, int log_format) throws IOException, ParseException {
		boolean any_valid = false;
		boolean got_flight = false;

		int record_length = 8;

		state.set_serial(flights.config_data.serial);
		monitor.set_serial(flights.config_data.serial);

		for (int i = 0; i < AltosEepromChunk.chunk_size && !done; i += record_length) {
			AltosEeprom r = eechunk.eeprom(i, log_format, state);

			if (r == null)
				continue;

			record_length = r.record_length();

			r.update_state(state);

			if (!got_flight && state.flight != AltosLib.MISSING)
				monitor.set_flight(state.flight);

			/* Monitor state transitions to update display */
			if (state.state != AltosLib.ao_flight_invalid &&
			    state.state <= AltosLib.ao_flight_landed)
			{
				if (state.state > AltosLib.ao_flight_pad)
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

	void CaptureLog(AltosEepromLog log) throws IOException, InterruptedException, TimeoutException, ParseException {
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

			AltosEepromChunk	eechunk = new AltosEepromChunk(link, block, block == log.start_block);

			/*
			 * Guess what kind of data is there if the device
			 * didn't tell us
			 */

			if (log_format == AltosLib.AO_LOG_FORMAT_UNKNOWN) {
				if (block == log.start_block) {
					if (eechunk.data(0) == AltosLib.AO_LOG_FLIGHT)
						log_format = AltosLib.AO_LOG_FORMAT_FULL;
					else
						log_format = AltosLib.AO_LOG_FORMAT_TINY;
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

	public void run () {
		try {
			boolean	failed = false;
			if (remote)
				link.start_remote();

			for (AltosEepromLog log : flights) {
				parse_exception = null;
				if (log.selected) {
					monitor.reset();
					try {
						CaptureLog(log);
					} catch (ParseException e) {
						parse_exception = e;
					}
				}
				if (parse_exception != null) {
					failed = true;
					monitor.show_message(String.format("Flight %d download error\n%s\nValid log data saved",
									   log.flight,
									   parse_exception.getMessage()),
							     link.name,
							     AltosEepromMonitor.WARNING_MESSAGE);
				}
			}
			success = !failed;
		} catch (IOException ee) {
			monitor.show_message(ee.getLocalizedMessage(),
					     link.name,
					     AltosEepromMonitor.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			monitor.show_message(String.format("Connection to \"%s\" interrupted",
							   link.name),
					     "Connection Interrupted",
					     AltosEepromMonitor.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			monitor.show_message(String.format("Connection to \"%s\" failed",
							   link.name),
					     "Connection Failed",
					     AltosEepromMonitor.ERROR_MESSAGE);
		} finally {
			if (remote) {
				try {
					link.stop_remote();
				} catch (InterruptedException ie) {
				}
			}
			link.flush_output();
		}
		monitor.done(success);
	}

	public void start() {
		eeprom_thread = new Thread(this);
		monitor.set_thread(eeprom_thread);
		eeprom_thread.start();
	}

	public AltosEepromDownload(AltosEepromMonitor given_monitor,
				   AltosLink given_link,
				   boolean given_remote,
				   AltosEepromList given_flights) {

		monitor = given_monitor;
		link = given_link;
		remote = given_remote;
		flights = given_flights;
		success = false;

		monitor.set_states(AltosLib.ao_flight_boost, AltosLib.ao_flight_landed);

		monitor.start();
	}
}
