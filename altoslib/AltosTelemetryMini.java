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


public class AltosTelemetryMini extends AltosTelemetryStandard {
	int	state;

	int	v_batt;
	int	sense_a;
	int	sense_m;

	int	pres;
	int	temp;

	int	acceleration;
	int	speed;
	int	height;

	int	ground_pres;

	public AltosTelemetryMini(int[] bytes) {
		super(bytes);

		state	      = int8(5);

		v_batt        = int16(6);
		sense_a       = int16(8);
		sense_m       = int16(10);

		pres          = int32(12);
		temp          = int16(16);

		acceleration  = int16(18);
		speed         = int16(20);
		height        = int16(22);

		ground_pres   = int32(24);
	}

	public void update_state(AltosState state) {
		super.update_state(state);

		state.set_state(this.state);

		state.set_battery_voltage(AltosConvert.tele_mini_voltage(v_batt));
		state.set_apogee_voltage(AltosConvert.tele_mini_voltage(sense_a));
		state.set_main_voltage(AltosConvert.tele_mini_voltage(sense_m));

		state.set_ground_pressure(ground_pres);

		state.set_pressure(pres);
		state.set_temperature(temp/100.0);

		state.set_kalman(height, speed/16.0, acceleration/16.0);
	}
}
