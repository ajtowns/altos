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

package altosui;

import java.io.*;
import org.altusmetrum.altoslib_1.*;

public class AltosFlightStats {
	double		max_height;
	double		max_speed;
	double		max_acceleration;
	double[]	state_accel_speed = new double[Altos.ao_flight_invalid + 1];
	double[]	state_baro_speed = new double[Altos.ao_flight_invalid + 1];
	double[]	state_accel = new double[Altos.ao_flight_invalid + 1];
	int[]		state_count = new int[Altos.ao_flight_invalid + 1];
	double[]	state_start = new double[Altos.ao_flight_invalid + 1];
	double[]	state_end = new double[Altos.ao_flight_invalid + 1];
	int		serial;
	int		flight;
	int		year, month, day;
	int		hour, minute, second;
	double		lat, lon;
	double		pad_lat, pad_lon;
	boolean		has_gps;
	boolean		has_other_adc;
	boolean		has_rssi;

	double landed_time(AltosRecordIterable iterable) {
		AltosState	state = null;
		for (AltosRecord record : iterable) {
			state = new AltosState(record, state);

			if (state.state == Altos.ao_flight_landed)
				break;
		}

		double	landed_height = state.height;

		state = null;

		boolean	above = true;

		double	landed_time = -1000;

		for (AltosRecord record : iterable) {
			state = new AltosState(record, state);

			if (state.height > landed_height + 10) {
				above = true;
			} else {
				if (above && state.height < landed_height + 2) {
					above = false;
					landed_time = state.time;
				}
			}
		}
		if (landed_time == -1000)
			landed_time = state.time;
		return landed_time;
	}

	double boost_time(AltosRecordIterable iterable) {
		double boost_time = -1000;

		AltosState state = null;

		for (AltosRecord record : iterable) {
			state = new AltosState(record, state);
			
			if (state.acceleration < 1)
				boost_time = state.time;
			if (state.state >= Altos.ao_flight_boost)
				break;
		}
		if (boost_time == -1000)
			boost_time = state.time;
		return boost_time;
	}


	public AltosFlightStats(AltosRecordIterable iterable) throws InterruptedException, IOException {
		AltosState	state = null;
		AltosState	new_state = null;
		double		boost_time = boost_time(iterable);
		double		end_time = 0;
		double		landed_time = landed_time(iterable);

		year = month = day = -1;
		hour = minute = second = -1;
		serial = flight = -1;
		lat = lon = -1;
		has_gps = false;
		has_other_adc = false;
		has_rssi = false;
		for (AltosRecord record : iterable) {
			if (serial < 0)
				serial = record.serial;
			if ((record.seen & AltosRecord.seen_flight) != 0 && flight < 0)
				flight = record.flight;
			if ((record.seen & AltosRecord.seen_temp_volt) != 0)
				has_other_adc = true;
			if (record.rssi != 0)
				has_rssi = true;
			new_state = new AltosState(record, state);
			end_time = new_state.time;
			state = new_state;
			if (state.time >= boost_time && state.state < Altos.ao_flight_boost)
				state.state = Altos.ao_flight_boost;
			if (state.time >= landed_time && state.state < Altos.ao_flight_landed)
				state.state = Altos.ao_flight_landed;
			if (0 <= state.state && state.state < Altos.ao_flight_invalid) {
				if (state.state >= Altos.ao_flight_boost) {
					if (state.gps != null && state.gps.locked &&
					    year < 0) {
						year = state.gps.year;
						month = state.gps.month;
						day = state.gps.day;
						hour = state.gps.hour;
						minute = state.gps.minute;
						second = state.gps.second;
					}
				}
				state_accel[state.state] += state.acceleration;
				state_accel_speed[state.state] += state.accel_speed;
				state_baro_speed[state.state] += state.baro_speed;
				state_count[state.state]++;
				if (state_start[state.state] == 0.0)
					state_start[state.state] = state.time;
				if (state_end[state.state] < state.time)
					state_end[state.state] = state.time;
				max_height = state.max_height;
				if (state.max_accel_speed != 0)
					max_speed = state.max_accel_speed;
				else
					max_speed = state.max_baro_speed;
				max_acceleration = state.max_acceleration;
			}
			if (state.gps != null && state.gps.locked && state.gps.nsat >= 4) {
				if (state.state <= Altos.ao_flight_pad) {
					pad_lat = state.gps.lat;
					pad_lon = state.gps.lon;
				}
				lat = state.gps.lat;
				lon = state.gps.lon;
				has_gps = true;
			}
		}
		for (int s = Altos.ao_flight_startup; s <= Altos.ao_flight_landed; s++) {
			if (state_count[s] > 0) {
				state_accel_speed[s] /= state_count[s];
				state_baro_speed[s] /= state_count[s];
				state_accel[s] /= state_count[s];
			}
			if (state_start[s] == 0)
				state_start[s] = end_time;
			if (state_end[s] == 0)
				state_end[s] = end_time;
		}
	}
}
