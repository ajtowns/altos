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

	static final String[] state_names = {
		"startup",
		"idle",
		"pad",
		"boost",
		"fast",
		"coast",
		"drogue",
		"main",
		"landed",
		"invalid",
	};

	int[] ParseHex(String line) {
		String[] tokens = line.split("\\s+");
		int[] array = new int[tokens.length];

		for (int i = 0; i < tokens.length; i++)
			try {
				array[i] = Integer.parseInt(tokens[i], 16);
			} catch (NumberFormatException ne) {
				return null;
			}
		return array;
	}

	int checksum(int[] line) {
		int	csum = 0x5a;
		for (int i = 1; i < line.length; i++)
			csum += line[i];
		return csum & 0xff;
	}

	void FlushPending(FileWriter file, LinkedList<String> pending) throws IOException {
		while (!pending.isEmpty()) {
			file.write(pending.remove());
		}
	}

	JFrame			frame;
	AltosDevice		device;
	AltosSerial		serial_line;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	void CaptureLog() throws IOException, InterruptedException, TimeoutException {
		int			serial = 0;
		int			block, state_block = 0;
		int			addr;
		int			flight = 0;
		int			year = 0, month = 0, day = 0;
		int			state = 0;
		boolean			done = false;
		boolean			want_file = false;
		boolean			any_valid;
		FileWriter		eeprom_file = null;
		AltosFile		eeprom_name;
		LinkedList<String>	eeprom_pending = new LinkedList<String>();

		serial_line.printf("\nc s\nv\n");

		/* Pull the serial number out of the version information */

		for (;;) {
			String	line = serial_line.get_reply(5000);

			if (line == null)
				throw new TimeoutException();
			if (line.startsWith("serial-number")) {
				try {
					serial = Integer.parseInt(line.substring(13).trim());
				} catch (NumberFormatException ne) {
					serial = 0;
				}
			}

			eeprom_pending.add(String.format("%s\n", line));

			/* signals the end of the version info */
			if (line.startsWith("software-version"))
				break;
		}
		if (serial == 0)
			throw new IOException("no serial number found");

		monitor.set_serial(serial);
		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		state = 0; state_block = 0;
		for (block = 0; !done && block < 511; block++) {
			serial_line.printf("e %x\n", block);
			any_valid = false;
			monitor.set_value(state_names[state], state, block - state_block);
			for (addr = 0; addr < 0x100;) {
				String	line = serial_line.get_reply(5000);
				if (line == null)
					throw new TimeoutException();
				int[] values = ParseHex(line);

				if (values == null) {
					System.out.printf("invalid line: %s\n", line);
					continue;
				} else if (values[0] != addr) {
					System.out.printf("data address out of sync at 0x%x\n",
							  block * 256 + values[0]);
				} else if (checksum(values) != 0) {
					System.out.printf("invalid checksum at 0x%x\n",
							  block * 256 + values[0]);
				} else {
					any_valid = true;
					int	cmd = values[1];
					int	tick = values[3] + (values[4] << 8);
					int	a = values[5] + (values[6] << 8);
					int	b = values[7] + (values[8] << 8);

					if (cmd == Altos.AO_LOG_FLIGHT) {
						flight = b;
						monitor.set_flight(flight);
					}

					/* Monitor state transitions to update display */
					if (cmd == Altos.AO_LOG_STATE && a <= Altos.ao_flight_landed) {
						if (a > Altos.ao_flight_pad)
							want_file = true;
						if (a > state)
							state_block = block;
						state = a;
					}

					if (cmd == Altos.AO_LOG_GPS_DATE) {
						year = 2000 + (a & 0xff);
						month = (a >> 8) & 0xff;
						day = (b & 0xff);
						want_file = true;
					}

					if (eeprom_file == null) {
						if (serial != 0 && flight != 0 && want_file) {
							if (year != 0 && month != 0 && day != 0)
								eeprom_name = new AltosFile(year, month, day, serial, flight, "eeprom");
							else
								eeprom_name = new AltosFile(serial, flight, "eeprom");

							monitor.set_file(eeprom_name.getName());
							eeprom_file = new FileWriter(eeprom_name);
							if (eeprom_file != null) {
								FlushPending(eeprom_file, eeprom_pending);
								eeprom_pending = null;
							}
						}
					}

					String log_line = String.format("%c %4x %4x %4x\n",
									cmd, tick, a, b);
					if (eeprom_file != null)
						eeprom_file.write(log_line);
					else
						eeprom_pending.add(log_line);

					if (cmd == Altos.AO_LOG_STATE && a == Altos.ao_flight_landed) {
						done = true;
					}
				}
				addr += 8;
			}
			if (!any_valid)
				done = true;
		}
		if (eeprom_file == null) {
			eeprom_name = new AltosFile(serial,flight,"eeprom");
			eeprom_file = new FileWriter(eeprom_name);
			if (eeprom_file != null) {
				FlushPending(eeprom_file, eeprom_pending);
			}
		}
		if (eeprom_file != null) {
			eeprom_file.flush();
			eeprom_file.close();
		}
	}

	public void run () {
		if (remote) {
			serial_line.set_radio();
			serial_line.printf("p\nE 0\n");
			serial_line.flush_input();
		}

		monitor = new AltosEepromMonitor(frame, Altos.ao_flight_boost, Altos.ao_flight_landed);
		monitor.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					eeprom_thread.interrupt();
				}
			});
		try {
			CaptureLog();
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(frame,
						      device.toShortString(),
						      ee.getLocalizedMessage(),
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
		} catch (TimeoutException te) {
			JOptionPane.showMessageDialog(frame,
						      String.format("Connection to \"%s\" failed",
								    device.toShortString()),
						      "Connection Failed",
						      JOptionPane.ERROR_MESSAGE);
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
				eeprom_thread = new Thread(this);
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
