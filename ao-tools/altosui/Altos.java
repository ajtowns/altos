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

package altosui;

import java.awt.*;
import java.util.*;
import java.text.*;

public class Altos {
	/* EEProm command letters */
	static final int AO_LOG_FLIGHT = 'F';
	static final int AO_LOG_SENSOR = 'A';
	static final int AO_LOG_TEMP_VOLT = 'T';
	static final int AO_LOG_DEPLOY = 'D';
	static final int AO_LOG_STATE = 'S';
	static final int AO_LOG_GPS_TIME = 'G';
	static final int AO_LOG_GPS_LAT = 'N';
	static final int AO_LOG_GPS_LON = 'W';
	static final int AO_LOG_GPS_ALT = 'H';
	static final int AO_LOG_GPS_SAT = 'V';
	static final int AO_LOG_GPS_DATE = 'Y';

	/* Added for 'serial-number' entry in eeprom files */
	static final int AO_LOG_SERIAL_NUMBER = 1000;

	/* Added to flag invalid records */
	static final int AO_LOG_INVALID = -1;

	/* Flight state numbers and names */
	static final int ao_flight_startup = 0;
	static final int ao_flight_idle = 1;
	static final int ao_flight_pad = 2;
	static final int ao_flight_boost = 3;
	static final int ao_flight_fast = 4;
	static final int ao_flight_coast = 5;
	static final int ao_flight_drogue = 6;
	static final int ao_flight_main = 7;
	static final int ao_flight_landed = 8;
	static final int ao_flight_invalid = 9;

	static HashMap<String,Integer>	string_to_state = new HashMap<String,Integer>();

	static boolean map_initialized = false;

	static void initialize_map()
	{
		string_to_state.put("startup", ao_flight_startup);
		string_to_state.put("idle", ao_flight_idle);
		string_to_state.put("pad", ao_flight_pad);
		string_to_state.put("boost", ao_flight_boost);
		string_to_state.put("fast", ao_flight_fast);
		string_to_state.put("coast", ao_flight_coast);
		string_to_state.put("drogue", ao_flight_drogue);
		string_to_state.put("main", ao_flight_main);
		string_to_state.put("landed", ao_flight_landed);
		string_to_state.put("invalid", ao_flight_invalid);
		map_initialized = true;
	}

	static String[] state_to_string = {
		"startup",
		"idle",
		"pad",
		"boost",
		"fast",
		"coast",
		"drogue",
		"main",
		"landed",
		"invalid",
	};

	static public int state(String state) {
		if (!map_initialized)
			initialize_map();
		if (string_to_state.containsKey(state))
			return string_to_state.get(state);
		return ao_flight_invalid;
	}

	static public String state_name(int state) {
		if (state < 0 || state_to_string.length <= state)
			return "invalid";
		return state_to_string[state];
	}
}
