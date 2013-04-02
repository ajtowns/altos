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


public class AltosTelemetryRecordSensor extends AltosTelemetryRecordRaw {
	int	state;
	int	accel;
	int	pres;
	int	temp;
	int	v_batt;
	int	sense_d;
	int	sense_m;

	int	acceleration;
	int	speed;
	int	height;

	int	ground_accel;
	int	ground_pres;
	int	accel_plus_g;
	int	accel_minus_g;

	public AltosTelemetryRecordSensor(int[] in_bytes, int rssi) {
		super(in_bytes, rssi);
		state         = uint8(5);

		accel         = int16(6);
		pres          = int16(8);
		temp          = int16(10);
		v_batt        = int16(12);
		sense_d       = int16(14);
		sense_m       = int16(16);

		acceleration  = int16(18);
		speed         = int16(20);
		height        = int16(22);

		ground_pres   = int16(24);
		ground_accel  = int16(26);
		accel_plus_g  = int16(28);
		accel_minus_g = int16(30);
	}

	public AltosRecord update_state(AltosRecord prev) {
		AltosRecord	n = super.update_state(prev);

		AltosRecordTM	next;
		if (!(n instanceof AltosRecordTM))
			next = new AltosRecordTM(n);
		else
			next = (AltosRecordTM) n;

		next.state = state;
		if (type == packet_type_TM_sensor)
			next.accel = accel;
		else
			next.accel = AltosRecord.MISSING;
		next.pres = pres;
		next.temp = temp;
		next.batt = v_batt;
		if (type == packet_type_TM_sensor || type == packet_type_Tm_sensor) {
			next.drogue = sense_d;
			next.main = sense_m;
		} else {
			next.drogue = AltosRecord.MISSING;
			next.main = AltosRecord.MISSING;
		}

		next.kalman_acceleration = acceleration / 16.0;
		next.kalman_speed = speed / 16.0;
		next.kalman_height = height;

		next.ground_pres = ground_pres;
		if (type == packet_type_TM_sensor) {
			next.ground_accel = ground_accel;
			next.accel_plus_g = accel_plus_g;
			next.accel_minus_g = accel_minus_g;
		} else {
			next.ground_accel = AltosRecord.MISSING;
			next.accel_plus_g = AltosRecord.MISSING;
			next.accel_minus_g = AltosRecord.MISSING;
		}

		next.seen |= AltosRecord.seen_sensor | AltosRecord.seen_temp_volt;

		return next;
	}
}
