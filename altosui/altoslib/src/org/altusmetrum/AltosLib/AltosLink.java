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

package org.altusmetrum.AltosLib;

import java.lang.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import java.text.*;

public abstract class AltosLink {
	public abstract void print(String data);
	public abstract void close();

	public static boolean debug = false;
	public static void set_debug(boolean in_debug) { debug = in_debug; }
	LinkedList<String> pending_output = new LinkedList<String>();

	public LinkedList<LinkedBlockingQueue<AltosLine>> monitors = new LinkedList<LinkedBlockingQueue<AltosLine>> ();;
	public LinkedBlockingQueue<AltosLine> reply_queue = new LinkedBlockingQueue<AltosLine>();

	public void add_monitor(LinkedBlockingQueue<AltosLine> q) {
		set_monitor(true);
		monitors.add(q);
	}

	public void remove_monitor(LinkedBlockingQueue<AltosLine> q) {
		monitors.remove(q);
		if (monitors.isEmpty())
			set_monitor(false);
	}

	public void printf(String format, Object ... arguments) {
		String	line = String.format(format, arguments);
		if (debug)
			pending_output.add(line);
		print(line);
	}

	public String get_reply_no_dialog(int timeout) throws InterruptedException, TimeoutException {
		flush_output();
		AltosLine line = reply_queue.poll(timeout, TimeUnit.MILLISECONDS);
		if (line != null)
			return line.line;
		return null;
	}

	public String get_reply(int timeout) throws InterruptedException {
		try {
			return get_reply_no_dialog(timeout);
		} catch (TimeoutException te) {
			return null;
		}
	}

	public String get_reply() throws InterruptedException {
		return get_reply(5000);
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

	public void add_string(String line) throws InterruptedException {
		if (line.startsWith("TELEM") || line.startsWith("VERSION") || line.startsWith("CRC")) {
			add_telem(new AltosLine(line));
		} else {
			add_reply(new AltosLine(line));
		}
	}

	public void add_bytes(byte[] bytes, int len) throws InterruptedException {
		String	line;
		try {
			line = new String(bytes, 0, len, "UTF-8");
		} catch (UnsupportedEncodingException ue) {
			line = "";
			for (int i = 0; i < len; i++)
				line = line + bytes[i];
		}
		if (debug)
			System.out.printf("\t\t\t\t\t%s\n", line);
		add_string(line);
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
		flush_input(100);
	}


	/*
	 * Various command-level operations on
	 * the link
	 */
	public boolean monitor_mode = false;
	public int telemetry = AltosLib.ao_telemetry_standard;
	public double frequency;
	AltosConfigData	config_data;

	private int telemetry_len() {
		return AltosLib.telemetry_len(telemetry);
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

	public void set_radio_frequency(double frequency,
					boolean has_setting,
					int cal) {
		if (debug)
			System.out.printf("set_radio_frequency %7.3f %b %d\n", frequency, has_setting, cal);
		if (has_setting)
			set_radio_setting(AltosConvert.radio_frequency_to_setting(frequency, cal));
		else
			set_channel(AltosConvert.radio_frequency_to_channel(frequency));
	}

	public AltosConfigData config_data() throws InterruptedException, TimeoutException {
		if (config_data == null)
			config_data = new AltosConfigData(this);
		return config_data;
	}

	public void set_radio_frequency(double in_frequency) throws InterruptedException, TimeoutException {
		frequency = in_frequency;
		config_data();
		set_radio_frequency(frequency,
				    config_data.radio_setting != 0,
				    config_data.radio_calibration);
	}

	public void set_callsign(String callsign) {
		printf ("c c %s\n", callsign);
		flush_output();
	}

	public boolean remote;
	public int serial;
	public String name;

	public void start_remote() throws TimeoutException, InterruptedException {
		if (debug)
			System.out.printf("start remote %7.3f\n", frequency);
		if (frequency == 0.0)
			frequency = AltosPreferences.frequency(serial);
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

	public AltosLink() {
	}
}
