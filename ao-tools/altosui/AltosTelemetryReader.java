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

import java.lang.*;
import java.text.*;
import java.io.*;
import java.util.concurrent.*;

class AltosTelemetryReader extends AltosFlightReader {
	AltosDevice	device;
	AltosSerial	serial;
	AltosLog	log;

	LinkedBlockingQueue<AltosLine> telem;

	AltosRecord read() throws InterruptedException, ParseException, AltosCRCException, IOException {
		AltosLine l = telem.take();
		if (l.line == null)
			throw new IOException("IO error");
		return new AltosTelemetry(l.line);
	}

	void close(boolean interrupted) {
		serial.remove_monitor(telem);
		log.close();
		serial.close();
	}

	void set_channel(int channel) {
		serial.set_channel(channel);
	}

	void set_callsign(String callsign) {
		serial.set_callsign(callsign);
	}

	public AltosTelemetryReader (AltosDevice in_device) throws FileNotFoundException, IOException {
		device = in_device;
		serial = new AltosSerial();
		log = new AltosLog(serial);
		name = device.getPath();

		telem = new LinkedBlockingQueue<AltosLine>();
		serial.add_monitor(telem);
	}
}
