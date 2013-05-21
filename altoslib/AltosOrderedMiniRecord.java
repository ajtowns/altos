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

package org.altusmetrum.altoslib_1;

import java.text.ParseException;

/*
 * AltosRecords with an index field so they can be sorted by tick while preserving
 * the original ordering for elements with matching ticks
 */
class AltosOrderedMiniRecord extends AltosEepromMini implements Comparable<AltosOrderedMiniRecord> {

	public int	index;

	public AltosOrderedMiniRecord(String line, int in_index, int prev_tick, boolean prev_tick_valid)
		throws ParseException {
		super(line);
		if (prev_tick_valid) {
			tick |= (prev_tick & ~0xffff);
			if (tick < prev_tick) {
				if (prev_tick - tick > 0x8000)
					tick += 0x10000;
			} else {
				if (tick - prev_tick > 0x8000)
					tick -= 0x10000;
			}
		}
		index = in_index;
	}

	public int compareTo(AltosOrderedMiniRecord o) {
		int	tick_diff = tick - o.tick;
		if (tick_diff != 0)
			return tick_diff;
		return index - o.index;
	}
}