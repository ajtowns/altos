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

package org.altusmetrum.altoslib_1;

import java.text.*;

public class AltosEepromTeleScience {
	public int	type;
	public int	tick;
	public int	tm_state;
	public int	tm_tick;
	public int[]	data;
	public boolean	valid;

	public static final int AO_LOG_TELESCIENCE_START = 's';
	public static final int AO_LOG_TELESCIENCE_DATA = 'd';

	static final int	max_data = 12;
	public static final int	record_length = 32;

	public AltosEepromTeleScience (AltosEepromChunk chunk, int start) throws ParseException {
		type = chunk.data(start);

		valid = !chunk.erased(start, record_length);
		if (valid) {
			if (AltosConvert.checksum(chunk.data, start, record_length) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x",
								       chunk.address + start), 0);
		} else {
			type = AltosLib.AO_LOG_INVALID;
		}

		tick = chunk.data16(start+2);
		tm_tick = chunk.data16(start+4);
		tm_state = chunk.data(start+6);
		data = new int[max_data];
		for (int i = 0; i < max_data; i++)
			data[i] = chunk.data16(start + 8 + i * 2);
	}
}
