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
import altosui.AltosLine;
import libaltosJNI.libaltos;
import libaltosJNI.altos_device;
import libaltosJNI.SWIGTYPE_p_altos_file;
import libaltosJNI.SWIGTYPE_p_altos_list;
import libaltosJNI.libaltosConstants;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial implements Runnable {

	SWIGTYPE_p_altos_file altos;
	LinkedList<LinkedBlockingQueue<AltosLine>> monitors;
	LinkedBlockingQueue<AltosLine> reply_queue;
	Thread input_thread;
	String line;
	byte[] line_bytes;
	int line_count;
	boolean monitor_mode;

	public void run () {
		int c;

		try {
			for (;;) {
				c = libaltos.altos_getchar(altos, 0);
				if (Thread.interrupted())
					break;
				if (c == libaltosConstants.LIBALTOS_ERROR) {
					for (int e = 0; e < monitors.size(); e++) {
						LinkedBlockingQueue<AltosLine> q = monitors.get(e);
						q.put(new AltosLine());
					}
					reply_queue.put (new AltosLine());
					break;
				}
				if (c == libaltosConstants.LIBALTOS_TIMEOUT)
					continue;
				if (c == '\r')
					continue;
				synchronized(this) {
					if (c == '\n') {
						if (line_count != 0) {
							try {
								line = new String(line_bytes, 0, line_count, "UTF-8");
							} catch (UnsupportedEncodingException ue) {
								line = "";
								for (int i = 0; i < line_count; i++)
									line = line + line_bytes[i];
							}
							if (line.startsWith("VERSION") || line.startsWith("CRC")) {
								for (int e = 0; e < monitors.size(); e++) {
									LinkedBlockingQueue<AltosLine> q = monitors.get(e);
									q.put(new AltosLine (line));
								}
							} else {
//								System.out.printf("GOT: %s\n", line);
								reply_queue.put(new AltosLine (line));
							}
							line_count = 0;
							line = "";
						}
					} else {
						if (line_bytes == null) {
							line_bytes = new byte[256];
						} else if (line_count == line_bytes.length) {
							byte[] new_line_bytes = new byte[line_count * 2];
							System.arraycopy(line_bytes, 0, new_line_bytes, 0, line_count);
							line_bytes = new_line_bytes;
						}
						line_bytes[line_count] = (byte) c;
						line_count++;
					}
				}
			}
		} catch (InterruptedException e) {
		}
	}

	public void flush_output() {
		if (altos != null)
			libaltos.altos_flush(altos);
	}

	public void flush_input() {
		flush_output();
		try {
			Thread.sleep(200);
		} catch (InterruptedException ie) {
		}
		synchronized(this) {
			if (!"VERSION".startsWith(line) &&
			    !line.startsWith("VERSION"))
				line = "";
			reply_queue.clear();
		}
	}

	public String get_reply() throws InterruptedException {
		flush_output();
		AltosLine line = reply_queue.take();
		return line.line;
	}

	public void add_monitor(LinkedBlockingQueue<AltosLine> q) {
		set_monitor(true);
		monitors.add(q);
	}

	public void remove_monitor(LinkedBlockingQueue<AltosLine> q) {
		monitors.remove(q);
		if (monitors.isEmpty())
			set_monitor(false);
	}

	public boolean opened() {
		return altos != null;
	}

	public void close() {
		if (altos != null) {
			libaltos.altos_close(altos);
		}
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
		print("~\nE 0\n");
		flush_output();
		set_monitor(monitor_mode);
		set_channel(AltosPreferences.channel());
		set_callsign(AltosPreferences.callsign());
	}

	public void set_channel(int channel) {
		if (altos != null) {
			if (monitor_mode)
				printf("m 0\nc r %d\nm 1\n", channel);
			else
				printf("c r %d\n", channel);
			flush_output();
		}
	}

	void set_monitor(boolean monitor) {
		monitor_mode = monitor;
		if (altos != null) {
			if (monitor)
				printf("m 1\n");
			else
				printf("m 0\n");
			flush_output();
		}
	}

	public void set_callsign(String callsign) {
		if (altos != null) {
			printf ("c c %s\n", callsign);
			flush_output();
		}
	}

	public AltosSerial() {
		altos = null;
		input_thread = null;
		line = "";
		monitor_mode = false;
		monitors = new LinkedList<LinkedBlockingQueue<AltosLine>> ();
		reply_queue = new LinkedBlockingQueue<AltosLine> ();
	}
}
