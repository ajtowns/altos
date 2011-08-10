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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.util.prefs.*;
import java.util.concurrent.*;

public class AltosFlightStats {
	double		max_height;
	double		max_speed;
	double		max_acceleration;
	double[]	state_speed = new double[Altos.ao_flight_invalid + 1];
	double[]	state_baro_speed = new double[Altos.ao_flight_invalid + 1];
	double[]	state_accel = new double[Altos.ao_flight_invalid + 1];
	int[]		state_count = new int[Altos.ao_flight_invalid + 1];
	double[]	state_start = new double[Altos.ao_flight_invalid + 1];
	double[]	state_end = new double[Altos.ao_flight_invalid + 1];

	public AltosFlightStats(AltosFlightReader reader) throws InterruptedException, IOException {
		AltosState	state = null;
		AltosState	new_state = null;
		double		boost_time = -1;
		double		start_time;

		for (;;) {
			try {
				AltosRecord	record = reader.read();
				if (record == null)
					break;
				new_state = new AltosState(record, state);
				if (state == null) {
					start_time = new_state.time;
				}
				state = new_state;
				if (0 <= state.state && state.state < Altos.ao_flight_invalid) {
					if (boost_time == -1 && state.state >= Altos.ao_flight_boost)
						boost_time = state.time;
					state_accel[state.state] += state.acceleration;
					state_speed[state.state] += state.speed;
					state_baro_speed[state.state] += state.baro_speed;
					state_count[state.state]++;
					if (state_start[state.state] == 0.0)
						state_start[state.state] = state.time;
					if (state_end[state.state] < state.time)
						state_end[state.state] = state.time;
					max_height = state.max_height;
					max_speed = state.max_speed;
					max_acceleration = state.max_acceleration;
				}
			} catch (ParseException pp) {
				System.out.printf("Parse error: %d \"%s\"\n", pp.getErrorOffset(), pp.getMessage());
			} catch (AltosCRCException ce) {

			}
		}
		for (int s = Altos.ao_flight_startup; s <= Altos.ao_flight_landed; s++) {
			if (state_count[s] > 0) {
				state_speed[s] /= state_count[s];
				state_baro_speed[s] /= state_count[s];
				state_accel[s] /= state_count[s];
			}
		}
	}

	public AltosFlightStats(AltosRecordIterable iterable, File file) throws InterruptedException, IOException {
		this(new AltosReplayReader(iterable.iterator(), file));
	}

	public AltosFlightStats(AltosRecordIterable iterable) throws InterruptedException, IOException {
		this(iterable, new File(""));
	}
}
