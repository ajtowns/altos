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

package org.altusmetrum.altosuilib_2;

import java.io.*;
import java.util.*;
import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_4.*;
import libaltosJNI.*;

/*
 * This class reads from the serial port and places each received
 * line in a queue. Dealing with that queue is left up to other
 * threads.
 */

public class AltosSerial extends AltosLink  {

	static java.util.List<String> devices_opened = Collections.synchronizedList(new LinkedList<String>());

	public AltosDevice device;
	SWIGTYPE_p_altos_file altos;
	Thread input_thread;
	String line;
	byte[] line_bytes;
	int line_count;
	Frame frame;

	public int getchar() {
		if (altos == null)
			return ERROR;
		return libaltos.altos_getchar(altos, 0);
	}

	public void flush_output() {
		super.flush_output();
		if (altos != null) {
			if (libaltos.altos_flush(altos) != 0)
				close_serial();
		}
	}

	JDialog		timeout_dialog;

	private void start_timeout_dialog_internal() {

		Object[] options = { "Cancel" };

		JOptionPane	pane = new JOptionPane();
		pane.setMessage(String.format("Connecting to %s, %7.3f MHz as %s", device.toShortString(), frequency, callsign));
		pane.setOptions(options);
		pane.setInitialValue(null);

		timeout_dialog = pane.createDialog(frame, "Connecting...");

		timeout_dialog.setVisible(true);

		Object o = pane.getValue();
		if (o == null)
			return;
		if (options[0].equals(o))
			reply_abort = true;
		timeout_dialog.dispose();
		timeout_dialog = null;
	}

	/*
	 * These are required by the AltosLink implementation
	 */

	public boolean can_cancel_reply() {
		/*
		 * Can cancel any replies not called from the dispatch thread
		 */
		return !SwingUtilities.isEventDispatchThread();
	}

	public boolean show_reply_timeout() {
		if (!SwingUtilities.isEventDispatchThread() && frame != null) {
			Runnable r = new Runnable() {
					public void run() {
						start_timeout_dialog_internal();
					}
				};
			SwingUtilities.invokeLater(r);
			return true;
		}
		return false;
	}

	public void hide_reply_timeout() {
		Runnable r = new Runnable() {
				public void run() {
					timeout_dialog.setVisible(false);
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private synchronized void close_serial() {
		synchronized (devices_opened) {
			devices_opened.remove(device.getPath());
		}
		if (altos != null) {
			libaltos.altos_free(altos);
			altos = null;
		}
		abort_reply();
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

		close_serial();

		if (input_thread != null) {
			try {
				input_thread.interrupt();
				input_thread.join();
			} catch (InterruptedException ie) {
			}
			input_thread = null;
		}
		if (debug)
			System.out.printf("Closing %s\n", device.getPath());
	}

	private void putc(char c) {
		if (altos != null)
			if (libaltos.altos_putchar(altos, c) != 0)
				close_serial();
	}

	public void putchar(byte c) {
		if (altos != null) {
			if (debug)
				System.out.printf(" %02x", (int) c & 0xff);
			if (libaltos.altos_putchar(altos, (char) c) != 0)
				close_serial();
		}
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
