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
import org.altusmetrum.AltosLib.*;

import libaltosJNI.*;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial extends AltosLink implements Runnable {

	static java.util.List<String> devices_opened = Collections.synchronizedList(new LinkedList<String>());

	AltosDevice device;
	SWIGTYPE_p_altos_file altos;
	Thread input_thread;
	String line;
	byte[] line_bytes;
	int line_count;
	Frame frame;

	public void run () {
		int c;
		byte[] line_bytes = null;
		int line_count = 0;

		try {
			for (;;) {
				c = libaltos.altos_getchar(altos, 0);
				if (Thread.interrupted())
					break;
				if (c == libaltosConstants.LIBALTOS_ERROR) {
					add_telem (new AltosLine());
					add_reply (new AltosLine());
					break;
				}
				if (c == libaltosConstants.LIBALTOS_TIMEOUT)
					continue;
				if (c == '\r')
					continue;
				synchronized(this) {
					if (c == '\n') {
						if (line_count != 0) {
							add_bytes(line_bytes, line_count);
							line_count = 0;
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
		super.flush_output();
		if (altos != null) {
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
		pane.setMessage(String.format("Connecting to %s, %7.3f MHz", device.toShortString(), frequency));
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

	public void flush_input() throws InterruptedException {
		if (remote)
			flush_input(500);
		else
			flush_input(100);
	}

	int	in_reply;

	public String get_reply(int timeout) throws InterruptedException {
		boolean	can_cancel = true;
		String	reply = null;

		try {
			++in_reply;

			if (SwingUtilities.isEventDispatchThread()) {
				can_cancel = false;
				if (remote)
					System.out.printf("Uh-oh, reading remote serial device from swing thread\n");
			}
			flush_output();
			if (remote && can_cancel) {
				timeout = 500;
			}
			abort = false;
			timeout_started = false;
			for (;;) {
				AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
				if (line != null) {
					stop_timeout_dialog();
					reply = line.line;
					break;
				}
				if (!remote || !can_cancel || check_timeout()) {
					reply = null;
					break;
				}
			}
		} finally {
			--in_reply;
		}
		return reply;
	}

	public void close() {
		if (remote) {
			try {
				stop_remote();
			} catch (InterruptedException ie) {
			}
		}
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
		for (int i = 0; i < data.length(); i++)
			putc(data.charAt(i));
	}

	private void open() throws FileNotFoundException, AltosSerialInUseException {
		synchronized (devices_opened) {
			if (devices_opened.contains(device.getPath()))
				throw new AltosSerialInUseException(device);
			devices_opened.add(device.getPath());
		}
		altos = device.open();
		if (altos == null) {
			final String	message = device.getErrorString();
			close();
			throw new FileNotFoundException(String.format("%s (%s)",
								      device.toShortString(), message));
		}
		if (debug)
			System.out.printf("Open %s\n", device.getPath());
		input_thread = new Thread(this);
		input_thread.start();
		print("~\nE 0\n");
		set_monitor(false);
		flush_output();
	}

	public void set_frame(Frame in_frame) {
		frame = in_frame;
	}

	public AltosSerial(AltosDevice in_device) throws FileNotFoundException, AltosSerialInUseException {
		device = in_device;
		frame = null;
		serial = device.getSerial();
		name = device.toShortString();
		open();
	}
}
