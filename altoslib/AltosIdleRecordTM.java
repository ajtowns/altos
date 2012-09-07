/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

import java.util.concurrent.*;

class AltosADCTM {
	int	tick;
	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;

	public AltosADCTM(AltosLink link) throws InterruptedException, TimeoutException {
		link.printf("a\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (!line.startsWith("tick:"))
				continue;
			String[] items = line.split("\\s+");
			for (int i = 0; i < items.length;) {
				if (items[i].equals("tick:")) {
					tick = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("accel:")) {
					accel = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("pres:")) {
					pres = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("temp:")) {
					temp = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("batt:")) {
					batt = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("drogue:")) {
					drogue = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				if (items[i].equals("main:")) {
					main = Integer.parseInt(items[i+1]);
					i += 2;
					continue;
				}
				i++;
			}
			break;
		}
	}
}

class AltosGPSQuery extends AltosGPS {
	public AltosGPSQuery (AltosLink link, AltosConfigData config_data)
		throws TimeoutException, InterruptedException {
		boolean says_done = config_data.compare_version("1.0") >= 0;
		link.printf("g\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();
			String[] bits = line.split("\\s+");
			if (bits.length == 0)
				continue;
			if (line.startsWith("Date:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split(":");
				if (d.length < 3)
					continue;
				year = Integer.parseInt(d[0]) + 2000;
				month = Integer.parseInt(d[1]);
				day = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Time:")) {
				if (bits.length < 2)
					continue;
				String[] d = bits[1].split("/");
				if (d.length < 3)
					continue;
				hour = Integer.parseInt(d[0]);
				minute = Integer.parseInt(d[1]);
				second = Integer.parseInt(d[2]);
				continue;
			}
			if (line.startsWith("Lat/Lon:")) {
				if (bits.length < 3)
					continue;
				lat = Integer.parseInt(bits[1]) * 1.0e-7;
				lon = Integer.parseInt(bits[2]) * 1.0e-7;
				continue;
			}
			if (line.startsWith("Alt:")) {
				if (bits.length < 2)
					continue;
				alt = Integer.parseInt(bits[1]);
				continue;
			}
			if (line.startsWith("Flags:")) {
				if (bits.length < 2)
					continue;
				int status = Integer.decode(bits[1]);
				connected = (status & AltosLib.AO_GPS_RUNNING) != 0;
				locked = (status & AltosLib.AO_GPS_VALID) != 0;
				if (!says_done)
					break;
				continue;
			}
			if (line.startsWith("Sats:")) {
				if (bits.length < 2)
					continue;
				nsat = Integer.parseInt(bits[1]);
				cc_gps_sat = new AltosGPSSat[nsat];
				for (int i = 0; i < nsat; i++) {
					int	svid = Integer.parseInt(bits[2+i*2]);
					int	cc_n0 = Integer.parseInt(bits[3+i*2]);
					cc_gps_sat[i] = new AltosGPSSat(svid, cc_n0);
				}
			}
			if (line.startsWith("done"))
				break;
			if (line.startsWith("Syntax error"))
				break;
		}
	}
}

public class AltosIdleMonitor extends Thread {
	AltosLink		link;
	AltosIdleMonitorListener	listener;
	AltosState		state;
	boolean			remote;
	double			frequency;
	AltosState		previous_state;
	AltosConfigData		config_data;
	AltosADC		adc;
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

	void update_state() throws InterruptedException, TimeoutException {
		AltosRecordTM	record = new AltosRecordTM();
		int		rssi;

		try {
			if (remote) {
				link.set_radio_frequency(frequency);
				link.start_remote();
			} else
				link.flush_input();
			config_data = new AltosConfigData(link);
			adc = new AltosADC(link);
			gps = new AltosGPSQuery(link, config_data);
		} finally {
			if (remote) {
				link.stop_remote();
				rssi = AltosRSSI();
			} else
				rssi = 0;
		}

		record.version = 0;
		record.callsign = config_data.callsign;
		record.serial = config_data.serial;
		record.flight = config_data.log_available() > 0 ? 255 : 0;
		record.rssi = rssi;
		record.status = 0;
		record.state = AltosLib.ao_flight_idle;

		record.tick = adc.tick;

		record.accel = adc.accel;
		record.pres = adc.pres;
		record.batt = adc.batt;
		record.temp = adc.temp;
		record.drogue = adc.drogue;
		record.main = adc.main;

		record.ground_accel = record.accel;
		record.ground_pres = record.pres;
		record.accel_plus_g = config_data.accel_cal_plus;
		record.accel_minus_g = config_data.accel_cal_minus;
		record.acceleration = 0;
		record.speed = 0;
		record.height = 0;
		record.gps = gps;
		state = new AltosState (record, state);
	}

	public void set_frequency(double in_frequency) {
		frequency = in_frequency;
	}

	public void post_state() {
		listener.update(state);
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
	}
}
