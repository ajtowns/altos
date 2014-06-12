/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

public abstract class AltosEeprom implements AltosStateUpdate {
	public int	cmd;
	public int	tick;
	public int	data8[];
	public boolean	valid;

	public int data8(int i) {
		return data8[i];
	}

	public int data16(int i) {
		return ((data8[i] | (data8[i+1] << 8)) << 16) >> 16;
	}

	public int data24(int i) {
		return data8[i] | (data8[i+1] << 8) | (data8[i+2] << 16);
	}

	public int data32(int i) {
		return data8[i] | (data8[i+1] << 8) | (data8[i+2] << 16) | (data8[i+3] << 24);
	}

	public boolean has_seconds() { return false; }

	public int seconds() { return 0; }

	public final static int header_length = 4;

	public abstract int record_length();

	public void update_state(AltosState state) {
		if (cmd == AltosLib.AO_LOG_FLIGHT)
			state.set_boost_tick(tick);
		else
			state.set_tick(tick);
	}

	public void write(PrintStream out) {
		out.printf("%c %04x", cmd, tick);
		if (data8 != null) {
			for (int i = 0; i < data8.length; i++)
				out.printf (" %02x", data8[i]);
		}
		out.printf ("\n");
	}

	public String string() {
		String	s;

		s = String.format("%c %04x", cmd, tick);
		if (data8 != null) {
			for (int i = 0; i < data8.length; i++) {
				String	d = String.format(" %02x", data8[i]);
				s = s.concat(d);
			}
		}
		s = s.concat("\n");
		return s;
	}

	void parse_chunk(AltosEepromChunk chunk, int start) throws ParseException {
		cmd = chunk.data(start);

		int data_length = record_length() - header_length;

		valid = !chunk.erased(start, record_length());
		if (valid) {
			if (AltosConvert.checksum(chunk.data, start, record_length()) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x",
								       chunk.address + start), 0);
		} else {
			cmd = AltosLib.AO_LOG_INVALID;
		}

		tick = chunk.data16(start+2);

		data8 = new int[data_length];
		for (int i = 0; i < data_length; i++)
			data8[i] = chunk.data(start + header_length + i);
	}

	void parse_string(String line) {
		valid = false;
		tick = 0;
		cmd = AltosLib.AO_LOG_INVALID;

		int data_length = record_length() - header_length;

		if (line == null)
			return;
		try {
			String[] tokens = line.split("\\s+");

			if (tokens[0].length() == 1) {
				if (tokens.length == 2 + data_length) {
					cmd = tokens[0].codePointAt(0);
					tick = Integer.parseInt(tokens[1],16);
					valid = true;
					data8 = new int[data_length];

					for (int i = 0; i < data_length; i++)
						data8[i] = Integer.parseInt(tokens[2 + i],16);
				}
			}
		} catch (NumberFormatException ne) {
		}
	}
}
