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

import libaltosJNI.*;

public class AltosEepromDownload implements Runnable {

	JFrame			frame;
	AltosSerial		serial_line;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	int			flight;
	int			serial;
	int			year, month, day;
	boolean			want_file;
	FileWriter		eeprom_file;
	LinkedList<String>	eeprom_pending;

	AltosEepromList		flights;
	ActionListener		listener;
	boolean			success;
	ParseException		parse_exception;
	String			extension;

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

			if (extension == null)
				extension = "data";
			if (year != 0 && month != 0 && day != 0)
				eeprom_name = new AltosFile(year, month, day, serial, flight, extension);
			else
				eeprom_name = new AltosFile(serial, flight, extension);

			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				monitor.set_file(eeprom_name.getName());
				FlushPending();
				eeprom_pending = null;
			}
		}
	}

	void Log(AltosEepromRecord r) throws IOException {
		if (r.cmd != Altos.AO_LOG_INVALID) {
			String log_line = String.format("%c %4x %4x %4x\n",
							r.cmd, r.tick, r.a, r.b);
			if (eeprom_file != null)
				eeprom_file.write(log_line);
			else
				eeprom_pending.add(log_line);
		}
	}

	void set_serial(int in_serial) {
		serial = in_serial;
		monitor.set_serial(serial);
	}

	void set_flight(int in_flight) {
		flight = in_flight;
		monitor.set_flight(flight);
	}
		
	boolean			done;
	int			state;

	void CaptureFull(AltosEepromChunk eechunk) throws IOException {
		boolean	any_valid = false;

		extension = "eeprom";
		set_serial(flights.config_data.serial);
		for (int i = 0; i < eechunk.chunk_size && !done; i += AltosEepromRecord.record_length) {
			try {
				AltosEepromRecord r = new AltosEepromRecord(eechunk, i);
				if (r.cmd == Altos.AO_LOG_FLIGHT)
					set_flight(r.b);

				/* Monitor state transitions to update display */
				if (r.cmd == Altos.AO_LOG_STATE && r.a <= Altos.ao_flight_landed) {
					state = r.a;
					if (state > Altos.ao_flight_pad)
						want_file = true;
				}

				if (r.cmd == Altos.AO_LOG_GPS_DATE) {
					year = 2000 + (r.a & 0xff);
					month = (r.a >> 8) & 0xff;
					day = (r.b & 0xff);
					want_file = true;
				}
				if (r.cmd == Altos.AO_LOG_STATE && r.a == Altos.ao_flight_landed)
					done = true;
				any_valid = true;
				Log(r);
			} catch (ParseException pe) {
				if (parse_exception == null)
					parse_exception = pe;
			}
		}

		if (!any_valid)
			done = true;

		CheckFile(false);
	}

	boolean	start;
	int	tiny_tick;

	void CaptureTiny (AltosEepromChunk eechunk) throws IOException {
		boolean any_valid = false;

		extension = "eeprom";
		set_serial(flights.config_data.serial);
		for (int i = 0; i < eechunk.data.length && !done; i += 2) {
			int			v = eechunk.data16(i);
			AltosEepromRecord	r;

			if (i == 0 && start) {
				tiny_tick = 0;
				start = false;
				set_flight(v);
				r = new AltosEepromRecord(Altos.AO_LOG_FLIGHT, tiny_tick, 0, v);
				any_valid = true;
			} else {
				int	s = v ^ 0x8000;

				if (Altos.ao_flight_startup <= s && s <= Altos.ao_flight_invalid) {
					state = s;
					r = new AltosEepromRecord(Altos.AO_LOG_STATE, tiny_tick, state, 0);
					if (state == Altos.ao_flight_landed)
						done = true;
					state = s;
					any_valid = true;
				} else {
					if (v != 0xffff)
						any_valid = true;

					r = new AltosEepromRecord(Altos.AO_LOG_PRESSURE, tiny_tick, 0, v);

					/*
					 * The flight software records ascent data every 100ms, and descent
					 * data every 1s.
					 */
					if (state < Altos.ao_flight_drogue)
						tiny_tick += 10;
					else
						tiny_tick += 100;
				}
			}
			Log(r);
		}
		CheckFile(false);
		if (!any_valid)
			done = true;
	}

	void LogTeleScience(AltosEepromTeleScience r) throws IOException {
		if (r.type != Altos.AO_LOG_INVALID) {
			String log_line = String.format("%c %4x %4x %d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
							r.type, r.tick, r.tm_tick, r.tm_state,
							r.data[0], r.data[1], r.data[2], r.data[3], 
							r.data[4], r.data[5], r.data[6], r.data[7], 
							r.data[8], r.data[9], r.data[10], r.data[11]);
			if (eeprom_file != null)
				eeprom_file.write(log_line);
			else
				eeprom_pending.add(log_line);
		}
	}
	
	boolean	telescience_start;

	void CaptureTeleScience (AltosEepromChunk eechunk) throws IOException {
		boolean	any_valid = false;

		extension = "science";
		for (int i = 0; i < eechunk.chunk_size && !done; i += AltosEepromTeleScience.record_length) {
			try {
				AltosEepromTeleScience r = new AltosEepromTeleScience(eechunk, i);
				if (r.type == AltosEepromTeleScience.AO_LOG_TELESCIENCE_START) {
					if (telescience_start) {
						done = true;
						break;
					}
					set_serial(r.data[0]);
					set_flight(r.data[1]);
					telescience_start = true;
				} else {
					if (!telescience_start)
						break;
				}
				state = r.tm_state;
				want_file =true;
				any_valid = true;
				LogTeleScience(r);
			} catch (ParseException pe) {
				if (parse_exception == null)
					parse_exception = pe;
			}
		}

		CheckFile(false);
		if (!any_valid)
			done = true;
	}

	void LogMega(AltosEepromMega r) throws IOException {
		if (r.cmd != Altos.AO_LOG_INVALID) {
			String log_line = String.format("%c %4x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x\n",
							r.cmd, r.tick,
							r.data8[0], r.data8[1], r.data8[2], r.data8[3],
							r.data8[4], r.data8[5], r.data8[6], r.data8[7],
							r.data8[8], r.data8[9], r.data8[10], r.data8[11],
							r.data8[12], r.data8[13], r.data8[14], r.data8[15],
							r.data8[16], r.data8[17], r.data8[18], r.data8[19],
							r.data8[20], r.data8[21], r.data8[22], r.data8[23],
							r.data8[24], r.data8[25], r.data8[26], r.data8[27]);
			if (eeprom_file != null)
				eeprom_file.write(log_line);
			else
				eeprom_pending.add(log_line);
		}
	}

	void CaptureMega(AltosEepromChunk eechunk) throws IOException {
		boolean any_valid = false;

		extension = "mega";
		set_serial(flights.config_data.serial);
		for (int i = 0; i < eechunk.chunk_size && !done; i += AltosEepromMega.record_length) {
			try {
				AltosEepromMega r = new AltosEepromMega(eechunk, i);
				if (r.cmd == Altos.AO_LOG_FLIGHT)
					set_flight(r.data16(0));

				/* Monitor state transitions to update display */
				if (r.cmd == Altos.AO_LOG_STATE && r.data16(0) <= Altos.ao_flight_landed) {
					state = r.data16(0);
					if (state > Altos.ao_flight_pad)
						want_file = true;
				}

				if (r.cmd == Altos.AO_LOG_GPS_TIME) {
					year = 2000 + r.data8(14);
					month = r.data8(15);
					day = r.data8(14);
					want_file = true;
				}

				if (r.cmd == Altos.AO_LOG_STATE && r.data16(0) == Altos.ao_flight_landed)
					done = true;
				any_valid = true;
				LogMega(r);
			} catch (ParseException pe) {
				if (parse_exception == null)
					parse_exception = pe;
			}
		}
		if (!any_valid)
			done = true;

		CheckFile(false);
	}
	
	void CaptureTelemetry(AltosEepromChunk eechunk) throws IOException {
		
	}

	void CaptureLog(AltosEepromLog log) throws IOException, InterruptedException, TimeoutException {
		int			block, state_block = 0;
		int			log_format = flights.config_data.log_format;

		state = 0;
		done = false;
		start = true;

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
		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		state = 0; state_block = log.start_block;
		for (block = log.start_block; !done && block < log.end_block; block++) {
			monitor.set_value(AltosLib.state_name(state), state, block - state_block);

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

			switch (log_format) {
			case AltosLib.AO_LOG_FORMAT_FULL:
				extension = "eeprom";
				CaptureFull(eechunk);
				break;
			case AltosLib.AO_LOG_FORMAT_TINY:
				extension = "eeprom";
				CaptureTiny(eechunk);
				break;
			case AltosLib.AO_LOG_FORMAT_TELEMETRY:
				extension = "telem";
				CaptureTelemetry(eechunk);
				break;
			case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
				extension = "science";
				CaptureTeleScience(eechunk);
				break;
			case AltosLib.AO_LOG_FORMAT_MEGAMETRUM:
				extension = "mega";
				CaptureMega(eechunk);
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
