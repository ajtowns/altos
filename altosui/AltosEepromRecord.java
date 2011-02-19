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

public class AltosEepromRecord {
	public int	cmd;
	public int	tick;
	public int	a;
	public int	b;
	public String	data;
	public boolean	tick_valid;

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

	public AltosEepromRecord (AltosSerial serial_line, int addr)
		throws TimeoutException, ParseException, InterruptedException {
		String	line = serial_line.get_reply(5000);
		if (line == null)
			throw new TimeoutException();
		int[] values = ParseHex(line);

		if (values == null || values.length < 9) {
			System.out.printf("invalid line %s", line);
			throw new ParseException(String.format("inalid line %s", line), 0);
		}
		if (values[0] != (addr & 0xff))
			throw new ParseException(String.format("data address out of sync at 0x%x",
							       addr), 0);
		int i;
		for (i = 1; i < values.length; i++)
			if (values[i] != 0xff)
				break;
		cmd = values[1];
		tick_valid = true;
		if (i != values.length) {
			if (checksum(values) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x in line %s", addr, line), 0);
		} else {
			cmd = Altos.AO_LOG_INVALID;
			tick_valid = false;
		}

		tick = values[3] + (values[4] << 8);
		a = values[5] + (values[6] << 8);
		b = values[7] + (values[8] << 8);
		data = null;
	}

	public AltosEepromRecord (String line) {
		tick_valid = false;
		tick = 0;
		a = 0;
		b = 0;
		data = null;
		if (line == null) {
			cmd = Altos.AO_LOG_INVALID;
			data = "";
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = Altos.AO_LOG_INVALID;
						data = line;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						tick_valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else if (tokens[0].equals("Config") && tokens[1].equals("version:")) {
					cmd = Altos.AO_LOG_CONFIG_VERSION;
					data = tokens[2];
				} else if (tokens[0].equals("Main") && tokens[1].equals("deploy:")) {
					cmd = Altos.AO_LOG_MAIN_DEPLOY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Apogee") && tokens[1].equals("delay:")) {
					cmd = Altos.AO_LOG_APOGEE_DELAY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("channel:")) {
					cmd = Altos.AO_LOG_RADIO_CHANNEL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Callsign:")) {
					cmd = Altos.AO_LOG_CALLSIGN;
					data = tokens[1].replaceAll("\"","");
				} else if (tokens[0].equals("Accel") && tokens[1].equals("cal")) {
					cmd = Altos.AO_LOG_ACCEL_CAL;
					a = Integer.parseInt(tokens[3]);
					b = Integer.parseInt(tokens[5]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("cal:")) {
					cmd = Altos.AO_LOG_RADIO_CAL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("manufacturer")) {
					cmd = Altos.AO_LOG_MANUFACTURER;
					data = tokens[1];
				} else if (tokens[0].equals("product")) {
					cmd = Altos.AO_LOG_PRODUCT;
					data = tokens[1];
				} else if (tokens[0].equals("serial-number")) {
					cmd = Altos.AO_LOG_SERIAL_NUMBER;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("software-version")) {
					cmd = Altos.AO_LOG_SOFTWARE_VERSION;
					data = tokens[1];
				} else {
					cmd = Altos.AO_LOG_INVALID;
					data = line;
				}
			} catch (NumberFormatException ne) {
				cmd = Altos.AO_LOG_INVALID;
				data = line;
			}
		}
	}

	public AltosEepromRecord(int in_cmd, int in_tick, int in_a, int in_b) {
		tick_valid = true;
		cmd = in_cmd;
		tick = in_tick;
		a = in_a;
		b = in_b;
	}
}
