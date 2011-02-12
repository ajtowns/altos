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
import java.util.concurrent.*;
import java.util.*;

import libaltosJNI.*;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial implements Runnable {

	static List<String> devices_opened = Collections.synchronizedList(new LinkedList<String>());

	AltosDevice device;
	SWIGTYPE_p_altos_file altos;
	LinkedList<LinkedBlockingQueue<AltosLine>> monitors;
	LinkedBlockingQueue<AltosLine> reply_queue;
	Thread input_thread;
	String line;
	byte[] line_bytes;
	int line_count;
	boolean monitor_mode;
	static boolean debug;
	boolean remote;
	LinkedList<String> pending_output = new LinkedList<String>();

	static void set_debug(boolean new_debug) {
		debug = new_debug;
	}

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
							if (debug)
								System.out.printf("\t\t\t\t\t%s\n", line);
							if (line.startsWith("VERSION") || line.startsWith("CRC")) {
								for (int e = 0; e < monitors.size(); e++) {
									LinkedBlockingQueue<AltosLine> q = monitors.get(e);
									q.put(new AltosLine (line));
								}
							} else {
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
		if (altos != null) {
			for (String s : pending_output)
				System.out.print(s);
			pending_output.clear();
			libaltos.altos_flush(altos);
		}
	}

	public void flush_input() {
		flush_output();
		boolean	got_some;

		int timeout = 100;
		if (remote)
			timeout = 300;
		do {
			try {
				Thread.sleep(timeout);
			} catch (InterruptedException ie) {
			}
			got_some = !reply_queue.isEmpty();
			synchronized(this) {
				if (!"VERSION".startsWith(line) &&
				    !line.startsWith("VERSION"))
					line = "";
				reply_queue.clear();
			}
		} while (got_some);
	}

	public String get_reply() throws InterruptedException {
		flush_output();
		AltosLine line = reply_queue.take();
		return line.line;
	}

	public String get_reply(int timeout) throws InterruptedException {
		flush_output();
		AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
		if (line == null)
			return null;
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
		synchronized (devices_opened) {
			devices_opened.remove(device.getPath());
		}
		if (debug)
			System.out.printf("Closing %s\n", device.getPath());
	}

	private void putc(char c) {
		if (altos != null)
			libaltos.altos_putchar(altos, c);
	}

	public void print(String data) {
		if (debug)
			pending_output.add(data);
		for (int i = 0; i < data.length(); i++)
			putc(data.charAt(i));
	}

	public void printf(String format, Object ... arguments) {
		print(String.format(format, arguments));
	}

	private void open() throws FileNotFoundException, AltosSerialInUseException {
		synchronized (devices_opened) {
			if (devices_opened.contains(device.getPath()))
				throw new AltosSerialInUseException(device);
			devices_opened.add(device.getPath());
		}
		altos = libaltos.altos_open(device);
		if (altos == null) {
			close();
			throw new FileNotFoundException(device.toShortString());
		}
		if (debug)
			System.out.printf("Open %s\n", device.getPath());
		input_thread = new Thread(this);
		input_thread.start();
		print("~\nE 0\n");
		set_monitor(false);
		flush_output();
	}

	public void set_radio() {
		set_channel(AltosPreferences.channel(device.getSerial()));
		set_callsign(AltosPreferences.callsign());
	}

	public void set_channel(int channel) {
		if (altos != null) {
			if (monitor_mode)
				printf("m0\nc r %d\nm1\n", channel);
			else
				printf("c r %d\n", channel);
			flush_output();
		}
	}

	void set_monitor(boolean monitor) {
		monitor_mode = monitor;
		if (altos != null) {
			if (monitor)
				printf("m1\n");
			else
				printf("m0\n");
			flush_output();
		}
	}

	public void set_callsign(String callsign) {
		if (altos != null) {
			printf ("c c %s\n", callsign);
			flush_output();
		}
	}

	public void start_remote() {
		if (debug)
			System.out.printf("start remote\n");
		set_radio();
		printf("p\nE 0\n");
		flush_input();
		remote = true;
	}

	public void stop_remote() {
		if (debug)
			System.out.printf("stop remote\n");
		flush_input();
		printf ("~");
		flush_output();
		remote = false;
	}

	public AltosSerial(AltosDevice in_device) throws FileNotFoundException, AltosSerialInUseException {
		device = in_device;
		line = "";
		monitor_mode = false;
		monitors = new LinkedList<LinkedBlockingQueue<AltosLine>> ();
		reply_queue = new LinkedBlockingQueue<AltosLine> ();
		open();
	}
}
