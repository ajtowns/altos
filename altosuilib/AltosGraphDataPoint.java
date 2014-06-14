/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altosuilib_2;

import org.altusmetrum.altoslib_4.*;

public class AltosGraphDataPoint implements AltosUIDataPoint {

	AltosState	state;

	public static final int data_height = 0;
	public static final int data_speed = 1;
	public static final int data_accel = 2;
	public static final int data_temp = 3;
	public static final int data_battery_voltage = 4;
	public static final int data_drogue_voltage = 5;
	public static final int data_main_voltage = 6;
	public static final int data_rssi = 7;
	public static final int data_state = 8;
	public static final int data_gps_height = 9;
	public static final int data_gps_nsat_solution = 10;
	public static final int data_gps_nsat_view = 11;
	public static final int data_gps_altitude = 12;
	public static final int data_temperature = 13;
	public static final int data_range = 14;
	public static final int data_distance = 15;
	public static final int data_pressure = 16;
	public static final int data_accel_x = 17;
	public static final int data_accel_y = 18;
	public static final int data_accel_z = 19;
	public static final int data_gyro_x = 20;
	public static final int data_gyro_y = 21;
	public static final int data_gyro_z = 22;
	public static final int data_mag_x = 23;
	public static final int data_mag_y = 24;
	public static final int data_mag_z = 25;
	public static final int data_orient = 26;
	public static final int data_gps_course = 27;
	public static final int data_gps_ground_speed = 28;
	public static final int data_gps_climb_rate = 29;
	public static final int data_ignitor_0 = 30;
	public static final int data_ignitor_num = 32;
	public static final int data_ignitor_max = data_ignitor_0 + data_ignitor_num - 1;
	public static final int data_ignitor_fired_0 = data_ignitor_0 + data_ignitor_num;
	public static final int data_ignitor_fired_max = data_ignitor_fired_0 + data_ignitor_num - 1;

	public double x() throws AltosUIDataMissing {
		double	time = state.time_since_boost();
		if (time < -2)
			throw new AltosUIDataMissing(-1);
		return time;
	}

	public double y(int index) throws AltosUIDataMissing {
		double y = AltosLib.MISSING;
		switch (index) {
		case data_height:
			y = state.height();
			break;
		case data_speed:
			y = state.speed();
			break;
		case data_accel:
			y = state.acceleration();
			break;
		case data_temp:
			y = state.temperature;
			break;
		case data_battery_voltage:
			y = state.battery_voltage;
			break;
		case data_drogue_voltage:
			y = state.apogee_voltage;
			break;
		case data_main_voltage:
			y = state.main_voltage;
			break;
		case data_rssi:
			y = state.rssi;
			break;
		case data_gps_height:
			y = state.gps_height;
			break;
		case data_gps_nsat_solution:
			if (state.gps != null)
				y = state.gps.nsat;
			break;
		case data_gps_nsat_view:
			if (state.gps != null) {
				if (state.gps.cc_gps_sat != null)
					y = state.gps.cc_gps_sat.length;
				else
					y = 0;
			}
			break;
		case data_gps_altitude:
			y = state.gps_altitude();
			break;
		case data_temperature:
			y = state.temperature;
			break;
		case data_range:
			y = state.range;
			break;
		case data_distance:
			if (state.from_pad != null)
				y = state.from_pad.distance;
			break;
		case data_pressure:
			y = state.pressure();
			break;

		case data_accel_x:
		case data_accel_y:
		case data_accel_z:
		case data_gyro_x:
		case data_gyro_y:
		case data_gyro_z:
			AltosIMU	imu = state.imu;
			if (imu == null)
				break;
			switch (index) {
			case data_accel_x:
				y = imu.accel_x;
				break;
			case data_accel_y:
				y = imu.accel_y;
				break;
			case data_accel_z:
				y = imu.accel_z;
				break;
			case data_gyro_x:
				y = imu.gyro_x;
				break;
			case data_gyro_y:
				y = imu.gyro_y;
				break;
			case data_gyro_z:
				y = imu.gyro_z;
				break;
			}
			break;
		case data_mag_x:
		case data_mag_y:
		case data_mag_z:
			AltosMag	mag = state.mag;
			if (mag == null)
				break;
			switch (index) {
			case data_mag_x:
				y = mag.x;
				break;
			case data_mag_y:
				y = mag.y;
				break;
			case data_mag_z:
				y = mag.z;
				break;
			}
			break;
		case data_orient:
			y = state.orient();
			break;
		case data_gps_course:
			if (state.gps != null)
				y = state.gps.course;
			else
				y = AltosLib.MISSING;
			break;
		case data_gps_ground_speed:
			if (state.gps != null)
				y = state.gps.ground_speed;
			else
				y = AltosLib.MISSING;
			break;
		case data_gps_climb_rate:
			if (state.gps != null)
				y = state.gps.climb_rate;
			else
				y = AltosLib.MISSING;
			break;
		default:
			if (data_ignitor_0 <= index && index <= data_ignitor_max) {
				int ignitor = index - data_ignitor_0;
				if (state.ignitor_voltage != null && ignitor < state.ignitor_voltage.length)
					y = state.ignitor_voltage[ignitor];
			} else if (data_ignitor_fired_0 <= index && index <= data_ignitor_fired_max) {
				int ignitor = index - data_ignitor_fired_0;
				if (state.ignitor_voltage != null && ignitor < state.ignitor_voltage.length) {
					if ((state.pyro_fired & (1 << ignitor)) != 0)
						y = 1;
					else
						y = 0;
				}
			}
			break;
		}
		if (y == AltosLib.MISSING)
			throw new AltosUIDataMissing(index);
		return y;
	}

	public int id(int index) {
		if (index == data_state) {
			int s = state.state;
			if (AltosLib.ao_flight_boost <= s && s <= AltosLib.ao_flight_landed)
				return s;
		} else if (data_ignitor_fired_0 <= index && index <= data_ignitor_fired_max) {
			int ignitor = index - data_ignitor_fired_0;
			if (state.ignitor_voltage != null && ignitor < state.ignitor_voltage.length) {
				if (state.ignitor_voltage != null && ignitor < state.ignitor_voltage.length) {
					if ((state.pyro_fired & (1 << ignitor)) != 0)
						return 1;
				}
			}
		}
		return -1;
	}

	public String id_name(int index) {
		if (index == data_state) {
			return state.state_name();
		} else if (data_ignitor_fired_0 <= index && index <= data_ignitor_fired_max) {
			int ignitor = index - data_ignitor_fired_0;
			if (state.ignitor_voltage != null && ignitor < state.ignitor_voltage.length)
				return AltosLib.ignitor_name(ignitor);
		}
		return "";
	}

	public AltosGraphDataPoint (AltosState state) {
		this.state = state;
	}
}
