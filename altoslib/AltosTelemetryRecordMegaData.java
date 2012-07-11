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


public class AltosTelemetryRecordMegaData extends AltosTelemetryRecordRaw {

	int	state;
	
	int	v_batt;
	int	v_pyro;
	int	sense[];

	int	ground_pres;
	int	ground_accel;
	int	accel_plus_g;
	int	accel_minus_g;

	int	acceleration;
	int	speed;
	int	height;

	public AltosTelemetryRecordMegaData(int[] in_bytes) {
		super(in_bytes);

		state = int8(5);

		v_batt = int16(6);
		v_pyro = int16(8);

		sense = new int[6];	

		for (int i = 0; i < 6; i++) {
			sense[i] = int8(10 + i) << 4;
			sense[i] |= sense[i] >> 8;
		}

		ground_pres = int32(16);
		ground_accel = int16(20);
		accel_plus_g = int16(22);
		accel_minus_g = int16(24);

		acceleration = int16(26);
		speed = int16(28);
		height = int16(30);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	n = super.update_state(previous);

		AltosRecordMM	next;
		if (!(n instanceof AltosRecordMM)) {
			next = new AltosRecordMM(n);
		} else {
			next = (AltosRecordMM) n;
		}

		next.state = state;

		next.v_batt = v_batt;
		next.v_pyro = v_pyro;

		for (int i = 0; i < 6; i++)
			next.sense[i] = sense[i];

		next.ground_accel = ground_accel;
		next.ground_pres = ground_pres;
		next.accel_plus_g = accel_plus_g;
		next.accel_minus_g = accel_minus_g;

		next.acceleration = acceleration / 16.0;
		next.speed = speed / 16.0;
		next.height = height;

		next.seen |= AltosRecord.seen_flight | AltosRecord.seen_temp_volt;

		return next;
	}
}
