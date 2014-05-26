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

class AltosTelemetryOrdered implements Comparable<AltosTelemetryOrdered> {
	AltosTelemetry	telem;
	int		index;
	int		tick;

	public int compareTo(AltosTelemetryOrdered o) {
		int	tick_diff = tick - o.tick;

		if (tick_diff != 0)
			return tick_diff;
		return index - o.index;
	}

	AltosTelemetryOrdered (AltosTelemetry telem, int index, int tick) {
		this.telem = telem;
		this.index = index;
		this.tick = tick;
	}
}

class AltosTelemetryOrderedIterator implements Iterator<AltosTelemetry> {
	Iterator<AltosTelemetryOrdered> iterator;

	public AltosTelemetryOrderedIterator(TreeSet<AltosTelemetryOrdered> telems) {
		iterator = telems.iterator();
	}

	public boolean hasNext() {
		return iterator.hasNext();
	}

	public AltosTelemetry next() {
		return iterator.next().telem;
	}

	public void remove () {
	}
}

public class AltosTelemetryIterable implements Iterable<AltosTelemetry> {
	TreeSet<AltosTelemetryOrdered>	telems;
	int tick;
	int index;

	public void add (AltosTelemetry telem) {
		int	t = telem.tick;
		if (!telems.isEmpty()) {
			while (t < tick - 1000)
				t += 65536;
		}
		tick = t;
		telems.add(new AltosTelemetryOrdered(telem, index++, tick));
	}

	public Iterator<AltosTelemetry> iterator () {
		return new AltosTelemetryOrderedIterator(telems);
	}

	public AltosTelemetryIterable (FileInputStream input) {
		telems = new TreeSet<AltosTelemetryOrdered> ();
		tick = 0;
		index = 0;

		try {
			for (;;) {
				String line = AltosLib.gets(input);
				if (line == null) {
					break;
				}
				try {
					AltosTelemetry telem = AltosTelemetry.parse(line);
					if (telem == null)
						break;
					add(telem);
				} catch (ParseException pe) {
					System.out.printf("parse exception %s\n", pe.getMessage());
				} catch (AltosCRCException ce) {
				}
			}
		} catch (IOException io) {
			System.out.printf("io exception\n");
		}
	}
}
