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
import java.util.concurrent.LinkedBlockingQueue;

import altosui.AltosSerial;
import altosui.AltosSerialMonitor;
import altosui.AltosTelemetry;
import altosui.AltosState;
import altosui.AltosDeviceDialog;
import altosui.AltosPreferences;
import altosui.AltosLog;
import altosui.AltosVoice;

import libaltosJNI.*;

public class AltosEeprom {

	static final int AO_LOG_FLIGHT = 'F';
	static final int AO_LOG_SENSOR = 'A';
	static final int AO_LOG_TEMP_VOLT = 'T';
	static final int AO_LOG_DEPLOY = 'D';
	static final int AO_LOG_STATE = 'S';
	static final int AO_LOG_GPS_TIME = 'G';
	static final int AO_LOG_GPS_LAT = 'N';
	static final int AO_LOG_GPS_LON = 'W';
	static final int AO_LOG_GPS_ALT = 'H';
	static final int AO_LOG_GPS_SAT = 'V';
	static final int AO_LOG_GPS_DATE = 'Y';

	static final int ao_flight_startup = 0;
	static final int ao_flight_idle = 1;
	static final int ao_flight_pad = 2;
	static final int ao_flight_boost = 3;
	static final int ao_flight_fast = 4;
	static final int ao_flight_coast = 5;
	static final int ao_flight_drogue = 6;
	static final int ao_flight_main = 7;
	static final int ao_flight_landed = 8;
	static final int ao_flight_invalid = 9;

	static String[] state_names = {
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

	static int[] ParseHex(String line) {
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

	static int checksum(int[] line) {
		int	csum = 0x5a;
		for (int i = 1; i < line.length; i++)
			csum += line[i];
		return csum & 0xff;
	}

	static void FlushPending(FileWriter file, LinkedList<String> pending) throws IOException {
		while (!pending.isEmpty()) {
			file.write(pending.remove());
		}
	}

	static void CaptureLog(AltosSerial serial_line) throws IOException, InterruptedException {
		int			serial = 0;
		int			block;
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

		serial_line.printf("v\n");

		/* Pull the serial number out of the version information */

		for (;;) {
			String	line = serial_line.get_reply();

			if (line.startsWith("serial-number")) {
				try {
					serial = Integer.parseInt(line.substring(13).trim());
					eeprom_pending.add(String.format("%s\n", line));
				} catch (NumberFormatException ne) {
					serial = 0;
				}
			}

			/* signals the end of the version info */
			if (line.startsWith("software-version"))
				break;
		}
		if (serial == 0)
			throw new IOException("no serial number found");

		/* Now scan the eeprom, reading blocks of data and converting to .eeprom file form */

		for (block = 0; !done && block < 511; block++) {
			serial_line.printf("e %x\n", block);
			any_valid = false;
			for (addr = 0; addr < 0x100;) {
				String	line = serial_line.get_reply();
				int[] values = ParseHex(line);

				if (values == null) {
					System.out.printf("invalid line: %s\n", line);
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

					if (cmd == AO_LOG_FLIGHT)
						flight = b;

					/* Monitor state transitions to update display */
					if (cmd == AO_LOG_STATE && a <= ao_flight_landed) {
						System.out.printf ("%s\n", state_names[a]);
						if (a > ao_flight_pad)
							want_file = true;
						state = a;
					}

					if (cmd == AO_LOG_GPS_DATE) {
						year = 2000 + (a & 0xff);
						month = (a >> 8) & 0xff;
						day = (b & 0xff);
						want_file = true;
					}

					if (eeprom_file == null) {
						if (serial != 0 && flight != 0 && want_file) {
							if (year != 0 && month != 0 && day != 0)
								eeprom_name = new AltosFile(year, month, day, serial, flight, "eeprom-new");
							else
								eeprom_name = new AltosFile(serial, flight, "eeprom-new");

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

					if (cmd == AO_LOG_STATE && a == ao_flight_landed) {
						done = true;
					}
				}
				addr += 8;
			}
			if (!any_valid)
				done = true;
		}
		if (eeprom_file == null) {
			eeprom_name = new AltosFile(serial,flight,"eeprom-new");
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

	public static void SaveFlightData (JFrame frame) {
		altos_device	device = AltosDeviceDialog.show(frame, null);
		boolean		remote = false;
		AltosSerial	serial_line = new AltosSerial();

		if (device == null)
			return;
		try {
			serial_line.open(device);
			if (!device.getProduct().startsWith("TeleMetrum"))
				remote = true;

			if (remote) {
				serial_line.printf("m 0\n");
				serial_line.set_channel(AltosPreferences.channel());
				serial_line.printf("p\n");
			}
			CaptureLog(serial_line);
			if (remote)
				serial_line.printf("~");
			serial_line.close();
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(frame,
						      String.format("Cannot open device \"%s\"",
								    device.getPath()),
						      "Cannot open target device",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(frame,
						      device.getPath(),
						      ee.getLocalizedMessage(),
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
		}
	}
}
