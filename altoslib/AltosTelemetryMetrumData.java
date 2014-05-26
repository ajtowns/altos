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

package org.altusmetrum.altoslib_4;


public class AltosTelemetryMetrumData extends AltosTelemetryStandard {

	int	ground_pres;
	int	ground_accel;
	int	accel_plus_g;
	int	accel_minus_g;

	public AltosTelemetryMetrumData(int[] bytes) {
		super(bytes);

		ground_pres = int32(8);
		ground_accel = int16(12);
		accel_plus_g = int16(14);
		accel_minus_g = int16(16);
	}

	public void update_state(AltosState state) {
		state.set_ground_accel(ground_accel);
		state.set_accel_g(accel_plus_g, accel_minus_g);
		state.set_ground_pressure(ground_pres);
	}
}
