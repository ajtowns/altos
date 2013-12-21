/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.altoslib_3;

import java.io.*;
import java.util.*;
import java.text.*;

public class AltosEepromTM extends AltosEeprom {
	public int	a;
	public int	b;

	public static final int	record_length = 8;

	public void write(PrintStream out) {
		out.printf("%c %4x %4x %4x\n", cmd, tick, a, b);
	}

	public int record_length() { return record_length; }

	public String string() {
		return String.format("%c %4x %4x %4x\n", cmd, tick, a, b);
	}

	public void update_state(AltosState state) {
		super.update_state(state);

		AltosGPS	gps;

		/* Flush any pending GPS changes */
		if (state.gps_pending) {
			switch (cmd) {
			case AltosLib.AO_LOG_GPS_LAT:
			case AltosLib.AO_LOG_GPS_LON:
			case AltosLib.AO_LOG_GPS_ALT:
			case AltosLib.AO_LOG_GPS_SAT:
			case AltosLib.AO_LOG_GPS_DATE:
				break;
			default:
				state.set_temp_gps();
				break;
			}
		}

		switch (cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			state.set_state(AltosLib.ao_flight_pad);
			state.set_ground_accel(a);
			state.set_flight(b);
			state.set_boost_tick(tick);
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.set_accel(a);
			state.set_pressure(AltosConvert.barometer_to_pressure(b));
			break;
		case AltosLib.AO_LOG_PRESSURE:
			state.set_pressure(AltosConvert.barometer_to_pressure(b));
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			state.set_temperature(AltosConvert.thermometer_to_temperature(a));
			state.set_battery_voltage(AltosConvert.cc_battery_to_voltage(b));
			break;
		case AltosLib.AO_LOG_DEPLOY:
			state.set_apogee_voltage(AltosConvert.cc_ignitor_to_voltage(a));
			state.set_main_voltage(AltosConvert.cc_ignitor_to_voltage(b));
			break;
		case AltosLib.AO_LOG_STATE:
			state.set_state(a);
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			gps = state.make_temp_gps(false);

			gps.hour = (a & 0xff);
			gps.minute = (a >> 8);
			gps.second = (b & 0xff);

			int flags = (b >> 8);

			gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;
			break;
		case AltosLib.AO_LOG_GPS_LAT:
			gps = state.make_temp_gps(false);

			int lat32 = a | (b << 16);
			gps.lat = (double) lat32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_LON:
			gps = state.make_temp_gps(false);

			int lon32 = a | (b << 16);
			gps.lon = (double) lon32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_ALT:
			gps = state.make_temp_gps(false);
			gps.alt = a;
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			gps = state.make_temp_gps(true);
			int svid = a;
			int c_n0 = b >> 8;
			gps.add_sat(svid, c_n0);
			break;
		case AltosLib.AO_LOG_GPS_DATE:
			gps = state.make_temp_gps(false);
			gps.year = (a & 0xff) + 2000;
			gps.month = a >> 8;
			gps.day = b & 0xff;
			break;
		}
	}

	public AltosEepromTM (AltosEepromChunk chunk, int start) throws ParseException {

		cmd = chunk.data(start);
		valid = true;

		valid = !chunk.erased(start, record_length);
		if (valid) {
			if (AltosConvert.checksum(chunk.data, start, record_length) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x",
								       chunk.address + start), 0);
		} else {
			cmd = AltosLib.AO_LOG_INVALID;
		}

		tick = chunk.data16(start + 2);
		a = chunk.data16(start + 4);
		b = chunk.data16(start + 6);
	}

	public AltosEepromTM (String line) {
		valid = false;
		tick = 0;
		a = 0;
		b = 0;
		if (line == null) {
			cmd = AltosLib.AO_LOG_INVALID;
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = AltosLib.AO_LOG_INVALID;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else {
					cmd = AltosLib.AO_LOG_INVALID;
				}
			} catch (NumberFormatException ne) {
				cmd = AltosLib.AO_LOG_INVALID;
			}
		}
	}

	public AltosEepromTM(int in_cmd, int in_tick, int in_a, int in_b) {
		valid = true;
		cmd = in_cmd;
		tick = in_tick;
		a = in_a;
		b = in_b;
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> tms = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				AltosEepromTM tm = new AltosEepromTM(line);
				tms.add(tm);
			} catch (IOException ie) {
				break;
			}
		}

		return tms;
	}

}
