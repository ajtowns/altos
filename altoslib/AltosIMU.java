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

public class AltosIMU implements Cloneable {
	public double		accel_x;
	public double		accel_y;
	public double		accel_z;

	public double		gyro_x;
	public double		gyro_y;
	public double		gyro_z;

/*
 * XXX use ground measurements to adjust values

	public double		ground_accel_x;
	public double		ground_accel_y;
	public double		ground_accel_z;

	public double		ground_gyro_x;
	public double		ground_gyro_y;
	public double		ground_gyro_z;
*/

	public static int	counts_per_g = 2048;

	public static double convert_accel(int counts) {
		return (double) counts / (double) counts_per_g * (-AltosConvert.GRAVITATIONAL_ACCELERATION);
	}

	public static double	counts_per_degsec = 16.4;

	public static double convert_gyro(int counts) {
		return (double) counts / counts_per_degsec;
	}

	public boolean parse_string(String line) {
		if (!line.startsWith("Accel:"))
			return false;

		String[] items = line.split("\\s+");

		if (items.length >= 8) {
			accel_x = convert_accel(Integer.parseInt(items[1]));
			accel_y = convert_accel(Integer.parseInt(items[2]));
			accel_z = convert_accel(Integer.parseInt(items[3]));
			gyro_x = convert_gyro(Integer.parseInt(items[5]));
			gyro_y = convert_gyro(Integer.parseInt(items[6]));
			gyro_z = convert_gyro(Integer.parseInt(items[7]));
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
