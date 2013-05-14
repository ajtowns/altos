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

package org.altusmetrum.altoslib_1;

import java.io.*;
import java.util.concurrent.*;


public class AltosIdleMonitor extends Thread {
	AltosLink		link;
	AltosIdleMonitorListener	listener;
	AltosState		state;
	boolean			remote;
	double			frequency;
	String			callsign;
	AltosState		previous_state;
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

	boolean has_sensor_tm(AltosConfigData config_data) {
		return config_data.product.startsWith("TeleMetrum") || config_data.product.startsWith("TeleMini");
	}

	boolean has_sensor_mm(AltosConfigData config_data) {
		return config_data.product.startsWith("TeleMega");
	}

	boolean has_gps(AltosConfigData config_data) {
		return config_data.product.startsWith("TeleMetrum") || config_data.product.startsWith("TeleMega");
	}

	AltosRecord sensor_mm(AltosConfigData config_data) throws InterruptedException, TimeoutException {
		AltosRecordMM record_mm = new AltosRecordMM();
		AltosSensorMM sensor = new AltosSensorMM(link);
		AltosMs5607 ms5607 = new AltosMs5607Query(link);
		AltosIMU imu = new AltosIMUQuery(link);

		record_mm.accel_plus_g = config_data.accel_cal_plus;
		record_mm.accel_minus_g = config_data.accel_cal_minus;

		record_mm.ground_accel = sensor.accel;
		record_mm.accel = sensor.accel;
		record_mm.ground_pres = ms5607.pa;
		record_mm.pres = ms5607.pa;
		record_mm.temp = ms5607.cc;

		record_mm.v_batt = sensor.v_batt;
		record_mm.v_pyro = sensor.v_pyro;
		record_mm.sense = sensor.sense;

		record_mm.imu = imu;

		return record_mm;
	}

	void update_state() throws InterruptedException, TimeoutException {
		AltosRecord	record = null;

		try {
			if (remote) {
				link.set_radio_frequency(frequency);
				link.set_callsign(callsign);
				link.start_remote();
			} else
				link.flush_input();
			config_data = new AltosConfigData(link);

			if (has_sensor_tm(config_data))
				record = new AltosSensorTM(link, config_data);
			else if (has_sensor_mm(config_data))
				record = sensor_mm(config_data);
			else
				record = new AltosRecordNone();

			if (has_gps(config_data))
				gps = new AltosGPSQuery(link, config_data);

			record.version = 0;
			record.callsign = config_data.callsign;
			record.serial = config_data.serial;
			record.flight = config_data.log_available() > 0 ? 255 : 0;
			record.status = 0;
			record.state = AltosLib.ao_flight_idle;
			record.gps = gps;
			record.gps_sequence++;
			state = new AltosState (record, state);
		} finally {
			if (remote) {
				link.stop_remote();
				if (record != null) {
					record.rssi = link.rssi();
					listener_state.battery = link.monitor_battery();
				}
			} else {
				if (record != null)
					record.rssi = 0;
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

	public void post_state() {
		listener.update(state, listener_state);
	}

	public void abort() {
		if (isAlive()) {
			interrupt();
			link.abort_reply();
			try {
				join();
			} catch (InterruptedException ie) {
			}
		}
	}

	public void run() {
		try {
			for (;;) {
				try {
					update_state();
					post_state();
				} catch (TimeoutException te) {
				}
				Thread.sleep(1000);
			}
		} catch (InterruptedException ie) {
			link.close();
		}
	}

	public AltosIdleMonitor(AltosIdleMonitorListener in_listener, AltosLink in_link, boolean in_remote)
		throws FileNotFoundException, InterruptedException, TimeoutException {
		listener = in_listener;
		link = in_link;
		remote = in_remote;
		state = null;
		listener_state = new AltosListenerState();
	}
}
