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
import java.util.concurrent.*;
import java.awt.*;
import org.altusmetrum.altosuilib_2.*;

public class AltosLaunch {
	AltosDevice	device;
	AltosSerial	serial;
	boolean		serial_started;
	int		launcher_serial;
	int		launcher_channel;
	int		rssi;

	final static int	Unknown = -1;
	final static int	Good = 0;
	final static int	Bad = 1;

	int		armed;
	int		igniter;

	private void start_serial() throws InterruptedException {
		serial_started = true;
	}

	private void stop_serial() throws InterruptedException {
		if (!serial_started)
			return;
		serial_started = false;
		if (serial == null)
			return;
	}

	class string_ref {
		String	value;

		public String get() {
			return value;
		}
		public void set(String i) {
			value = i;
		}
		public string_ref() {
			value = null;
		}
	}

	private boolean get_string(String line, String label, string_ref s) {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			s.set(quoted);
			return true;
		} else {
			return false;
		}
	}

	public boolean status() throws InterruptedException, TimeoutException {
		boolean ok = false;
		if (serial == null)
			return false;
		string_ref status_name = new string_ref();
		start_serial();
		serial.printf("l %d %d\n", launcher_serial, launcher_channel);
		for (;;) {
			String line = serial.get_reply(20000);
			if (line == null)
				throw new TimeoutException();
			if (get_string(line, "Rssi: ", status_name)) {
				try {
					rssi = Altos.fromdec(status_name.get());
				} catch (NumberFormatException ne) {
				}
				break;
			} else if (get_string(line, "Armed: ", status_name)) {
				armed = Good;
				String status = status_name.get();
				if (status.startsWith("igniter good"))
					igniter = Good;
				else if (status.startsWith("igniter bad"))
					igniter = Bad;
				else
					igniter = Unknown;
				ok = true;
			} else if (get_string(line, "Disarmed: ", status_name)) {
				armed = Bad;
				if (status_name.get().startsWith("igniter good"))
					igniter = Good;
				else if (status_name.get().startsWith("igniter bad"))
					igniter = Bad;
				else
					igniter = Unknown;
				ok = true;
			} else if (get_string(line, "Error ", status_name)) {
				armed = Unknown;
				igniter = Unknown;
				ok = false;
				break;
			}
		}
		stop_serial();
		if (!ok) {
			armed = Unknown;
			igniter = Unknown;
		}
		return ok;
	}

	public static String status_string(int status) {
		switch (status) {
		case Good:
			return "good";
		case Bad:
			return "open";
		}
		return "unknown";
	}

	public void arm() {
		if (serial == null)
			return;
		try {
			start_serial();
			serial.printf("a %d %d\n", launcher_serial, launcher_channel);
			serial.flush_output();
		} catch (InterruptedException ie) {
		} finally {
			try {
				stop_serial();
			} catch (InterruptedException ie) {
			}
		}
	}

	public void fire() {
		if (serial == null)
			return;
		try {
			start_serial();
			serial.printf("i %d %d\n", launcher_serial, launcher_channel);
			serial.flush_output();
		} catch (InterruptedException ie) {
		} finally {
			try {
				stop_serial();
			} catch (InterruptedException ie) {
			}
		}
	}

	public void close() {
		try {
			stop_serial();
		} catch (InterruptedException ie) {
		}
		serial.close();
		serial = null;
	}

	public void set_frame(Frame frame) {
		serial.set_frame(frame);
	}

	public void set_remote(int in_serial, int in_channel) {
		launcher_serial = in_serial;
		launcher_channel = in_channel;
	}

	public AltosLaunch(AltosDevice in_device) throws FileNotFoundException, AltosSerialInUseException {

		device = in_device;
		serial = new AltosSerial(device);
	}
}