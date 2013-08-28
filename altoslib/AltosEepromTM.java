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

package org.altusmetrum.altoslib_1;

import java.text.*;

public class AltosEepromTM implements AltosStateUpdate {
	public int	cmd;
	public int	tick;
	public int	a;
	public int	b;
	public String	data;
	public boolean	tick_valid;

	public static final int	record_length = 8;

	public void update_state(AltosState state) {
		state.set_tick(tick);
		switch (cmd) {
		case AltosLib.AO_LOG_FLIGHT:
			state.ground_accel = a;
			state.flight = b;
			state.set_boost_tick(tick);
			state.time = 0;
			break;
		case AltosLib.AO_LOG_SENSOR:
			state.set_telemetrum(a, b);
			break;
		case AltosLib.AO_LOG_PRESSURE:
			state.set_telemetrum(AltosState.MISSING, b);
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
/*
			
			record.temp = a;
			record.batt = b;
			eeprom_state.seen |= AltosRecord.seen_temp_volt;
*/
			break;
		case AltosLib.AO_LOG_DEPLOY:
/*
			record.drogue = a;
			record.main = b;
			eeprom_state.seen |= AltosRecord.seen_deploy;
			has_ignite = true;
*/
			break;
		case AltosLib.AO_LOG_STATE:
			state.state = a;
			break;
//		case AltosLib.AO_LOG_GPS_TIME:
//			eeprom_state.gps_tick = record.tick;
//			eeprom_state.seen |= AltosRecord.seen_gps_time;
//			AltosGPS old = state.gps;
//			AltosGPS gps = new AltosGPS();
//
//			/* GPS date doesn't get repeated through the file */
//			if (old != null) {
//				gps.year = old.year;
//				gps.month = old.month;
//				gps.day = old.day;
//			}
//			gps.hour = (a & 0xff);
//			gps.minute = (a >> 8);
//			gps.second = (b & 0xff);
//
//			int flags = (b >> 8);
//			gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
//			gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
//			gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
//				AltosLib.AO_GPS_NUM_SAT_SHIFT;
//			state.temp_gps = gps;
//			break;
//		case AltosLib.AO_LOG_GPS_LAT:
//			int lat32 = a | (b << 16);
//			if (state.temp_gps == null)
//				state.temp_gps = new AltosGPS();
//			state.temp_gps.lat = (double) lat32 / 1e7;
//			break;
//		case AltosLib.AO_LOG_GPS_LON:
//			int lon32 = a | (b << 16);
//			if (state.temp_gps == null)
//				state.temp_gps = new AltosGPS();
//			state.temp_gps.lon = (double) lon32 / 1e7;
//			break;
//		case AltosLib.AO_LOG_GPS_ALT:
//			if (state.temp_gps == null)
//				state.temp_gps = new AltosGPS();
//			state.temp_gps.alt = a;
//			break;
//		case AltosLib.AO_LOG_GPS_SAT:
//			if (record.tick == eeprom_state.gps_tick) {
//				int svid = a;
//				int c_n0 = b >> 8;
//				if (record.gps == null)
//					record.gps = new AltosGPS();
//				record.gps.add_sat(svid, c_n0);
//			}
//			break;
//		case AltosLib.AO_LOG_GPS_DATE:
//			if (record.gps == null)
//				record.gps = new AltosGPS();
//			record.gps.year = (a & 0xff) + 2000;
//			record.gps.month = a >> 8;
//			record.gps.day = b & 0xff;
//			break;

		case AltosLib.AO_LOG_CONFIG_VERSION:
			break;
		case AltosLib.AO_LOG_MAIN_DEPLOY:
			break;
		case AltosLib.AO_LOG_APOGEE_DELAY:
			break;
		case AltosLib.AO_LOG_RADIO_CHANNEL:
			break;
		case AltosLib.AO_LOG_CALLSIGN:
			state.callsign = data;
			break;
		case AltosLib.AO_LOG_ACCEL_CAL:
			state.accel_plus_g = a;
			state.accel_minus_g = b;
			break;
		case AltosLib.AO_LOG_RADIO_CAL:
			break;
		case AltosLib.AO_LOG_MANUFACTURER:
			break;
		case AltosLib.AO_LOG_PRODUCT:
			break;
		case AltosLib.AO_LOG_SERIAL_NUMBER:
			state.serial = a;
			break;
		case AltosLib.AO_LOG_SOFTWARE_VERSION:
			break;
		}
	}

