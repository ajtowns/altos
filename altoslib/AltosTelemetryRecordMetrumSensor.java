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

package org.altusmetrum.altoslib_1;


public class AltosTelemetryRecordMetrumSensor extends AltosTelemetryRecordRaw {
	int	state;

	int	accel;
	int	pres;
	int	temp;

	int	acceleration;
	int	speed;
	int	height;

	int	v_batt;
	int	sense_a;
	int	sense_m;

	public AltosTelemetryRecordMetrumSensor(int[] in_bytes, int rssi) {
		super(in_bytes, rssi);

		state	      = int8(5);
		accel         = int16(6);
		pres          = int32(8);
		temp          = int16(12);

		acceleration  = int16(14);
		speed         = int16(16);
		height        = int16(18);

		v_batt        = int16(20);
		sense_a       = int16(22);
		sense_m       = int16(24);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	n = super.update_state(previous);

		AltosRecordTM2	next;
		if (!(n instanceof AltosRecordTM2)) {
			next = new AltosRecordTM2(n);
		} else {
			next = (AltosRecordTM2) n;
		}

		next.state = state;

		next.accel = accel;
		next.pres = pres;
		next.temp = temp;

		next.kalman_acceleration = acceleration / 16.0;
		next.kalman_speed = speed / 16.0;
		next.kalman_height = height;

		next.v_batt = v_batt;
		next.sense_a = sense_a;
		next.sense_m = sense_m;

		next.seen |= AltosRecord.seen_sensor | AltosRecord.seen_temp_volt;;

		return next;
	}
}
