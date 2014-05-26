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

public class AltosEepromMetrum2 extends AltosEeprom {
	public static final int	record_length = 16;

	public int record_length() { return record_length; }

	/* AO_LOG_FLIGHT elements */
	public int flight() { return data16(0); }
	public int ground_accel() { return data16(2); }
	public int ground_pres() { return data32(4); }
	public int ground_temp() { return data32(8); }

	/* AO_LOG_STATE elements */
	public int state() { return data16(0); }
	public int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */
	public int pres() { return data32(0); }
	public int temp() { return data32(4); }
	public int accel() { return data16(8); }

	/* AO_LOG_TEMP_VOLT elements */
	public int v_batt() { return data16(0); }
	public int sense_a() { return data16(2); }
	public int sense_m() { return data16(4); }

	/* AO_LOG_GPS_POS elements */
	public int latitude() { return data32(0); }
	public int longitude() { return data32(4); }
	public int altitude() { return data16(8); }

	/* AO_LOG_GPS_TIME elements */
	public int hour() { return data8(0); }
	public int minute() { return data8(1); }
	public int second() { return data8(2); }
	public int flags() { return data8(3); }
	public int year() { return data8(4); }
	public int month() { return data8(5); }
	public int day() { return data8(6); }

	/* AO_LOG_GPS_SAT elements */
	public int nsat() { return data8(0); }
	public int more() { return data8(1); }
	public int svid(int n) { return data8(2 + n * 2); }
	public int c_n(int n) { return data8(2 + n * 2 + 1); }

	public AltosEepromMetrum2 (AltosEepromChunk chunk, int start) throws ParseException {
		parse_chunk(chunk, start);
	}

	public void update_state(AltosState state) {
		super.update_state(state);

		AltosGPS	gps;

		/* Flush any pending GPS changes */
		if (state.gps_pending) {
			switch (cmd) {
			case AltosLib.AO_LOG_GPS_POS:
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
			state.set_flight(flight());
			state.set_ground_accel(ground_accel());
			state.set_ground_pressure(ground_pres());
//			state.set_temperature(ground_temp() / 100.0);
			break;
		case AltosLib.AO_LOG_STATE:
			state.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.set_ms5607(pres(), temp());
			state.set_accel(accel());

			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			state.set_battery_voltage(AltosConvert.mega_battery_voltage(v_batt()));

			state.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sense_a()));
			state.set_main_voltage(AltosConvert.mega_pyro_voltage(sense_m()));

			break;
		case AltosLib.AO_LOG_GPS_POS:
			gps = state.make_temp_gps(false);
			gps.lat = latitude() / 1e7;
			gps.lon = longitude() / 1e7;
			gps.alt = altitude();
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			gps = state.make_temp_gps(false);

			gps.hour = hour();
			gps.minute = minute();
			gps.second = second();

			int flags = flags();

			gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;

			gps.year = 2000 + year();
			gps.month = month();
			gps.day = day();
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			gps = state.make_temp_gps(true);

			int n = nsat();
			for (int i = 0; i < n; i++)
				gps.add_sat(svid(i), c_n(i));
			break;
		}
	}

	public AltosEepromMetrum2 (String line) {
		parse_string(line);
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> metrums = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				try {
					AltosEepromMetrum2 metrum = new AltosEepromMetrum2(line);

					if (metrum.cmd != AltosLib.AO_LOG_INVALID)
						metrums.add(metrum);
				} catch (Exception e) {
					System.out.printf ("exception\n");
				}
			} catch (IOException ie) {
				break;
			}
		}

		return metrums;
	}
}