	public AltosEepromTM (AltosEepromChunk chunk, int start) throws ParseException {

		cmd = chunk.data(start);
		tick_valid = true;

		tick_valid = !chunk.erased(start, record_length);
		if (tick_valid) {
			if (AltosConvert.checksum(chunk.data, start, record_length) != 0)
				throw new ParseException(String.format("invalid checksum at 0x%x",
								       chunk.address + start), 0);
		} else {
			cmd = AltosLib.AO_LOG_INVALID;
		}

		tick = chunk.data16(start + 2);
		a = chunk.data16(start + 4);
		b = chunk.data16(start + 6);

		data = null;
	}

	public AltosEepromTM (String line) {
		tick_valid = false;
		tick = 0;
		a = 0;
		b = 0;
		data = null;
		if (line == null) {
			cmd = AltosLib.AO_LOG_INVALID;
			data = "";
		} else {
			try {
				String[] tokens = line.split("\\s+");

				if (tokens[0].length() == 1) {
					if (tokens.length != 4) {
						cmd = AltosLib.AO_LOG_INVALID;
						data = line;
					} else {
						cmd = tokens[0].codePointAt(0);
						tick = Integer.parseInt(tokens[1],16);
						tick_valid = true;
						a = Integer.parseInt(tokens[2],16);
						b = Integer.parseInt(tokens[3],16);
					}
				} else if (tokens[0].equals("Config") && tokens[1].equals("version:")) {
					cmd = AltosLib.AO_LOG_CONFIG_VERSION;
					data = tokens[2];
				} else if (tokens[0].equals("Main") && tokens[1].equals("deploy:")) {
					cmd = AltosLib.AO_LOG_MAIN_DEPLOY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Apogee") && tokens[1].equals("delay:")) {
					cmd = AltosLib.AO_LOG_APOGEE_DELAY;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("channel:")) {
					cmd = AltosLib.AO_LOG_RADIO_CHANNEL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Callsign:")) {
					cmd = AltosLib.AO_LOG_CALLSIGN;
					data = tokens[1].replaceAll("\"","");
				} else if (tokens[0].equals("Accel") && tokens[1].equals("cal")) {
					cmd = AltosLib.AO_LOG_ACCEL_CAL;
					a = Integer.parseInt(tokens[3]);
					b = Integer.parseInt(tokens[5]);
				} else if (tokens[0].equals("Radio") && tokens[1].equals("cal:")) {
					cmd = AltosLib.AO_LOG_RADIO_CAL;
					a = Integer.parseInt(tokens[2]);
				} else if (tokens[0].equals("Max") && tokens[1].equals("flight") && tokens[2].equals("log:")) {
					cmd = AltosLib.AO_LOG_MAX_FLIGHT_LOG;
					a = Integer.parseInt(tokens[3]);
				} else if (tokens[0].equals("manufacturer")) {
					cmd = AltosLib.AO_LOG_MANUFACTURER;
					data = tokens[1];
				} else if (tokens[0].equals("product")) {
					cmd = AltosLib.AO_LOG_PRODUCT;
					data = tokens[1];
				} else if (tokens[0].equals("serial-number")) {
					cmd = AltosLib.AO_LOG_SERIAL_NUMBER;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("log-format")) {
					cmd = AltosLib.AO_LOG_LOG_FORMAT;
					a = Integer.parseInt(tokens[1]);
				} else if (tokens[0].equals("software-version")) {
					cmd = AltosLib.AO_LOG_SOFTWARE_VERSION;
					data = tokens[1];
				} else {
					cmd = AltosLib.AO_LOG_INVALID;
					data = line;
				}
			} catch (NumberFormatException ne) {
v				cmd = AltosLib.AO_LOG_INVALID;
				data = line;
			}
		}
	}

	public AltosEepromTM(int in_cmd, int in_tick, int in_a, int in_b) {
		tick_valid = true;
		cmd = in_cmd;
		tick = in_tick;
		a = in_a;
		b = in_b;
	}
}
