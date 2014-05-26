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

package org.altusmetrum.altoslib_4;

import java.io.*;
import java.util.concurrent.*;


public class AltosIdleMonitor extends Thread {
	AltosLink		link;
	AltosIdleMonitorListener	listener;

	AltosIdleFetch		fetch;

	boolean			remote;
	double			frequency;
	String			callsign;

	AltosListenerState	listener_state;
	AltosConfigData		config_data;
	AltosGPS		gps;

	int AltosRSSI() throws TimeoutException, InterruptedException {
		link.printf("s\n");
		String line = link.get_reply_no_dialog(5000);
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

	void start_link() throws InterruptedException, TimeoutException {
		if (remote) {
			link.set_radio_frequency(frequency);
			link.set_callsign(callsign);
			link.start_remote();
		} else
			link.flush_input();
	}

	void stop_link() throws InterruptedException, TimeoutException {
		if (remote)
			link.stop_remote();
	}

	void update_state(AltosState state) throws InterruptedException, TimeoutException {
		boolean		worked = false;

		try {
			start_link();
			fetch.update_state(state);
			worked = true;
		} finally {
			stop_link();
			if (worked) {
				if (remote)
					state.set_rssi(link.rssi(), 0);
				listener_state.battery = link.monitor_battery();
			}
		}
	}

	public void set_frequency(double in_frequency) {
		frequency = in_frequency;
		link.abort_reply();
	}

	public void set_callsign(String in_callsign) {
		callsign = in_callsign;
		link.abort_reply();
	}

	public void abort() throws InterruptedException {
		while (isAlive()) {
			interrupt();
			link.abort_reply();
			Thread.sleep(100);
		}
		join();
	}

	public void run() {
		AltosState state = new AltosState();
		try {
			while (!link.has_error) {
				try {
					link.config_data();
					update_state(state);
					listener.update(state, listener_state);
				} catch (TimeoutException te) {
				}
				Thread.sleep(1000);
			}
		} catch (InterruptedException ie) {
		}
		try {
			link.close();
		} catch (InterruptedException ie) {
		}
	}

	public AltosIdleMonitor(AltosIdleMonitorListener in_listener, AltosLink in_link, boolean in_remote)
		throws FileNotFoundException, InterruptedException, TimeoutException {
		listener = in_listener;
		link = in_link;
		remote = in_remote;
		listener_state = new AltosListenerState();
		fetch = new AltosIdleFetch(link);
	}
}
