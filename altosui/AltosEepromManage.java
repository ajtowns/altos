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

public class AltosEepromManage implements ActionListener {

	JFrame			frame;
	boolean			remote;
	AltosDevice		device;
	AltosSerial		serial_line;
	AltosEepromList		flights;
	AltosEepromDownload	download;
	AltosEepromDelete	delete;
	boolean			any_download;
	boolean			any_delete;

	public void finish() {
		if (serial_line != null) {
			serial_line.flush_input();
			serial_line.close();
			serial_line = null;
		}
	}

	private String showDeletedFlights() {
		String	result = "";

		for (AltosEepromLog flight : flights) {
			if (flight.delete) {
				if (result.equals(""))
					result = String.format("%d", flight.flight);
				else
					result = String.format("%s, %d", result, flight.flight);
			}
		}
		return result;
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();
		boolean	success = e.getID() != 0;
		boolean running = false;

		if (cmd.equals("download")) {
			if (success) {
				if (any_delete) {
					delete.start();
					running = true;
				}
			}
		} else if (cmd.equals("delete")) {
			if (success) {
				JOptionPane.showMessageDialog(frame,
							      String.format("Flights erased: %s",
									    showDeletedFlights()),
							      serial_line.device.toShortString(),
							      JOptionPane.INFORMATION_MESSAGE);
			}
		}
		if (!running)
			finish();
	}

	public AltosEepromManage(JFrame given_frame) {

		boolean	running = false;

		frame = given_frame;
		device = AltosDeviceDialog.show(frame, AltosDevice.product_any);

		remote = false;
		any_download = false;
		any_delete = false;

		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				if (!device.matchProduct(AltosDevice.product_telemetrum))
					remote = true;

				flights = new AltosEepromList(serial_line, remote);

				if (flights.size() == 0) {
					JOptionPane.showMessageDialog(frame,
								      String.format("No flights available on %d",
										    device.getSerial()),
								      serial_line.device.toShortString(),
						JOptionPane.INFORMATION_MESSAGE);
				} else {
					AltosEepromSelect	select = new AltosEepromSelect(frame, flights);

					if (select.run()) {
						for (AltosEepromLog flight : flights) {
							any_download = any_download || flight.download;
							any_delete = any_delete || flight.delete;
						}
						if (any_download) {
							download = new AltosEepromDownload(frame,
											   serial_line,
											   remote,
											   flights);
							download.addActionListener(this);
						}

						if (any_delete) {
							delete = new AltosEepromDelete(frame,
										       serial_line,
										       remote,
										       flights);
							delete.addActionListener(this);
						}

						/*
						 * Start flight log download
						 */

						if (any_download) {
							download.start();
							running = true;
						}
						else if (any_delete) {
							delete.start();
							running = true;
						}
					}
				}
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
			} catch (TimeoutException te) {
				JOptionPane.showMessageDialog(frame,
							      String.format("Communications failed with \"%s\"",
									    device.toShortString()),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (InterruptedException ie) {
			}
			if (!running)
				finish();
		}
	}
}
