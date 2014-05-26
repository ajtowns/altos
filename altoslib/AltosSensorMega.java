/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

import java.util.concurrent.TimeoutException;

class AltosSensorMega {
	int		tick;
	int[]		sense;
	int		v_batt;
	int		v_pbatt;
	int		temp;

	public AltosSensorMega() {
		sense = new int[6];
	}

	public AltosSensorMega(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		String[] items = link.adc();
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("A:")) {
				sense[0] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("B:")) {
				sense[1] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("C:")) {
				sense[2] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("D:")) {
				sense[3] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("drogue:")) {
				sense[4] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("main:")) {
				sense[5] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("batt:")) {
				v_batt = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("pbatt:")) {
				v_pbatt = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("temp:")) {
				temp = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			i++;
		}
	}

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosSensorMega	sensor_mega = new AltosSensorMega(link);

			state.set_battery_voltage(AltosConvert.mega_battery_voltage(sensor_mega.v_batt));
			state.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sensor_mega.sense[4]));
			state.set_main_voltage(AltosConvert.mega_pyro_voltage(sensor_mega.sense[5]));

			double[]	ignitor_voltage = new double[4];
			for (int i = 0; i < 4; i++)
				ignitor_voltage[i] = AltosConvert.mega_pyro_voltage(sensor_mega.sense[i]);
			state.set_ignitor_voltage(ignitor_voltage);

		} catch (TimeoutException te) {
		}
	}
}

