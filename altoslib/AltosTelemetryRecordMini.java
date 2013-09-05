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

package org.altusmetrum.altoslib_2;


public class AltosTelemetryRecordMini extends AltosTelemetryRecordRaw {
	int	state;

	int	accel;
	int	pres;
	int	temp;

	int	v_batt;
	int	sense_a;
	int	sense_m;

	int	acceleration;
	int	speed;
	int	height;

	int	ground_pres;

	public AltosTelemetryRecordMini(int[] in_bytes, int rssi) {
		super(in_bytes, rssi);

		state	      = int8(5);
		v_batt	      = int16(6);
		sense_a	      = int16(8);
		sense_m	      = int16(10);

		pres	      = int32(12);
		temp	      = int16(16);

		acceleration  = int16(18);
		speed        = int16(20);
		height        = int16(22);

		ground_pres   = int32(24);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	n = super.update_state(previous);

		AltosRecordMini	next;
		if (!(n instanceof AltosRecordMini)) {
			next = new AltosRecordMini(n);
		} else {
			next = (AltosRecordMini) n;
		}

		next.pres = pres;
		next.temp = temp;

		next.sense_a = sense_a;
		next.sense_m = sense_m;

		next.ground_pres = ground_pres;
		next.flight_accel = acceleration;
		next.flight_vel = speed;
		next.flight_height = height;
		next.flight_pres = pres;

		next.seen |= AltosRecord.seen_sensor;

		return next;
	}
}
