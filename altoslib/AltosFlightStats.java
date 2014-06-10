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

import java.io.*;

public class AltosFlightStats {
	public double		max_height;
	public double		max_gps_height;
	public double		max_speed;
	public double		max_acceleration;
	public double[]	state_speed = new double[AltosLib.ao_flight_invalid + 1];
	public double[]	state_accel = new double[AltosLib.ao_flight_invalid + 1];
	public int[]		state_count = new int[AltosLib.ao_flight_invalid + 1];
	public double[]	state_start = new double[AltosLib.ao_flight_invalid + 1];
	public double[]	state_end = new double[AltosLib.ao_flight_invalid + 1];
	public int		serial;
	public int		flight;
	public int		year, month, day;
	public int		hour, minute, second;
	public double		lat, lon;
	public double		pad_lat, pad_lon;
	public boolean		has_flight_data;
	public boolean		has_gps;
	public boolean		has_flight_adc;
	public boolean		has_battery;
	public boolean		has_rssi;
	public boolean		has_imu;
	public boolean		has_mag;
	public boolean		has_orient;
	public int		num_ignitor;

	double landed_time(AltosStateIterable states) {
		AltosState state = null;

		for (AltosState s : states) {
			state = s;
			if (state.state == AltosLib.ao_flight_landed)
				break;
		}

		if (state == null)
			return 0;

		double	landed_height = state.height();

		state = null;

		boolean	above = true;

		double	landed_time = -1000;

		for (AltosState s : states) {
			state = s;

			if (state.height() > landed_height + 10) {
				above = true;
			} else {
				if (above && state.height() < landed_height + 2) {
					above = false;
					landed_time = state.time;
				}
			}
		}
		if (landed_time == -1000)
			landed_time = state.time;
		return landed_time;
	}

	double boost_time(AltosStateIterable states) {
		double boost_time = AltosLib.MISSING;
		AltosState	state = null;

		for (AltosState s : states) {
			state = s;
			if (state.acceleration() < 1)
				boost_time = state.time;
			if (state.state >= AltosLib.ao_flight_boost && state.state <= AltosLib.ao_flight_landed)
				break;
		}
		if (state == null)
			return 0;

		if (boost_time == AltosLib.MISSING)
			boost_time = state.time;
		return boost_time;
	}


	public AltosFlightStats(AltosStateIterable states) throws InterruptedException, IOException {
		double		boost_time = boost_time(states);
		double		end_time = 0;
		double		landed_time = landed_time(states);

		year = month = day = AltosLib.MISSING;
		hour = minute = second = AltosLib.MISSING;
		serial = flight = AltosLib.MISSING;
		lat = lon = AltosLib.MISSING;
		has_flight_data = false;
		has_gps = false;
		has_flight_adc = false;
		has_battery = false;
		has_rssi = false;
		has_imu = false;
		has_mag = false;
		has_orient = false;
		for (AltosState state : states) {
			if (serial == AltosLib.MISSING && state.serial != AltosLib.MISSING)
				serial = state.serial;
			if (flight == AltosLib.MISSING && state.flight != AltosLib.MISSING)
				flight = state.flight;
			if (state.battery_voltage != AltosLib.MISSING)
				has_battery = true;
			if (state.main_voltage != AltosLib.MISSING)
				has_flight_adc = true;
			if (state.rssi != AltosLib.MISSING)
				has_rssi = true;
			end_time = state.time;

			if (state.pressure() != AltosLib.MISSING)
				has_flight_data = true;

			int state_id = state.state;
			if (state.time >= boost_time && state_id < AltosLib.ao_flight_boost)
				state_id = AltosLib.ao_flight_boost;
			if (state.time >= landed_time && state_id < AltosLib.ao_flight_landed)
				state_id = AltosLib.ao_flight_landed;
			if (state.gps != null && state.gps.locked) {
				year = state.gps.year;
				month = state.gps.month;
				day = state.gps.day;
				hour = state.gps.hour;
				minute = state.gps.minute;
				second = state.gps.second;
			}
			max_height = state.max_height();
			max_speed = state.max_speed();
			max_acceleration = state.max_acceleration();
			max_gps_height = state.max_gps_height();

			if (0 <= state_id && state_id < AltosLib.ao_flight_invalid) {
				double acceleration = state.acceleration();
				double speed = state.speed();
				if (acceleration != AltosLib.MISSING && speed != AltosLib.MISSING) {
					state_accel[state_id] += acceleration;
					state_speed[state_id] += speed;
					state_count[state_id]++;
				}
				if (state_start[state_id] == 0.0)
					state_start[state_id] = state.time;
				if (state_end[state_id] < state.time)
					state_end[state_id] = state.time;
			}
			if (state.pad_lat != AltosLib.MISSING) {
				pad_lat = state.pad_lat;
				pad_lon = state.pad_lon;
			}
			if (state.gps != null && state.gps.locked && state.gps.nsat >= 4) {
				lat = state.gps.lat;
				lon = state.gps.lon;
				has_gps = true;
			}
			if (state.imu != null)
				has_imu = true;
			if (state.mag != null)
				has_mag = true;
			if (state.orient() != AltosLib.MISSING)
				has_orient = true;
			if (state.ignitor_voltage != null && state.ignitor_voltage.length > num_ignitor)
				num_ignitor = state.ignitor_voltage.length;
		}
		for (int s = AltosLib.ao_flight_startup; s <= AltosLib.ao_flight_landed; s++) {
			if (state_count[s] > 0) {
				state_speed[s] /= state_count[s];
				state_accel[s] /= state_count[s];
			} else {
				state_speed[s] = AltosLib.MISSING;
				state_accel[s] = AltosLib.MISSING;
			}
			if (state_start[s] == 0)
				state_start[s] = end_time;
			if (state_end[s] == 0)
				state_end[s] = end_time;
		}
	}
}
