/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_4;

import java.io.*;
import java.util.concurrent.*;
import java.util.*;

public abstract class AltosLink implements Runnable {

	public final static int ERROR = -1;
	public final static int TIMEOUT = -2;

	public abstract int getchar() throws InterruptedException;
	public abstract void print(String data) throws InterruptedException;
	public abstract void putchar(byte c);
	public abstract void close() throws InterruptedException;

	public static boolean debug = false;
	public static void set_debug(boolean in_debug) { debug = in_debug; }

	public boolean has_error;

	LinkedList<String> pending_output = new LinkedList<String>();

	public LinkedList<LinkedBlockingQueue<AltosLine>> monitors = new LinkedList<LinkedBlockingQueue<AltosLine>> ();;
	public LinkedBlockingQueue<AltosLine> reply_queue = new LinkedBlockingQueue<AltosLine>();
	public LinkedBlockingQueue<byte[]> binary_queue = new LinkedBlockingQueue<byte[]>();

	public synchronized void add_monitor(LinkedBlockingQueue<AltosLine> q) {
		set_monitor(true);
		monitors.add(q);
	}

	public synchronized void remove_monitor(LinkedBlockingQueue<AltosLine> q) {
		monitors.remove(q);
		if (monitors.isEmpty())
			set_monitor(false);
	}

	public void printf(String format, Object ... arguments) {
		String	line = String.format(format, arguments);
		if (debug)
			pending_output.add(line);
		try {
			print(line);
		} catch (InterruptedException ie) {

		}
	}

	public String get_reply_no_dialog(int timeout) throws InterruptedException, TimeoutException {
		flush_output();
		AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
		if (line != null)
			return line.line;
		return null;
	}

	public String get_reply() throws InterruptedException {
		return get_reply(5000);
	}


	public abstract boolean can_cancel_reply();
	public abstract boolean show_reply_timeout();
	public abstract void hide_reply_timeout();

	public boolean	reply_abort;
	public int	in_reply;

	boolean		reply_timeout_shown = false;

	private boolean check_reply_timeout() {
		if (!reply_timeout_shown)
			reply_timeout_shown = show_reply_timeout();
		return reply_abort;
	}

	private void cleanup_reply_timeout() {
		if (reply_timeout_shown) {
			reply_timeout_shown = false;
			hide_reply_timeout();
		}
	}

	private int	len_read = 0;

	public void run () {
		int c;
		byte[] line_bytes = null;
		int line_count = 0;

		try {
			for (;;) {
				c = getchar();
				if (Thread.interrupted()) {
					break;
				}
				if (c == ERROR) {
					if (debug)
						System.out.printf("ERROR\n");
					has_error = true;
					add_telem (new AltosLine());
					add_reply (new AltosLine());
					break;
				}
				if (c == TIMEOUT) {
					if (debug)
						System.out.printf("TIMEOUT\n");
					continue;
				}
				if (c == '\r' && len_read == 0)
					continue;
				synchronized(this) {
					if (c == '\n' && len_read == 0) {
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
						if (len_read !=0 && line_count == len_read) {
							add_binary(line_bytes, line_count);
							line_count = 0;
							len_read = 0;
						}
					}
				}
			}
		} catch (InterruptedException e) {
		}
	}


	public String get_reply(int timeout) throws InterruptedException {
		boolean	can_cancel = can_cancel_reply();
		String	reply = null;

		if (!can_cancel && remote)
			System.out.printf("Uh-oh, reading remote serial device from swing thread\n");

		if (remote && can_cancel)
			timeout = 500;
		try {
			++in_reply;

			flush_output();

			reply_abort = false;
			reply_timeout_shown = false;
			for (;;) {
				AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
				if (line != null) {
					cleanup_reply_timeout();
					reply = line.line;
					break;
				}
				if (!remote || !can_cancel || check_reply_timeout()) {
					reply = null;
					break;
				}
			}
		} finally {
			--in_reply;
		}
		return reply;
	}

