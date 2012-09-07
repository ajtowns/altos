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

import java.util.concurrent.TimeoutException;

class AltosSensorTM extends AltosRecordTM {

	public AltosSensorTM(AltosLink link, AltosConfigData config_data) throws InterruptedException, TimeoutException {
		super();
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
		ground_accel = config_data.accel_cal_plus;
		ground_pres = pres;
		accel_plus_g = config_data.accel_cal_plus;
		accel_minus_g = config_data.accel_cal_minus;
	}
}

