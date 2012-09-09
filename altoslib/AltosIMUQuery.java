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

class AltosIMUQuery extends AltosIMU {

	public AltosIMUQuery (AltosLink link) throws InterruptedException, TimeoutException {
		link.printf("I\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (!line.startsWith("Accel:"))
				continue;
			String[] items = line.split("\\s+");
			if (items.length >= 8) {
				accel_x = Integer.parseInt(items[1]);
				accel_y = Integer.parseInt(items[2]);
				accel_z = Integer.parseInt(items[3]);
				gyro_x = Integer.parseInt(items[5]);
				gyro_y = Integer.parseInt(items[6]);
				gyro_z = Integer.parseInt(items[7]);
			}
			break;
		}
	}
}