	public byte[] get_binary_reply(int timeout, int len) throws InterruptedException {
		boolean	can_cancel = can_cancel_reply();
		byte[] bytes = null;

		synchronized(this) {
			len_read = len;
		}
		try {
			++in_reply;

			flush_output();

			reply_abort = false;
			reply_timeout_shown = false;
			for (;;) {
				bytes = binary_queue.poll(timeout, TimeUnit.MILLISECONDS);
				if (bytes != null) {
					cleanup_reply_timeout();
					break;
				}
				if (!remote || !can_cancel || check_reply_timeout()) {
					bytes = null;
					break;
				}
			}

		} finally {
			--in_reply;
		}
		return bytes;
	}

	public void add_telem(AltosLine line) throws InterruptedException {
		for (int e = 0; e < monitors.size(); e++) {
			LinkedBlockingQueue<AltosLine> q = monitors.get(e);
			q.put(line);
		}
	}

	public void add_reply(AltosLine line) throws InterruptedException {
		reply_queue.put (line);
	}

	public void abort_reply() {
		try {
			add_telem (new AltosLine());
			add_reply (new AltosLine());
		} catch (InterruptedException ie) {
		}
	}

	public void add_string(String line) throws InterruptedException {
		if (line.startsWith("TELEM") || line.startsWith("VERSION") || line.startsWith("CRC")) {
			add_telem(new AltosLine(line));
		} else {
			add_reply(new AltosLine(line));
		}
	}

	public void add_bytes(byte[] bytes, int len) throws InterruptedException {
		String	line;
		line = new String(bytes, 0, len, AltosLib.unicode_set);
		if (debug)
			System.out.printf("\t\t\t\t\t%s\n", line);
		add_string(line);
	}

	public void add_binary(byte[] bytes, int len) throws InterruptedException {
		byte[] dup = new byte[len];

		if (debug)
			System.out.printf ("\t\t\t\t\t%d:", len);
		for(int i = 0; i < len; i++) {
			dup[i] = bytes[i];
			if (debug)
				System.out.printf(" %02x", dup[i]);
		}
		if (debug)
			System.out.printf("\n");

		binary_queue.put(dup);
	}

	public void flush_output() {
		for (String s : pending_output)
			System.out.print(s);
		pending_output.clear();
	}

	public void flush_input(int timeout) throws InterruptedException {
		flush_output();
		boolean	got_some;

		do {
			Thread.sleep(timeout);
			got_some = !reply_queue.isEmpty();
			reply_queue.clear();
		} while (got_some);
	}


	public void flush_input() throws InterruptedException {
		if (remote)
			flush_input(500);
		else
			flush_input(100);
	}


	/*
	 * Various command-level operations on
	 * the link
	 */
	public boolean monitor_mode = false;
	public int telemetry = AltosLib.ao_telemetry_standard;
	public double frequency;
	public String callsign;
	AltosConfigData	config_data;

	private Object config_data_lock = new Object();

	private int telemetry_len() {
		return AltosLib.telemetry_len(telemetry);
	}

	private void set_radio_freq(int frequency) {
		if (monitor_mode)
			printf("m 0\nc F %d\nm %x\n",
			       frequency, telemetry_len());
		else
			printf("c F %d\n", frequency);
		flush_output();
	}

	public void set_radio_frequency(double frequency,
					boolean has_frequency,
					boolean has_setting,
					int cal) {
		if (debug)
			System.out.printf("set_radio_frequency %7.3f (freq %b) (set %b) %d\n", frequency, has_frequency, has_setting, cal);
		if (frequency == 0)
			return;
		if (has_frequency)
			set_radio_freq((int) Math.floor (frequency * 1000));
		else if (has_setting)
			set_radio_setting(AltosConvert.radio_frequency_to_setting(frequency, cal));
		else
			set_channel(AltosConvert.radio_frequency_to_channel(frequency));
	}

