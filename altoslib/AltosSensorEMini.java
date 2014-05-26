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

public class AltosSensorEMini {
	public int	tick;
	public int	apogee;
	public int	main;
	public int	batt;

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosSensorEMini	sensor_emini = new AltosSensorEMini(link);

			if (sensor_emini == null)
				return;
			state.set_battery_voltage(AltosConvert.easy_mini_voltage(sensor_emini.batt, config_data.serial));
			state.set_apogee_voltage(AltosConvert.easy_mini_voltage(sensor_emini.apogee, config_data.serial));
			state.set_main_voltage(AltosConvert.easy_mini_voltage(sensor_emini.main, config_data.serial));

		} catch (TimeoutException te) {
		}
	}

	public AltosSensorEMini(AltosLink link) throws InterruptedException, TimeoutException {
		String[] items = link.adc();
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("apogee:")) {
				apogee = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("main:")) {
				main = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("batt:")) {
				batt = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			i++;
		}
	}
}

