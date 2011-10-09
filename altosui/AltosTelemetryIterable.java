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

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosTelemetryIterable extends AltosRecordIterable {
	TreeSet<AltosRecord>	records;

	public Iterator<AltosRecord> iterator () {
		return records.iterator();
	}

	boolean has_gps = false;
	boolean has_accel = false;
	boolean has_ignite = false;
	public boolean has_gps() { return has_gps; }
	public boolean has_accel() { return has_accel; }
	public boolean has_ignite() { return has_ignite; };

	public AltosTelemetryIterable (FileInputStream input) {
		boolean saw_boost = false;
		int	current_tick = 0;
		int	boost_tick = 0;

		AltosRecord	previous = null;
		records = new TreeSet<AltosRecord> ();

		try {
			for (;;) {
				String line = AltosRecord.gets(input);
				if (line == null) {
					break;
				}
				try {
					AltosRecord record = AltosTelemetry.parse(line, previous);
					if (record == null)
						break;
					if (records.isEmpty()) {
						current_tick = record.tick;
					} else {
						int tick = record.tick;
						while (tick < current_tick - 0x1000)
							tick += 0x10000;
						current_tick = tick;
						record.tick = current_tick;
					}
					if (!saw_boost && record.state >= Altos.ao_flight_boost)
					{
						saw_boost = true;
						boost_tick = record.tick;
					}
					if (record.accel != AltosRecord.MISSING)
						has_accel = true;
					if (record.gps != null)
						has_gps = true;
					if (record.main != AltosRecord.MISSING)
						has_ignite = true;
					if (previous != null && previous.tick != record.tick)
						records.add(previous);
					previous = record;
				} catch (ParseException pe) {
					System.out.printf("parse exception %s\n", pe.getMessage());
				} catch (AltosCRCException ce) {
				}
			}
		} catch (IOException io) {
			System.out.printf("io exception\n");
		}

		if (previous != null)
			records.add(previous);

		/* adjust all tick counts to be relative to boost time */
		for (AltosRecord r : this)
			r.time = (r.tick - boost_tick) / 100.0;

		try {
			input.close();
		} catch (IOException ie) {
		}
	}
}
