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
import altosui.AltosTelemetry;

public class AltosTelemetryReader extends AltosReader {
	LinkedList<AltosRecord>	records;

	Iterator<AltosRecord> record_iterator;

	int	boost_tick;

	public AltosRecord read() throws IOException, ParseException {
		AltosRecord	r;
		if (!record_iterator.hasNext())
			return null;

		r = record_iterator.next();
		r.time = (r.tick - boost_tick) / 100.0;
		return r;
	}

	public AltosTelemetryReader (FileInputStream input) {
		boolean saw_boost = false;

		records = new LinkedList<AltosRecord> ();

		try {
			for (;;) {
				String line = AltosRecord.gets(input);
				if (line == null) {
					break;
				}
				try {
					AltosTelemetry record = new AltosTelemetry(line);
					if (record == null)
						break;
					if (!saw_boost && record.state >= Altos.ao_flight_boost)
					{
						saw_boost = true;
						boost_tick = record.tick;
					}
					records.add(record);
				} catch (ParseException pe) {
					System.out.printf("parse exception %s\n", pe.getMessage());
				}
			}
		} catch (IOException io) {
			System.out.printf("io exception\n");
		}
		record_iterator = records.iterator();
		try {
			input.close();
		} catch (IOException ie) {
		}
	}
}
