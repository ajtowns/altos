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

package org.altusmetrum.AltosLib;

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

public class AltosEepromRecord {
	public int	cmd;
	public int	tick;
	public int	a;
	public int	b;
	public String	data;
	public boolean	tick_valid;

	static final int	record_length = 8;

	public AltosEepromRecord (AltosEepromChunk chunk, int start) throws ParseException {

		cmd = chunk.data(start);
		tick_valid = true;

		tick_valid = !chunk.erased(start, record_length);
		if (tick_valid) {
			if (AltosConvert.checksum(chunk.data, start, record_length) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x",
								       chunk.address + start), 0);
		} else {
			cmd = AltosLib.AO_LOG_INVALID;
		}

		tick = chunk.data16(start + 2);
		a = chunk.data16(start + 4);
		b = chunk.data16(start + 6);

		data = null;
	}

	public AltosEepromRecord (String line) {
		tick_valid = false;
		tick = 0;
		a = 0;
		b = 0;
		data = null;
		if (line == null) {
			cmd = AltosLib.AO_LOG_INVALID;
			data = "";
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = AltosLib.AO_LOG_INVALID;
						data = line;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						tick_valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else if (tokens[0].equals("Config") && tokens[1].equals("version:")) {
					cmd = AltosLib.AO_LOG_CONFIG_VERSION;
					data = tokens[2];
				} else if (tokens[0].equals("Main") && tokens[1].equals("deploy:")) {
					cmd = AltosLib.AO_LOG_MAIN_DEPLOY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Apogee") && tokens[1].equals("delay:")) {
					cmd = AltosLib.AO_LOG_APOGEE_DELAY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("channel:")) {
					cmd = AltosLib.AO_LOG_RADIO_CHANNEL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Callsign:")) {
					cmd = AltosLib.AO_LOG_CALLSIGN;
					data = tokens[1].replaceAll("\"","");
				} else if (tokens[0].equals("Accel") && tokens[1].equals("cal")) {
					cmd = AltosLib.AO_LOG_ACCEL_CAL;
					a = Integer.parseInt(tokens[3]);
					b = Integer.parseInt(tokens[5]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("cal:")) {
					cmd = AltosLib.AO_LOG_RADIO_CAL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Max") && tokens[1].equals("flight") && tokens[2].equals("log:")) {
					cmd = AltosLib.AO_LOG_MAX_FLIGHT_LOG;
					a = Integer.parseInt(tokens[3]);
				} else if (tokens[0].equals("manufacturer")) {
					cmd = AltosLib.AO_LOG_MANUFACTURER;
					data = tokens[1];
				} else if (tokens[0].equals("product")) {
					cmd = AltosLib.AO_LOG_PRODUCT;
					data = tokens[1];
				} else if (tokens[0].equals("serial-number")) {
					cmd = AltosLib.AO_LOG_SERIAL_NUMBER;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("log-format")) {
					cmd = AltosLib.AO_LOG_LOG_FORMAT;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("software-version")) {
					cmd = AltosLib.AO_LOG_SOFTWARE_VERSION;
					data = tokens[1];
				} else {
					cmd = AltosLib.AO_LOG_INVALID;
					data = line;
				}
			} catch (NumberFormatException ne) {
				cmd = AltosLib.AO_LOG_INVALID;
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
