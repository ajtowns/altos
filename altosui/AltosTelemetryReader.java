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
import org.altusmetrum.AltosLib.*;

class AltosTelemetryReader extends AltosFlightReader {
	AltosDevice	device;
	AltosSerial	serial;
	AltosLog	log;
	AltosRecord	previous;
	double		frequency;
	int		telemetry;

	LinkedBlockingQueue<AltosLine> telem;

	AltosRecord read() throws InterruptedException, ParseException, AltosCRCException, IOException {
		AltosLine l = telem.take();
		if (l.line == null)
			throw new IOException("IO error");
		AltosRecord	next = AltosTelemetry.parse(l.line, previous);
		previous = next;
		return next;
	}

	void flush() {
		telem.clear();
	}

	void close(boolean interrupted) {
		serial.remove_monitor(telem);
		log.close();
		serial.close();
	}

	public void set_frequency(double in_frequency) throws InterruptedException, TimeoutException {
		frequency = in_frequency;
		serial.set_radio_frequency(frequency);
	}

	public boolean supports_telemetry(int telemetry) {

		try {
			/* Version 1.0 or later firmware supports all telemetry formats */
			if (serial.config_data().compare_version("1.0") >= 0)
				return true;

			/* Version 0.9 firmware only supports 0.9 telemetry */
			if (serial.config_data().compare_version("0.9") >= 0) {
				if (telemetry == Altos.ao_telemetry_0_9)
					return true;
				else
					return false;
			}

			/* Version 0.8 firmware only supports 0.8 telemetry */
			if (telemetry == Altos.ao_telemetry_0_8)
				return true;
			else
				return false;
		} catch (InterruptedException ie) {
			return true;
		} catch (TimeoutException te) {
			return true;
		}
	}

	void save_frequency() {
		AltosPreferences.set_frequency(device.getSerial(), frequency);
	}

	void set_telemetry(int in_telemetry) {
		telemetry = in_telemetry;
		serial.set_telemetry(telemetry);
	}

	void save_telemetry() {
		AltosPreferences.set_telemetry(device.getSerial(), telemetry);
	}

	File backing_file() {
		return log.file();
	}

	public AltosTelemetryReader (AltosDevice in_device)
		throws FileNotFoundException, AltosSerialInUseException, IOException, InterruptedException, TimeoutException {
		device = in_device;
		serial = new AltosSerial(device);
		log = new AltosLog(serial);
		name = device.toShortString();
		previous = null;

		telem = new LinkedBlockingQueue<AltosLine>();
		frequency = AltosPreferences.frequency(device.getSerial());
		set_frequency(frequency);
		telemetry = AltosPreferences.telemetry(device.getSerial());
		set_telemetry(telemetry);
		serial.set_callsign(AltosUIPreferences.callsign());
		serial.add_monitor(telem);
	}
}
