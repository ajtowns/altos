/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

public class AltosEepromGPS extends AltosEeprom {
	public static final int	record_length = 32;

	public static final int max_sat = 12;

	public int record_length() { return record_length; }

	/* AO_LOG_FLIGHT elements */
	public int flight() { return data16(0); }
	public int start_altitude() { return data16(2); }
	public int start_latitude() { return data32(4); }
	public int start_longitude() { return data32(8); }

	/* AO_LOG_GPS_TIME elements */
	public int latitude() { return data32(0); }
	public int longitude() { return data32(4); }
	public int altitude() { return data16(8); }
	public int hour() { return data8(10); }
	public int minute() { return data8(11); }
	public int second() { return data8(12); }
	public int flags() { return data8(13); }
	public int year() { return data8(14); }
	public int month() { return data8(15); }
	public int day() { return data8(16); }
	public int course() { return data8(17); }
	public int ground_speed() { return data16(18); }
	public int climb_rate() { return data16(20); }
	public int pdop() { return data8(22); }
	public int hdop() { return data8(23); }
	public int vdop() { return data8(24); }
	public int mode() { return data8(25); }

	public boolean has_seconds() { return cmd == AltosLib.AO_LOG_GPS_TIME; }

	public int seconds() {
		switch (cmd) {
		case AltosLib.AO_LOG_GPS_TIME:
			return second() + 60 * (minute() + 60 * (hour() + 24 * (day() + 31 * month())));
		default:
			return 0;
		}
	}

	public AltosEepromGPS (AltosEepromChunk chunk, int start) throws ParseException {
		parse_chunk(chunk, start);
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
			state.set_boost_tick(tick);
			state.set_flight(flight());
			/* no place to log start lat/lon yet */
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			state.set_tick(tick);
			gps = state.make_temp_gps(false);
			gps.lat = latitude() / 1e7;
			gps.lon = longitude() / 1e7;
			gps.alt = altitude();

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
			gps.ground_speed = ground_speed() * 1.0e-2;
			gps.course = course() * 2;
			gps.climb_rate = climb_rate() * 1.0e-2;
			gps.hdop = hdop();
			gps.vdop = vdop();
			break;
		}
	}

	public AltosEepromGPS (String line) {
		parse_string(line);
	}

	static public LinkedList<AltosEeprom> read(FileInputStream input) {
		LinkedList<AltosEeprom> tgpss = new LinkedList<AltosEeprom>();

		for (;;) {
			try {
				String line = AltosLib.gets(input);
				if (line == null)
					break;
				try {
					AltosEepromGPS tgps = new AltosEepromGPS(line);
					if (tgps.cmd != AltosLib.AO_LOG_INVALID)
						tgpss.add(tgps);
				} catch (Exception e) {
					System.out.printf ("exception\n");
				}
			} catch (IOException ie) {
				break;
			}
		}

		return tgpss;
	}
}
