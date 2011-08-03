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
import java.text.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;

import libaltosJNI.*;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial implements Runnable {

	static java.util.List<String> devices_opened = Collections.synchronizedList(new LinkedList<String>());

	AltosDevice device;
	SWIGTYPE_p_altos_file altos;
	LinkedList<LinkedBlockingQueue<AltosLine>> monitors;
	LinkedBlockingQueue<AltosLine> reply_queue;
	Thread input_thread;
	String line;
	byte[] line_bytes;
	int line_count;
	boolean monitor_mode;
	int telemetry;
	int channel;
	static boolean debug;
	boolean remote;
	LinkedList<String> pending_output = new LinkedList<String>();
	Frame frame;

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
							if (line.startsWith("TELEM") || line.startsWith("VERSION") || line.startsWith("CRC")) {
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

	boolean		abort;
	JDialog		timeout_dialog;
	boolean	timeout_started = false;

	private void stop_timeout_dialog() {
		if (timeout_started) {
			timeout_started = false;
			Runnable r = new Runnable() {
					public void run() {
						timeout_dialog.setVisible(false);
					}
				};
			SwingUtilities.invokeLater(r);
		}
	}

	private void start_timeout_dialog_internal() {

		Object[] options = { "Cancel" };

		JOptionPane	pane = new JOptionPane();
		pane.setMessage(String.format("Connecting to %s", device.getPath()));
		pane.setOptions(options);
		pane.setInitialValue(null);

		timeout_dialog = pane.createDialog(frame, "Connecting...");

		timeout_dialog.setVisible(true);

		Object o = pane.getValue();
		if (o == null)
			return;
		if (options[0].equals(o))
			abort = true;
		timeout_dialog.dispose();
		timeout_dialog = null;
	}

	private boolean check_timeout() {
		if (!timeout_started && frame != null) {
			if (!SwingUtilities.isEventDispatchThread()) {
				timeout_started = true;
				Runnable r = new Runnable() {
						public void run() {
							start_timeout_dialog_internal();
						}
					};
				SwingUtilities.invokeLater(r);
			}
		}
		return abort;
	}

	public void flush_input() {
		flush_output();
		boolean	got_some;

		int timeout = 100;
		if (remote)
			timeout = 500;
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
		if (SwingUtilities.isEventDispatchThread())
			System.out.printf("Uh-oh, reading serial device from swing thread\n");
		flush_output();
		AltosLine line = reply_queue.take();
		return line.line;
	}

	int	in_reply;

	public String get_reply(int timeout) throws InterruptedException {
		boolean	can_cancel = true;
		++in_reply;

		if (SwingUtilities.isEventDispatchThread()) {
			can_cancel = false;
			System.out.printf("Uh-oh, reading serial device from swing thread\n");
		}
		flush_output();
		if (remote && can_cancel) {
			timeout = 500;
		}
		abort = false;
		timeout_started = false;
		for (;;) {
			System.out.printf("timeout %d\n", timeout);
			AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
			if (line != null) {
				stop_timeout_dialog();
				--in_reply;
				return line.line;
			}
			System.out.printf("no line remote %b can_cancel %b\n", remote, can_cancel);
			if (!remote || !can_cancel || check_timeout()) {
				--in_reply;
				return null;
			}
		}
	}

	public String get_reply_no_dialog(int timeout) throws InterruptedException, TimeoutException {
		flush_output();
		AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
		if (line != null)
			return line.line;
		return null;
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
		if (in_reply != 0)
			System.out.printf("Uh-oh. Closing active serial device\n");

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
		altos = device.open();
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
		telemetry = AltosPreferences.telemetry(device.getSerial());
		channel = AltosPreferences.channel(device.getSerial());
		set_channel(channel);
		set_callsign(AltosPreferences.callsign());
	}

	private int telemetry_len() {
		return Altos.telemetry_len(telemetry);
	}

	public void set_channel(int in_channel) {
		channel = in_channel;
		if (altos != null) {
			if (monitor_mode)
				printf("m 0\nc r %d\nm %x\n",
				       channel, telemetry_len());
			else
				printf("c r %d\n", channel);
			flush_output();
		}
	}

	public void set_telemetry(int in_telemetry) {
		telemetry = in_telemetry;
		if (altos != null) {
			if (monitor_mode)
				printf("m 0\nm %x\n", telemetry_len());
			flush_output();
		}
	}

	void set_monitor(boolean monitor) {
		monitor_mode = monitor;
		if (altos != null) {
			if (monitor)
				printf("m %x\n", telemetry_len());
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
		try {
			flush_input();
		} finally {
			printf ("~\n");
			flush_output();
		}
		remote = false;
	}

	public void set_frame(Frame in_frame) {
		frame = in_frame;
	}

	public AltosSerial(AltosDevice in_device) throws FileNotFoundException, AltosSerialInUseException {
		device = in_device;
		line = "";
		monitor_mode = false;
		frame = null;
		telemetry = Altos.ao_telemetry_standard;
		monitors = new LinkedList<LinkedBlockingQueue<AltosLine>> ();
		reply_queue = new LinkedBlockingQueue<AltosLine> ();
		open();
	}
}
