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

public class AltosEepromTm extends AltosEeprom {
	public int	i;
	public int	a;
	public int	b;

	public static final int	record_length = 2;

	public void write(PrintStream out) {
		out.printf("%c %4x %4x %4x\n", cmd, tick, a, b);
	}

	public int record_length() { return record_length; }

	public String string() {
		return String.format("%c %4x %4x %4x\n", cmd, tick, a, b);
	}

	public void update_state(AltosState state) {
		super.update_state(state);

		switch (cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			state.set_state(AltosLib.ao_flight_boost);
			state.set_flight(b);
			break;
		case AltosLib.AO_LOG_PRESSURE:
			if (tick == 0)
				state.set_ground_pressure(AltosConvert.barometer_to_pressure(b));
			else
				state.set_pressure(AltosConvert.barometer_to_pressure(b));
			break;
		case AltosLib.AO_LOG_STATE:
			state.set_state(a);
			break;
		}
	}

	public AltosEepromTm (AltosEepromChunk chunk, int start, AltosState state) throws ParseException {
		int	value = chunk.data16(start);

		int	i = (chunk.address + start) / record_length;

		cmd = chunk.data(start);
		valid = true;

		valid = !chunk.erased(start, record_length);

		switch (i) {
		case 0:
			cmd = AltosLib.AO_LOG_FLIGHT;
			tick = 0;
			a = 0;
			b = value;
			break;
		case 1:
			cmd = AltosLib.AO_LOG_PRESSURE;
			tick = 0;
			a = 0;
			b = value;
			break;
		default:
			if ((value & 0x8000) != 0) {
				cmd = AltosLib.AO_LOG_STATE;
				tick = state.tick;
				a = value & 0x7fff;
				b = 0;
			} else {
				if (state.ascent)
					tick = state.tick + 10;
				else
					tick = state.tick + 100;
				cmd = AltosLib.AO_LOG_PRESSURE;
				a = 0;
				b = value;
			}
			break;
		}
	}

	public AltosEepromTm (String line) {
		valid = false;
		tick = 0;
		a = 0;
		b = 0;
		if (line == null) {
			cmd = AltosLib.AO_LOG_INVALID;
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = AltosLib.AO_LOG_INVALID;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else {
					cmd = AltosLib.AO_LOG_INVALID;
				}
			} catch (NumberFormatException ne) {
				cmd = AltosLib.AO_LOG_INVALID;
			}
		}
	}

	public AltosEepromTm(int in_cmd, int in_tick, int in_a, int in_b) {
		valid = true;
		cmd = in_cmd;
		tick = in_tick;
		a = in_a;
		b = in_b;
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> tms = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosEepromTm tm = new AltosEepromTm(line);
				tms.add(tm);
			} catch (IOException ie) {
				break;
			}
		}

		return tms;
	}

}
