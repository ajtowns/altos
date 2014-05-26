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

package org.altusmetrum.altoslib_4;

import java.util.concurrent.*;

public class AltosMs5607 {
	public int	reserved;
	public int	sens;
	public int	off;
	public int	tcs;
	public int	tco;
	public int	tref;
	public int	tempsens;
	public int	crc;

	public int	raw_pres;
	public int	raw_temp;
	public int	pa;
	public int	cc;

	static final boolean	ms5611 = false;

	void convert() {
		int	dT;
		int TEMP;
		long OFF;
		long SENS;
		//int P;

		dT = raw_temp - ((int) tref << 8);

		TEMP = (int) (2000 + (((long) dT * (long) tempsens) >> 23));

		if (ms5611) {
			OFF = ((long) off << 16) + (((long) tco * (long) dT) >> 7);

			SENS = ((long) sens << 15) + (((long) tcs * (long) dT) >> 8);
		} else {
			OFF = ((long) off << 17) + (((long) tco * (long) dT) >> 6);

			SENS = ((long) sens << 16) + (((long) tcs * (long) dT) >> 7);
		}

		if (TEMP < 2000) {
			int	T2 = (int) (((long) dT * (long) dT) >> 31);
			int TEMPM = TEMP - 2000;
			long OFF2 = ((long) 61 * (long) TEMPM * (long) TEMPM) >> 4;
			long SENS2 = (long) 2 * (long) TEMPM * (long) TEMPM;
			if (TEMP < 1500) {
				int TEMPP = TEMP + 1500;
				long TEMPP2 = (long) TEMPP * (long) TEMPP;
				OFF2 = OFF2 + 15 * TEMPP2;
				SENS2 = SENS2 + 8 * TEMPP2;
			}
			TEMP -= T2;
			OFF -= OFF2;
			SENS -= SENS2;
		}

		pa = (int) (((((long) raw_pres * SENS) >> 21) - OFF) >> 15);
		cc = TEMP;
	}

	public int set(int in_pres, int in_temp) {
		raw_pres = in_pres;
		raw_temp = in_temp;
		convert();
		return pa;
	}

	public boolean parse_line(String line) {
		String[] items = line.split("\\s+");
		if (line.startsWith("Pressure:")) {
			if (items.length >= 2) {
				raw_pres = Integer.parseInt(items[1]);
			}
		} else if (line.startsWith("Temperature:")) {
			if (items.length >= 2)
				raw_temp = Integer.parseInt(items[1]);
		} else if (line.startsWith("ms5607 reserved:")) {
			if (items.length >= 3)
				reserved = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 sens:")) {
			if (items.length >= 3) {
				sens = Integer.parseInt(items[2]);
			}
		} else if (line.startsWith("ms5607 off:")) {
			if (items.length >= 3)
				off = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 tcs:")) {
			if (items.length >= 3)
				tcs = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 tco:")) {
			if (items.length >= 3)
				tco = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 tref:")) {
			if (items.length >= 3)
				tref = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 tempsens:")) {
			if (items.length >= 3)
				tempsens = Integer.parseInt(items[2]);
		} else if (line.startsWith("ms5607 crc:")) {
			if (items.length >= 3)
				crc = Integer.parseInt(items[2]);
		} else if (line.startsWith("Altitude:")) {
			return false;
		}
		return true;
	}

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosMs5607	ms5607 = new AltosMs5607(link);

			if (ms5607 != null) {
				state.set_ms5607(ms5607);
				return;
			}
		} catch (TimeoutException te) {
		}
	}

	public AltosMs5607() {
		raw_pres = AltosLib.MISSING;
		raw_temp = AltosLib.MISSING;
		pa = AltosLib.MISSING;
		cc = AltosLib.MISSING;
	}

	public AltosMs5607 (AltosLink link) throws InterruptedException, TimeoutException {
		this();
		link.printf("c s\nB\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (!parse_line(line)) {
				break;
			}
		}
		convert();
	}
}
