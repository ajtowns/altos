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

public class AltosTelemetryMegaData extends AltosTelemetryStandard {
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

	public AltosTelemetryMegaData(int[] bytes) {
		super(bytes);

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

	public void update_state(AltosState state) {
		super.update_state(state);

		state.set_state(this.state);

		state.set_battery_voltage(AltosConvert.mega_battery_voltage(v_batt));
		state.set_pyro_voltage(AltosConvert.mega_pyro_voltage(v_pyro));

		state.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sense[4]));
		state.set_main_voltage(AltosConvert.mega_pyro_voltage(sense[5]));

		double voltages[] = new double[4];
		for (int i = 0; i < 4; i++)
			voltages[i] = AltosConvert.mega_pyro_voltage(sense[i]);

		state.set_ignitor_voltage(voltages);

		state.set_ground_accel(ground_accel);
		state.set_ground_pressure(ground_pres);
		state.set_accel_g(accel_plus_g, accel_minus_g);

		state.set_kalman(height, speed/16.0, acceleration / 16.0);
	}
}

