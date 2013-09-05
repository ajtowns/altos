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


public class AltosTelemetryRecordMetrumData extends AltosTelemetryRecordRaw {

	int	ground_pres;
	int	ground_accel;
	int	accel_plus_g;
	int	accel_minus_g;

	public AltosTelemetryRecordMetrumData(int[] in_bytes, int rssi) {
		super(in_bytes, rssi);

		ground_pres = int32(8);
		ground_accel = int16(12);
		accel_plus_g = int16(14);
		accel_minus_g = int16(16);
	}

	public AltosRecord update_state(AltosRecord previous) {
		AltosRecord	n = super.update_state(previous);

		AltosRecordTM2	next;
		if (!(n instanceof AltosRecordTM2)) {
			next = new AltosRecordTM2(n);
		} else {
			next = (AltosRecordTM2) n;
		}

		next.ground_accel = ground_accel;
		next.ground_pres = ground_pres;
		next.accel_plus_g = accel_plus_g;
		next.accel_minus_g = accel_minus_g;

		return next;
	}
}
