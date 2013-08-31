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

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosTelemetryIterable implements Iterable<AltosTelemetry> {
	LinkedList<AltosTelemetry>	telems;

	public Iterator<AltosTelemetry> iterator () {
		return telems.iterator();
	}

	public AltosTelemetryIterable (FileInputStream input) {
		telems = new LinkedList<AltosTelemetry> ();

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
					telems.add(telem);
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
