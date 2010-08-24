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
import altosui.AltosSerialMonitor;
import libaltosJNI.libaltos;
import libaltosJNI.altos_device;
import libaltosJNI.SWIGTYPE_p_altos_file;
import libaltosJNI.SWIGTYPE_p_altos_list;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial implements Runnable {

	SWIGTYPE_p_altos_file altos;
	LinkedList<LinkedBlockingQueue<String>> monitors;
	LinkedBlockingQueue<String> reply_queue;
	Thread input_thread;
	String line;

	public void run () {
		int c;

		try {
			for (;;) {
				c = libaltos.altos_getchar(altos, 0);
				if (Thread.interrupted())
					break;
				if (c == -1)
					continue;
				if (c == '\r')
					continue;
				synchronized(this) {
					if (c == '\n') {
						if (line != "") {
							if (line.startsWith("VERSION")) {
								for (int e = 0; e < monitors.size(); e++) {
									LinkedBlockingQueue<String> q = monitors.get(e);
									q.put(line);
								}
							} else {
//								System.out.printf("GOT: %s\n", line);
								reply_queue.put(line);
							}
							line = "";
						}
					} else {
						line = line + (char) c;
					}
				}
			}
		} catch (InterruptedException e) {
		}
	}

	public void flush_reply() {
		libaltos.altos_flush(altos);
		try {
			Thread.sleep(100);
		} catch (InterruptedException ie) {
		}
		reply_queue.clear();
	}

	public String get_reply() throws InterruptedException {
		libaltos.altos_flush(altos);
		String line = reply_queue.take();
		return line;
	}

	public void add_monitor(LinkedBlockingQueue<String> q) {
		monitors.add(q);
	}

	public void remove_monitor(LinkedBlockingQueue<String> q) {
		monitors.remove(q);
	}

	public void flush () {
		synchronized(this) {
			if (!"VERSION".startsWith(line) && !line.startsWith("VERSION"))
				line = "";
			reply_queue.clear();
		}
	}

	public boolean opened() {
		return altos != null;
	}

	public void close() {
		if (altos != null)
			libaltos.altos_close(altos);
		if (input_thread != null) {
			try {
				input_thread.interrupt();
				input_thread.join();
			} catch (InterruptedException e) {
			}
			input_thread = null;
		}
		if (altos != null) {
			libaltos.altos_free(altos);
			altos = null;
		}
	}

	public void putc(char c) {
		if (altos != null)
			libaltos.altos_putchar(altos, c);
	}

	public void print(String data) {
//		System.out.printf("\"%s\" ", data);
		for (int i = 0; i < data.length(); i++)
			putc(data.charAt(i));
	}

	public void printf(String format, Object ... arguments) {
		print(String.format(format, arguments));
	}

	public void open(altos_device device) throws FileNotFoundException {
		close();
		altos = libaltos.altos_open(device);
		if (altos == null)
			throw new FileNotFoundException(device.getPath());
		input_thread = new Thread(this);
		input_thread.start();
		print("\nE 0\n");
		try {
			Thread.sleep(200);
		} catch (InterruptedException e) {
		}
		flush();
	}

	public void set_channel(int channel) {
		if (altos != null)
			printf("m 0\nc r %d\nm 1\n", channel);
	}

	public void set_callsign(String callsign) {
		if (altos != null)
			printf ("c c %s\n", callsign);
	}

	public AltosSerial() {
		altos = null;
		input_thread = null;
		line = "";
		monitors = new LinkedList<LinkedBlockingQueue<String>> ();
		reply_queue = new LinkedBlockingQueue<String> ();
	}
}