	public void set_radio_frequency(double in_frequency) throws InterruptedException, TimeoutException {
		frequency = in_frequency;
		config_data();
		set_radio_frequency(frequency,
				    config_data.radio_frequency > 0,
				    config_data.radio_setting > 0,
				    config_data.radio_calibration);
	}

	public void set_telemetry(int in_telemetry) {
		telemetry = in_telemetry;
		if (monitor_mode)
			printf("m 0\nm %x\n", telemetry_len());
		flush_output();
	}

	public void set_monitor(boolean monitor) {
		monitor_mode = monitor;
		if (monitor)
			printf("m %x\n", telemetry_len());
		else
			printf("m 0\n");
		flush_output();
	}

	private void set_channel(int channel) {
		if (monitor_mode)
			printf("m 0\nc r %d\nm %x\n",
			       channel, telemetry_len());
		else
			printf("c r %d\n", channel);
		flush_output();
	}

	private void set_radio_setting(int setting) {
		if (monitor_mode)
			printf("m 0\nc R %d\nm %x\n",
			       setting, telemetry_len());
		else
			printf("c R %d\n", setting);
		flush_output();
	}

	public AltosConfigData config_data() throws InterruptedException, TimeoutException {
		synchronized(config_data_lock) {
			if (config_data == null)
				config_data = new AltosConfigData(this);
			return config_data;
		}
	}

	public void set_callsign(String callsign) {
		this.callsign = callsign;
		printf ("c c %s\n", callsign);
		flush_output();
	}

	public boolean is_loader() throws InterruptedException {
		boolean	ret = false;
		printf("v\n");
		for (;;) {
			String line = get_reply();

			if (line == null)
				return false;
			if (line.startsWith("software-version"))
				break;
			if (line.startsWith("altos-loader"))
				ret = true;
		}
		return ret;
	}

	public void to_loader() throws InterruptedException {
		printf("X\n");
		flush_output();
		close();
		Thread.sleep(1000);
	}

	public boolean remote;
	public int serial;
	public String name;

	public void start_remote() throws TimeoutException, InterruptedException {
		if (frequency == 0.0)
			frequency = AltosPreferences.frequency(serial);
		if (debug)
			System.out.printf("start remote %7.3f\n", frequency);
		set_radio_frequency(frequency);
		set_callsign(AltosPreferences.callsign());
		printf("p\nE 0\n");
		flush_input();
		remote = true;
	}

	public void stop_remote() throws InterruptedException {
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

	public int rssi() throws TimeoutException, InterruptedException {
		if (remote)
			return 0;
		printf("s\n");
		String line = get_reply_no_dialog(5000);
		if (line == null)
			throw new TimeoutException();
		String[] items = line.split("\\s+");
		if (items.length < 2)
			return 0;
		if (!items[0].equals("RSSI:"))
			return 0;
		int rssi = Integer.parseInt(items[1]);
		return rssi;
	}

	public String[] adc() throws TimeoutException, InterruptedException {
		printf("a\n");
		for (;;) {
			String line = get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (!line.startsWith("tick:"))
				continue;
			String[] items = line.split("\\s+");
			return items;
		}
	}

	public boolean has_monitor_battery() {
		return config_data.has_monitor_battery();
	}

	public double monitor_battery() throws InterruptedException {
		int monitor_batt = AltosLib.MISSING;

		if (config_data.has_monitor_battery()) {
			try {
			String[] items = adc();
			for (int i = 0; i < items.length;) {
				if (items[i].equals("batt")) {
					monitor_batt = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				i++;
			}
			} catch (TimeoutException te) {
			}
		}
		if (monitor_batt == AltosLib.MISSING)
			return AltosLib.MISSING;
		return AltosConvert.cc_battery_to_voltage(monitor_batt);
	}

	public AltosLink() {
		callsign = "";
		has_error = false;
	}
}
