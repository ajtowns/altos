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

package org.altusmetrum.altoslib_2;

import java.util.concurrent.*;

public class AltosIMU implements Cloneable {
	public int		accel_x;
	public int		accel_y;
	public int		accel_z;

	public int		gyro_x;
	public int		gyro_y;
	public int		gyro_z;

	public boolean parse_string(String line) {
		if (!line.startsWith("Accel:"))
			return false;

		String[] items = line.split("\\s+");

		if (items.length >= 8) {
			accel_x = Integer.parseInt(items[1]);
			accel_y = Integer.parseInt(items[2]);
			accel_z = Integer.parseInt(items[3]);
			gyro_x = Integer.parseInt(items[5]);
			gyro_y = Integer.parseInt(items[6]);
			gyro_z = Integer.parseInt(items[7]);
		}
		return true;
	}

	public AltosIMU clone() {
		AltosIMU	n = new AltosIMU();

		n.accel_x = accel_x;
		n.accel_y = accel_y;
		n.accel_z = accel_z;

		n.gyro_x = gyro_x;
		n.gyro_y = gyro_y;
		n.gyro_z = gyro_z;
		return n;
	}

	static public void update_state(AltosState state, AltosLink link, AltosConfigData config_data) throws InterruptedException {
		try {
			AltosIMU	imu = new AltosIMU(link);

			if (imu != null)
				state.set_imu(imu);
		} catch (TimeoutException te) {
		}
	}

	public AltosIMU() {
		accel_x = AltosLib.MISSING;
		accel_y = AltosLib.MISSING;
		accel_z = AltosLib.MISSING;

		gyro_x = AltosLib.MISSING;
		gyro_y = AltosLib.MISSING;
		gyro_z = AltosLib.MISSING;
	}

	public AltosIMU(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		link.printf("I\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (parse_string(line))
				break;
		}
	}
}
