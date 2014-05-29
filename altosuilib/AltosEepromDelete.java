/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_4.*;

public class AltosEepromDelete implements Runnable {
	AltosEepromList		flights;
	Thread			eeprom_thread;
	AltosSerial		serial_line;
	boolean			remote;
	JFrame			frame;
	ActionListener		listener;
	boolean			success;

	private void DeleteLog (AltosEepromLog log)
		throws IOException, InterruptedException, TimeoutException {

		if (flights.config_data.flight_log_max != 0 || flights.config_data.log_format != 0) {

			/* Devices with newer firmware can erase the
			 * flash blocks containing each flight
			 */
			serial_line.flush_input();
			serial_line.printf("d %d\n", log.flight);
			for (;;) {
				/* It can take a while to erase the flash... */
				String line = serial_line.get_reply(20000);
				if (line == null)
					throw new TimeoutException();
				if (line.equals("Erased"))
					break;
				if (line.startsWith("No such"))
					throw new IOException(line);
			}
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

	public void run () {
		success = false;
		try {
			if (remote)
				serial_line.start_remote();

			for (AltosEepromLog log : flights) {
				if (log.selected) {
					DeleteLog(log);
				}
			}
			success = true;
		} catch (IOException ee) {
			show_error (ee.getLocalizedMessage(),
				    serial_line.device.toShortString());
		} catch (InterruptedException ie) {
		} catch (TimeoutException te) {
			show_error (String.format("Connection to \"%s\" failed",
						  serial_line.device.toShortString()),
				    "Connection Failed");
		} finally {
			try {
				if (remote)
					serial_line.stop_remote();
			} catch (InterruptedException ie) {
			} finally {
				serial_line.flush_output();
				serial_line.close();
			}
		}
		if (listener != null) {
			Runnable r = new Runnable() {
					public void run() {
						try {
							listener.actionPerformed(new ActionEvent(this,
												 success ? 1 : 0,
												 "delete"));
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

	public AltosEepromDelete(JFrame given_frame,
				 AltosSerial given_serial_line,
				 boolean given_remote,
				 AltosEepromList given_flights) {
		frame = given_frame;
		serial_line = given_serial_line;
		remote = given_remote;
		flights = given_flights;
		success = false;
	}
}
