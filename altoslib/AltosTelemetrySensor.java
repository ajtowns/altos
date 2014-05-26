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


public class AltosTelemetrySensor extends AltosTelemetryStandard {
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

	public AltosTelemetrySensor(int[] bytes) {
		super(bytes);
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

	public void update_state(AltosState state) {
		super.update_state(state);

		state.set_state(this.state);
		if (type == packet_type_TM_sensor) {
			state.set_ground_accel(ground_accel);
			state.set_accel_g(accel_plus_g, accel_minus_g);
			state.set_accel(accel);
		}
		state.set_ground_pressure(AltosConvert.barometer_to_pressure(ground_pres));
		state.set_pressure(AltosConvert.barometer_to_pressure(pres));
		state.set_temperature(AltosConvert.thermometer_to_temperature(temp));
		state.set_battery_voltage(AltosConvert.cc_battery_to_voltage(v_batt));
		if (type == packet_type_TM_sensor || type == packet_type_Tm_sensor) {
			state.set_apogee_voltage(AltosConvert.cc_ignitor_to_voltage(sense_d));
			state.set_main_voltage(AltosConvert.cc_ignitor_to_voltage(sense_m));
		}

		state.set_kalman(height, speed/16.0, acceleration / 16.0);
	}
}
