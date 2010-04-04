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

import java.lang.*;
import java.io.*;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.LinkedList;
import java.util.Iterator;
import gnu.io.*;
import altosui.AltosSerialMonitor;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */
class AltosSerialReader implements Runnable {
	InputStream	serial_in;
	LinkedBlockingQueue<String> monitor_queue;
	LinkedBlockingQueue<String> reply_queue;
	Thread input_thread;
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

	public String get_telem() throws InterruptedException {
		return monitor_queue.take();
	}

	public String get_reply() throws InterruptedException {
		return reply_queue.take();
	}

	public void flush () {
		synchronized(this) {
			if (!"VERSION".startsWith(line) && !line.startsWith("VERSION"))
				line = "";
			reply_queue.clear();
		}
	}

	public boolean opened() {
		return serial_in != null;
	}

	public void close() {
		if (serial_in != null) {
			try {
				serial_in.close();
			} catch (IOException e) {
			}
			serial_in = null;
		}
		if (input_thread != null) {
			try {
				input_thread.join();
			} catch (InterruptedException e) {
			}
			input_thread = null;
		}
	}

	public void open(File name) throws FileNotFoundException {
		close();
		serial_in = new FileInputStream(name);
		input_thread = new Thread(this);
		input_thread.start();
	}
	public void open(CommPort c) throws IOException {
		close();
		serial_in = c.getInputStream();
		input_thread = new Thread(this);
		input_thread.start();
	}
	public AltosSerialReader () {
		serial_in = null;
		input_thread = null;
		line = "";
		monitor_queue = new LinkedBlockingQueue<String> ();
		reply_queue = new LinkedBlockingQueue<String> ();
	}

}

public class AltosSerial implements Runnable {
	OutputStream serial_out = null;
	Thread monitor_thread = null;
	AltosSerialReader reader = null;
	LinkedList<AltosSerialMonitor> callbacks;

	public void run() {
		try {
			for (;;) {
				String s = reader.get_telem();
				synchronized(callbacks) {
					Iterator<AltosSerialMonitor> i = callbacks.iterator();
					while (i.hasNext()) {
						i.next().data(s);
					}
				}
			}
		} catch (InterruptedException e) {
		}
	}

	boolean need_monitor() {
		return reader.opened() && !callbacks.isEmpty();
	}

	void maybe_stop_monitor() {
		if (!need_monitor() && monitor_thread != null) {
			monitor_thread.interrupt();
			try {
				monitor_thread.join();
			} catch (InterruptedException e) {
			} finally {
				monitor_thread = null;
			}
		}
	}

	void maybe_start_monitor() {
		if (need_monitor() && monitor_thread == null) {
			monitor_thread = new Thread(this);
			monitor_thread.start();
		}
	}

	public void monitor(AltosSerialMonitor monitor) {
		synchronized(callbacks) {
			callbacks.add(monitor);
			maybe_start_monitor();
		}
	}


	public void unmonitor(AltosSerialMonitor monitor) {
		synchronized(callbacks) {
			callbacks.remove(monitor);
			maybe_stop_monitor();
		}
	}

	public void close() {
		synchronized(callbacks) {
			reader.close();
			maybe_stop_monitor();
		}
	}

	public void open(File serial_name) throws FileNotFoundException {
		reader.open(serial_name);
		serial_out = new FileOutputStream(serial_name);
	}

	public void open(CommPort comm_port) throws IOException {
		reader.open(comm_port);
		serial_out = comm_port.getOutputStream();
	}

	public void connect(String port_name) throws IOException, NoSuchPortException, PortInUseException {
		System.out.printf("Opening serial port %s\n", port_name);
		CommPort comm_port = new RXTXPort(port_name);
//		CommPortIdentifier port_identifier = CommPortIdentifier.getPortIdentifier(port_name);
//		CommPort comm_port = port_identifier.open("Altos", 1000);
		open(comm_port);
	}

	void init() {
		reader = new AltosSerialReader();
		callbacks = new LinkedList<AltosSerialMonitor>();
	}

	public AltosSerial() {
		init();
	}

	public AltosSerial(File serial_name) throws FileNotFoundException {
		init();
		open(serial_name);
	}

	public AltosSerial(CommPort comm_port) throws IOException {
		init();
		open(comm_port);
	}
}
