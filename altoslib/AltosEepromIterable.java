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

class AltosEepromOrdered implements Comparable<AltosEepromOrdered> {
	AltosEeprom	eeprom;
	int		index;
	int		tick;

	int cmdi() {
		if (eeprom.cmd == AltosLib.AO_LOG_FLIGHT)
			return 0;
		return 1;
	}

	public int compareTo(AltosEepromOrdered o) {
		int	cmd_diff = cmdi() - o.cmdi();

		if (cmd_diff != 0)
			return cmd_diff;

		if (eeprom.has_seconds() && o.eeprom.has_seconds()) {
			int	seconds_diff = eeprom.seconds() - o.eeprom.seconds();

			if (seconds_diff != 0)
				return seconds_diff;
		}

		int	tick_diff = tick - o.tick;

		if (tick_diff != 0)
			return tick_diff;
		return index - o.index;
	}

	AltosEepromOrdered (AltosEeprom eeprom, int index, int tick) {
		this.eeprom = eeprom;
		this.index = index;
		this.tick = tick;
	}
}

class AltosEepromOrderedIterator implements Iterator<AltosEeprom> {
	TreeSet<AltosEepromOrdered>	olist;
	Iterator<AltosEepromOrdered>	oiterator;

	public AltosEepromOrderedIterator(Iterable<AltosEeprom> eeproms) {
		olist = new TreeSet<AltosEepromOrdered>();

		int	tick = 0;
		int	index = 0;
		boolean	first = true;

		for (AltosEeprom e : eeproms) {
			int	t = e.tick;
			if (first)
				tick = t;
			else {
				while (t < tick - 32767)
					t += 65536;
				tick = t;
			}
			olist.add(new AltosEepromOrdered(e, index++, tick));
			first = false;
		}

		oiterator = olist.iterator();
	}

	public boolean hasNext() {
		return oiterator.hasNext();
	}

	public AltosEeprom next() {
		return oiterator.next().eeprom;
	}

	public void remove () {
	}
}

public class AltosEepromIterable implements Iterable<AltosEeprom> {
	public LinkedList<AltosEeprom> eeproms;

	public void write(PrintStream out) {
		for (AltosEeprom eeprom : eeproms)
			eeprom.write(out);
	}

	public AltosState state() {
		AltosState	state = new AltosState();

		for (AltosEeprom header : eeproms)
			header.update_state(state);
		return state;
	}

	public AltosEepromIterable(LinkedList<AltosEeprom> eeproms) {
		this.eeproms = eeproms;
	}

	public Iterator<AltosEeprom> iterator() {
		if (eeproms == null)
			eeproms = new LinkedList<AltosEeprom>();
		return new AltosEepromOrderedIterator(eeproms);
	}
}
