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

class AltosSensorMM {
	int		tick;
	int		sense[];
	int		v_batt;
	int		v_pyro;
	int		accel;
	int		accel_ref;

	public AltosSensorMM(AltosLink link) throws InterruptedException, TimeoutException {
		String[] items = link.adc();
		sense = new int[6];
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("0:")) {
				sense[0] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("1:")) {
				sense[1] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("2:")) {
				sense[2] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("3:")) {
				sense[3] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("4:")) {
				sense[4] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("5:")) {
				sense[5] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("6:")) {
				v_batt = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("7:")) {
				v_pyro = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("8:")) {
				accel = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("9:")) {
				accel_ref = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			i++;
		}
	}
}

