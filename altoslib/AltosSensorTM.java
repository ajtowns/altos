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

import java.util.concurrent.TimeoutException;

public class AltosSensorTM {
	public int	tick;
	public int	accel;
	public int	pres;
	public int	temp;
	public int	batt;
	public int	drogue;
	public int	main;

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosSensorTM	sensor_tm = new AltosSensorTM(link);

			if (sensor_tm == null)
				return;
			state.set_accel(sensor_tm.accel);
			state.set_pressure(AltosConvert.barometer_to_pressure(sensor_tm.pres));
			state.set_temperature(AltosConvert.thermometer_to_temperature(sensor_tm.temp));
			state.set_battery_voltage(AltosConvert.cc_battery_to_voltage(sensor_tm.batt));
			state.set_apogee_voltage(AltosConvert.cc_ignitor_to_voltage(sensor_tm.drogue));
			state.set_main_voltage(AltosConvert.cc_ignitor_to_voltage(sensor_tm.main));

		} catch (TimeoutException te) {
		}
	}

	public AltosSensorTM(AltosLink link) throws InterruptedException, TimeoutException {
		String[] items = link.adc();
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
	}
}

