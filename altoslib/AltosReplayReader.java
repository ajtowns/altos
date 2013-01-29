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

/*
 * Open an existing telemetry file and replay it in realtime
 */

public class AltosReplayReader extends AltosFlightReader {
	Iterator<AltosRecord>	iterator;
	File	file;

	public AltosRecord read() {
		if (iterator.hasNext())
			return iterator.next();
		return null;
	}

	public void close (boolean interrupted) {
	}

	public void update(AltosState state) throws InterruptedException {
		/* Make it run in realtime after the rocket leaves the pad */
		if (state.state > AltosLib.ao_flight_pad)
			Thread.sleep((int) (Math.min(state.time_change,10) * 1000));
	}

	public File backing_file() { return file; }

	public AltosReplayReader(Iterator<AltosRecord> in_iterator, File in_file) {
		iterator = in_iterator;
		file = in_file;
		name = file.getName();
	}
}
