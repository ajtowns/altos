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

package org.altusmetrum.altoslib_4;

import java.text.*;
import java.util.concurrent.*;

public class AltosEepromChunk {

	public static final int	chunk_size = 256;
	public static final int	per_line = 8;

	public int		data[];
	public int		address;
	public ParseException	parse_exception = null;

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

	public int data(int offset) {
		return data[offset];
	}

	public int data16(int offset) {
		return data[offset] | (data[offset + 1] << 8);
	}

	public int data32(int offset) {
		return data[offset] | (data[offset + 1] << 8) |
			(data[offset+2] << 16) | (data[offset+3] << 24);
	}

	public boolean erased(int start, int len) {
		for (int i = 0; i < len; i++)
			if (data[start+i] != 0xff)
				return false;
		return true;
	}

	public AltosEeprom eeprom(int offset, int log_format, AltosState state) throws ParseException {
		AltosEeprom	eeprom = null;
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_FULL:
			eeprom = new AltosEepromTM(this, offset);
			break;
		case AltosLib.AO_LOG_FORMAT_TINY:
			eeprom = new AltosEepromTm(this, offset, state);
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
			eeprom = new AltosEepromMega(this, offset);
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMETRUM:
			eeprom = new AltosEepromMetrum2(this, offset);
			break;
		case AltosLib.AO_LOG_FORMAT_TELEMINI:
		case AltosLib.AO_LOG_FORMAT_EASYMINI:
			eeprom = new AltosEepromMini(this, offset);
			break;
		case AltosLib.AO_LOG_FORMAT_TELEGPS:
			eeprom = new AltosEepromGPS(this, offset);
			break;
		default:
			throw new ParseException("unknown eeprom format " + log_format, 0);
		}
		return eeprom;
	}

	public AltosEepromChunk(AltosLink link, int block, boolean flush)
		throws TimeoutException, InterruptedException {

		int	offset;

		data = new int[chunk_size];
		address = block * chunk_size;
		if (flush)
			link.flush_input();
		link.printf("e %x\n", block);

		for (offset = 0; offset < chunk_size; offset += per_line) {
			try {
				String	line = link.get_reply(5000);

				if (line == null)
					throw new TimeoutException();

				int[] values = ParseHex(line);

				if (values == null || values.length != per_line + 1)
					throw new ParseException(String.format("invalid line %s", line), 0);
				if (values[0] != offset)
					throw new ParseException(String.format("data address out of sync at 0x%x",
									       address + offset), 0);
				for (int i = 0; i < per_line; i++)
					data[offset + i] = values[1 + i];
			} catch (ParseException pe) {
				for (int i = 0; i < per_line; i++)
					data[offset + i] = 0xff;
				if (parse_exception == null)
					parse_exception = pe;
			}
		}
	}
}
