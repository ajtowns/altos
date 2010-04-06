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
			for (;;) {
				c = serial_in.read();
				if (Thread.interrupted())
					break;
				if (c == -1)
					continue;
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
		String s = monitor_queue.take();
		System.out.println(s);
		return s;
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
				input_thread.interrupt();
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
		try {
		c.enableReceiveTimeout(1000);	/* icky. the read method cannot be interrupted */
		} catch (UnsupportedCommOperationException ee) {
		}
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

public class AltosSerial {
	OutputStream serial_out = null;
	AltosSerialReader reader = null;

	public String get_telem() throws InterruptedException {
		return reader.get_telem();
	}

	CommPort comm_port = null;

	public void close() {
		try {
			serial_out.close();
		} catch (IOException ee) {
		}
		reader.close();
		if (comm_port != null) {
			comm_port.close();
		}
	}

	public void open(File serial_name) throws FileNotFoundException {
		reader.open(serial_name);
		serial_out = new FileOutputStream(serial_name);
	}

	public void open(CommPort c) throws IOException {
		reader.open(c);
		serial_out = c.getOutputStream();
	}

	public void connect(String port_name) throws IOException, NoSuchPortException, PortInUseException {
		comm_port = new RXTXPort(port_name);
		open(comm_port);
	}

	void init() {
		reader = new AltosSerialReader();
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
