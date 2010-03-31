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

/*
 * Deal with TeleDongle on a serial port
 */

package altosui;

import java.lang.String;
import java.lang.System;
import java.lang.Character;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.concurrent.LinkedBlockingQueue;
import java.lang.InterruptedException;
import java.util.LinkedList;
import altosui.AltosSerialMonitor;
import java.util.Iterator;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */
class AltosSerialReader implements Runnable {
	FileInputStream	serial_in;
	LinkedBlockingQueue<String> monitor_queue;
	LinkedBlockingQueue<String> reply_queue;
	String line;

	public void run () {
		int c;

		try {
			while ((c = serial_in.read()) != -1) {
				if (c == '\r')
					continue;
				synchronized(this) {
					if (c == '\n') {
						if (line != "") {
							if (line.startsWith("VERSION"))
								monitor_queue.put(line);
							else
								reply_queue.put(line);
							line = "";
						}
					} else {
						line = line + (char) c;
					}
				}
			}
		} catch (IOException e) {
		} catch (InterruptedException e) {
		}
	}

	public String get_telem() {
		try {
			return monitor_queue.take();
		} catch (InterruptedException e) {
			return "";
		}
	}

	public String get_reply() {
		try {
			return reply_queue.take();
		} catch (InterruptedException e) {
			return "";
		}
	}

	public void flush () {
		synchronized(this) {
			if (!"VERSION".startsWith(line) && !line.startsWith("VERSION"))
				line = "";
			reply_queue.clear();
		}
	}
	public AltosSerialReader (FileInputStream in) {
		serial_in = in;
		monitor_queue = new LinkedBlockingQueue<String> ();
		reply_queue = new LinkedBlockingQueue<String> ();
		line = "";
	}

}

public class AltosSerial implements Runnable {
	FileInputStream	serial_in = null;
	FileOutputStream serial_out = null;
	AltosSerialReader reader;
	LinkedList<AltosSerialMonitor> callbacks;

	public void run() {
		for (;;) {
			String s = reader.get_reply();
			synchronized(callbacks) {
				Iterator<AltosSerialMonitor> i = callbacks.iterator();
				while (i.hasNext()) {
					i.next().data(s);
				}
			}
		}
	}

	public void start () {
		try {
			serial_out.write('?');
			serial_out.write('\r');
		} catch (IOException e) {
		}
		(new Thread(reader)).start();
		(new Thread(this)).start();
	}

	public void monitor(AltosSerialMonitor monitor) {
		synchronized(callbacks) {
			callbacks.add(monitor);
		}
	}

	public AltosSerial(String serial_name) {
		try {
			serial_in = new FileInputStream(serial_name);
			serial_out = new FileOutputStream(serial_name);
			reader = new AltosSerialReader(serial_in);
			callbacks = new LinkedList<AltosSerialMonitor>();
		} catch (FileNotFoundException e) {
		}
	}
}
