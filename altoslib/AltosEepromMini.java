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
import java.util.*;
import java.text.*;

public class AltosEepromMini extends AltosEeprom {
	public static final int	record_length = 16;

	public int record_length() { return record_length; }

	/* AO_LOG_FLIGHT elements */
	public int flight() { return data16(0); }
	public int ground_pres() { return data32(4); }

	/* AO_LOG_STATE elements */
	public int state() { return data16(0); }
	public int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */
	public int pres() { return data24(0); }
	public int temp() { return data24(3); }
	public int sense_a() { return data16(6); }
	public int sense_m() { return data16(8); }
	public int v_batt() { return data16(10); }

	double voltage(AltosState state, int sensor) {
		if (state.log_format == AltosLib.AO_LOG_FORMAT_EASYMINI)
			return AltosConvert.easy_mini_voltage(sensor, state.serial);
		else
			return AltosConvert.tele_mini_voltage(sensor);
	}

	public void update_state(AltosState state) {
		super.update_state(state);

		switch (cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			state.set_flight(flight());
			state.set_ground_pressure(ground_pres());
			break;
		case AltosLib.AO_LOG_STATE:
			state.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.set_ms5607(pres(), temp());
			state.set_apogee_voltage(voltage(state, sense_a()));
			state.set_main_voltage(voltage(state, sense_m()));
			state.set_battery_voltage(voltage(state, v_batt()));
			break;
		}
	}

	public AltosEepromMini (AltosEepromChunk chunk, int start) throws ParseException {
		parse_chunk(chunk, start);
	}

	public AltosEepromMini (String line) {
		parse_string(line);
	}

	public AltosEepromMini(int in_cmd, int in_tick) {
		cmd = in_cmd;
		tick = in_tick;
		valid = true;
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> minis = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosEepromMini mini = new AltosEepromMini(line);
				minis.add(mini);
			} catch (IOException ie) {
				break;
			}
		}

		return minis;
	}
}
