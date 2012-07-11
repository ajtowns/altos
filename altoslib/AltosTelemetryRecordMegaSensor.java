/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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


public class AltosTelemetryRecordMegaSensor extends AltosTelemetryRecordRaw {
	int	accel;
	int	pres;
	int	temp;

	int	accel_x;
	int	accel_y;
	int	accel_z;

	int	gyro_x;
	int	gyro_y;
	int	gyro_z;

	int	mag_x;
	int	mag_y;
	int	mag_z;

	int	rssi;

	public AltosTelemetryRecordMegaSensor(int[] in_bytes, int in_rssi) {
		super(in_bytes);

		accel         = int16(6);
		pres          = int32(8);
		temp          = int16(12);

		accel_x	      = int16(14);
		accel_y	      = int16(16);
		accel_z	      = int16(18);

		gyro_x	      = int16(20);
		gyro_y	      = int16(22);
		gyro_z	      = int16(24);

		mag_x	      = int16(26);
		mag_y	      = int16(28);
		mag_z	      = int16(30);

		rssi	      = in_rssi;
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	n = super.update_state(previous);

		AltosRecordMM	next;
		if (!(n instanceof AltosRecordMM)) {
			next = new AltosRecordMM(n);
		} else {
			next = (AltosRecordMM) n;
		}

		next.accel = accel;
		next.pres = pres;
		next.temp = temp;

		next.imu.accel_x = accel_x;
		next.imu.accel_y = accel_y;
		next.imu.accel_z = accel_z;

		next.imu.gyro_x = gyro_x;
		next.imu.gyro_y = gyro_y;
		next.imu.gyro_z = gyro_z;

		next.mag.x = mag_x;
		next.mag.y = mag_y;
		next.mag.z = mag_z;

		next.rssi = rssi;

		next.seen |= AltosRecord.seen_sensor;

		return next;
	}
}
