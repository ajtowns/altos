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

package altosui;

import org.altusmetrum.altosuilib_1.*;
import org.altusmetrum.altoslib_2.*;

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
	public static final int data_temperature = 12;
	public static final int data_range = 13;
	public static final int data_distance = 14;
	public static final int data_pressure = 15;

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
			if (state.gps != null && state.gps.cc_gps_sat != null)
				y = state.gps.cc_gps_sat.length;
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
		}
		if (y == AltosLib.MISSING)
			throw new AltosUIDataMissing(index);
		return y;
	}

	public int id(int index) {
		if (index == data_state) {
			int s = state.state;
			if (s < Altos.ao_flight_boost || s > Altos.ao_flight_landed)
				return -1;
			return s;
		}
		return 0;
	}

	public String id_name(int index) {
		if (index == data_state)
			return state.state_name();
		return "";
	}

	public AltosGraphDataPoint (AltosState state) {
		this.state = state;
	}
}